/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/codeguard.inc
 *
 * PURPOSE:
 *   Bind native quality commands and copied output to the current workspace owner.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/codeguard_workflow.h"
#include "umicom/codeguard/output_gtk4.h"

/* These queries have no side effects. A retained menu repeats admission at
 * dispatch; cancellation uses job ownership rather than workspace trust. */
static int RuntimeCodeGuardCommandEnabled(UmiStudioGtkRuntimeChrome *runtime,
    const char *command)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        !UmiGtk4WindowIsOpen(runtime->owner->window) || command == NULL ||
        g_object_get_data(G_OBJECT(runtime->owner->window), "umicom-codeguard-command") != NULL)
        return 0;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiCodeGuardReviewSnapshot review;
    if (UmiStudioCodeGuardSnapshot(services, &review) != UMI_STATUS_OK) return 0;
    if (strcmp(command, "codeguard.cancel") == 0) return review.pending;
    UmiStudioWorkspaceSnapshot workspace;
    return strcmp(command, "codeguard.scan") == 0 && !review.pending &&
        umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
        workspace.graph.open && workspace.graph.revision != 0U;
}

static UmiCodeGuardReviewSnapshot RuntimeCodeGuardEmpty(void)
{
    UmiCodeGuardReviewSnapshot snapshot = {0};
    snapshot.state = UMI_TASK_CREATED;
    snapshot.status = snapshot.operation_status = UMI_STATUS_NOT_FOUND;
    return snapshot;
}

/* Poll even without a mounted output panel, so completed work can retire.
 * Framework copies all report bytes before GTK observes them. Only the weak
 * host token and a retained widget cross native callbacks; no service borrow
 * survives those boundaries. A nested refresh leaves the outer publisher alone. */
static UmiStatus RuntimeRefreshCodeGuard(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL, *guard = NULL;
    GtkWidget *panel = NULL;
    UmiStatus status = UMI_STATUS_CANCELLED;
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) goto done;
    if (g_object_get_data(window, "umicom-codeguard-refresh") != NULL) {
        status = UMI_STATUS_OK;
        goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-codeguard-refresh", GINT_TO_POINTER(1));
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    UmiCodeGuardReviewSnapshot review = RuntimeCodeGuardEmpty();
    uint64_t context = 0U;
    status = umi_studio_workspace_snapshot(services, &workspace);
    if (status == UMI_STATUS_OK) {
        context = workspace.graph.open ? workspace.graph.revision : 0U;
        status = UmiStudioCodeGuardPoll(services, &review);
    }
    if (runtime->codeguard_output != NULL) {
        panel = g_object_ref(runtime->codeguard_output);
        if (status == UMI_STATUS_OK) {
            const UmiCodeGuardResult *result = review.has_report
                ? UmiStudioCodeGuardResult(services) : NULL;
            status = UmiCodeGuardOutputGtk4Update(panel, context, &review, result);
        } else {
            /* A failed observation must not leave old evidence looking current.
             * Publish an empty adapter snapshot, which also invalidates its
             * equality cache so a later successful poll can restore the report. */
            UmiCodeGuardReviewSnapshot empty = RuntimeCodeGuardEmpty();
            UmiStatus cleared = UmiCodeGuardOutputGtk4Update(panel, context, &empty, NULL);
            if (cleared != UMI_STATUS_OK) status = cleared;
        }
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
        services = umi_studio_ui_services(runtime->owner->ui);
        UmiStatus current = umi_studio_workspace_snapshot(services, &workspace);
        uint64_t now = current == UMI_STATUS_OK && workspace.graph.open
            ? workspace.graph.revision : 0U;
        if (current != UMI_STATUS_OK || now != context) {
            /* A text observer may switch projects while a copied report is
             * being displayed. Remove that retired evidence immediately, even
             * with Follow paused. The next tick projects the new workspace. */
            UmiCodeGuardReviewSnapshot empty = RuntimeCodeGuardEmpty();
            UmiStatus cleared = UmiCodeGuardOutputGtk4Update(panel, 0U, &empty, NULL);
            if (cleared != UMI_STATUS_OK) status = cleared;
            runtime = RuntimeResolveOwner(request, &window);
            if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
        }
    }
    if (status == UMI_STATUS_OK) runtime->codeguard_last_error = UMI_STATUS_OK;
    else if (status != UMI_STATUS_BUSY && runtime->codeguard_last_error != status) {
        /* Remember before notifying: observers may reenter or destroy Studio. */
        runtime->codeguard_last_error = status;
        char message[192];
        (void)snprintf(message, sizeof message, "CodeGuard update unavailable: %s",
            umi_status_text(status));
        UmiStatus notified = RuntimeNotifyAndContinue(runtime, message);
        if (notified != UMI_STATUS_OK) status = notified;
    }
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-codeguard-refresh", NULL);
    g_clear_object(&panel);
    g_clear_object(&guard);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Revealing Output is observable. Capture the originating graph before it and
 * refuse submission if callbacks close, replace or otherwise revise that graph.
 * No project command, save, compiler or external process is run by this route. */
static UmiStatus RuntimeCodeGuardCommand(UmiStudioGtkRuntimeChrome *runtime,
    const char *command, const char *argument)
{
    int cancel = command != NULL && strcmp(command, "codeguard.cancel") == 0;
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        (!cancel && (command == NULL || strcmp(command, "codeguard.scan") != 0)) ||
        (argument != NULL && argument[0] != '\0')) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL, *guard = NULL;
    GtkWidget *expander = NULL;
    UmiStatus status = UMI_STATUS_CANCELLED;
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) goto done;
    if (g_object_get_data(window, "umicom-codeguard-command") != NULL) {
        status = UMI_STATUS_BUSY;
        goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-codeguard-command", GINT_TO_POINTER(1));
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiCodeGuardReviewSnapshot review;
    status = UmiStudioCodeGuardSnapshot(services, &review);
    if (status != UMI_STATUS_OK) goto done;
    if (cancel) {
        status = review.pending ? UmiStudioCodeGuardCancel(services) : UMI_STATUS_INVALID_STATE;
        if (status == UMI_STATUS_OK) (void)RuntimeRefreshCodeGuard(runtime);
        goto observe;
    }
    if (review.pending) { status = UMI_STATUS_BUSY; goto done; }
    UmiStudioWorkspaceSnapshot origin, current;
    status = umi_studio_workspace_snapshot(services, &origin);
    if (status != UMI_STATUS_OK) goto done;
    if (!origin.graph.open || origin.graph.revision == 0U) {
        status = UMI_STATUS_INVALID_STATE;
        goto done;
    }
    status = runtime_show_surface(runtime, UMI_STUDIO_SURFACE_OUTPUT);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status != UMI_STATUS_OK) goto done;
    if (!GTK_IS_EXPANDER(runtime->codeguard_expander)) {
        status = UMI_STATUS_UNAVAILABLE;
        goto done;
    }
    expander = g_object_ref(runtime->codeguard_expander);
    gtk_expander_set_expanded(GTK_EXPANDER(expander), TRUE);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (!gtk_expander_get_expanded(GTK_EXPANDER(expander)) ||
        gtk_widget_get_root(expander) != GTK_ROOT(window) ||
        !gtk_widget_get_visible(expander)) {
        status = UMI_STATUS_CANCELLED;
        goto done;
    }
    services = umi_studio_ui_services(runtime->owner->ui);
    status = umi_studio_workspace_snapshot(services, &current);
    if (status != UMI_STATUS_OK) goto done;
    if (!current.graph.open || current.graph.revision != origin.graph.revision ||
        !umi_path_equal(current.graph.root, origin.graph.root)) {
        status = UMI_STATUS_CANCELLED;
        goto done;
    }
    uint64_t request_id;
    status = UmiStudioCodeGuardBegin(services, NULL, &request_id);
    /* Admission's success is independent of presentation. A later preview
     * failure must not invite an accidental duplicate submission. */
    if (status == UMI_STATUS_OK) (void)RuntimeRefreshCodeGuard(runtime);
observe:
    if (RuntimeResolveOwner(request, &window) == NULL) status = UMI_STATUS_CANCELLED;
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-codeguard-command", NULL);
    g_clear_object(&expander);
    g_clear_object(&guard);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}
