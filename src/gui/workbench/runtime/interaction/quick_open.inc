/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/quick_open.inc
 * PURPOSE: Connect Framework indexed-file selection to the current Studio workspace and document owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/quick_open.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "umicom/developer_workbench/quick_open_selection.h"
typedef struct RuntimeQuickOpenRequest
{
    UmiStudioFileDialogRequest *owner;
    UmiWorkspaceGraphSnapshot workspace;
    UmiFileIndex *index; /* Identity only; reacquire through the live host before use. */
} RuntimeQuickOpenRequest;
static void RuntimeQuickOpenFree(gpointer data)
{
    RuntimeQuickOpenRequest *request = data;
    runtime_file_dialog_request_free(request->owner);
    g_free(request);
}
static UmiStudioGtkRuntimeChrome *RuntimeQuickOpenOwner(RuntimeQuickOpenRequest *request, GObject **window)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(*window)))
        return NULL;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiWorkspaceGraphSnapshot current;
    /* Reject changing away and back as well as a different current root.
     * File-index revision alone cannot describe a replaced workspace owner. */
    if (umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &current) != UMI_STATUS_OK ||
        !current.open || current.revision != request->workspace.revision ||
        strcmp(current.root, request->workspace.root) != 0 ||
        umi_studio_services_file_index(services) != request->index)
        return NULL;
    return runtime;
}
static UmiStatus RuntimeQuickOpenRead(gpointer data, const char *query, size_t offset, uint64_t revision,
                                      UmiFileIndexEntry *entries, size_t capacity, UmiFileIndexPage *page)
{
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status =
        runtime == NULL
            ? UMI_STATUS_INVALID_STATE
            : UmiFileIndexReadPage(umi_studio_services_file_index(umi_studio_ui_services(runtime->owner->ui)),
                                   query, 0, offset, revision, entries, capacity, page);
    if (status == UMI_STATUS_OK && strcmp(page->stats.root, request->workspace.root) != 0)
        status = UMI_STATUS_BUSY;
    g_clear_object(&window);
    return status;
}
/* Quick Open must reveal the selected source even when the editor is hidden. Retain the previous model-only callback; Framework still checks the indexed selection before opening. */
#if 0
static UmiStatus RuntimeQuickOpenSelected(gpointer data, const char *query, const UmiFileIndexPage *page,
                                          size_t position, const UmiFileIndexEntry *entry)
{
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status = runtime == NULL
                           ? UMI_STATUS_INVALID_STATE
                           : UmiQuickOpenCheckSelection(
                                 umi_studio_services_file_index(umi_studio_ui_services(runtime->owner->ui)),
                                 query, page, position, entry);
    char view[UMI_UI_ID_CAPACITY];
    if (status == UMI_STATUS_OK)
        status = umi_document_coordinator_open(umi_studio_ui_documents(runtime->owner->ui), entry->path, view,
                                               sizeof(view));
    /* Opening may notify observers which close or replace the host. Resolve
     * its weak binding again before activating a view or refreshing chrome. */
    g_clear_object(&window);
    runtime = RuntimeQuickOpenOwner(request, &window);
    if (runtime == NULL)
        status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK)
    {
        status = umi_ui_workbench_activate_document(umi_studio_ui_workbench(runtime->owner->ui), view);
        g_clear_object(&window);
        runtime = RuntimeQuickOpenOwner(request, &window);
        if (runtime != NULL)
            (void)runtime_refresh_services(runtime);
    }
    g_clear_object(&window);
    return status;
}

#endif
static UmiStatus RuntimeQuickOpenSelected(gpointer data, const char *query, const UmiFileIndexPage *page,
    size_t position, const UmiFileIndexEntry *entry)
{
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status = runtime == NULL ? UMI_STATUS_INVALID_STATE
        : UmiQuickOpenCheckSelection(umi_studio_services_file_index(
            umi_studio_ui_services(runtime->owner->ui)), query, page, position, entry);
    /* Preserve the indexed selection check, then use the ordinary Open route.
     * That route retains the path and reveals/focuses the coordinator's editor. */
    if (status == UMI_STATUS_OK) status = RuntimeOpenSourcePath(runtime, entry->path);
    g_clear_object(&window);
    return status;
}
/* Source coordinates are resolved by Framework against the current draft.
 * Use one open-and-position operation so Back returns to the departure, rather
 * than to an intermediate activation at the start of the selected file. */
static UmiStatus RuntimeQuickOpenSelectedAt(gpointer data, const char *filter,
    const UmiFileIndexPage *page, size_t position, const UmiFileIndexEntry *entry,
    const UmiDocumentLocation *location)
{
    if (location == NULL) return RuntimeQuickOpenSelected(data, filter, page, position, entry);
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status = runtime == NULL ? UMI_STATUS_INVALID_STATE
        : UmiQuickOpenCheckSelection(umi_studio_services_file_index(
            umi_studio_ui_services(runtime->owner->ui)), filter, page, position, entry);
    if (status == UMI_STATUS_OK) {
        UmiDocumentCoordinator *documents = umi_studio_ui_documents(runtime->owner->ui);
        status = UmiDocumentCoordinatorOpenAtPosition(documents, entry->path,
            location->line, location->column, NULL);
        /* Opening and cursor movement can invoke host observers. Resolve the
         * captured workspace again before presenting its active editor. */
        g_clear_object(&window);
        runtime = RuntimeQuickOpenOwner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        else if (status == UMI_STATUS_OK) {
            UmiDocumentWorkingCopySnapshot active;
            status = umi_document_coordinator_active_snapshot(documents, &active);
            if (status == UMI_STATUS_OK && strcmp(active.path, entry->path) != 0)
                status = UMI_STATUS_INVALID_STATE;
            if (status == UMI_STATUS_OK) {
                runtime_copy_selection(runtime->selected_file_path, sizeof runtime->selected_file_path, entry->path);
                status = RuntimePresentActiveSource(runtime);
            }
        }
    }
    g_clear_object(&window);
    return status;
}
static void RuntimeQuickOpenWindowFree(gpointer data)
{
    GtkWindow *window = data;
    gtk_window_destroy(window);
    g_object_unref(window);
}
/* Quick Open creation now reserves one native construction and rechecks the captured workspace after construction and previous-picker removal. Retain the earlier publication path for review. */
#if 0
static void RuntimeShowQuickOpen(UmiStudioGtkRuntimeChrome *runtime)
{
    RuntimeQuickOpenRequest *request = g_new0(RuntimeQuickOpenRequest, 1);
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &request->workspace) !=
            UMI_STATUS_OK ||
        !request->workspace.open || !umi_path_is_absolute(request->workspace.root))
    {
        g_free(request);
        runtime_show_notification(
            runtime, "Open a workspace before using Quick Open. Explorer Refresh discovers its files.");
        return;
    }
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    request->index = umi_studio_services_file_index(services);
    if (request->owner == NULL)
    {
        RuntimeQuickOpenFree(request);
        return;
    }
    GtkWindow *dialog = NULL;
    GObject *window = G_OBJECT(g_object_ref(runtime->owner->window));
    UmiStatus status =
        UmiGtk4QuickOpenCreate(GTK_WINDOW(window), RuntimeQuickOpenRead, RuntimeQuickOpenSelected, request,
                               RuntimeQuickOpenFree, &dialog);
    if (status != UMI_STATUS_OK)
        RuntimeQuickOpenFree(request);
    else
    {
        /* One picker per owner; opening again captures the current workspace.
         * The parent holds the returned reference, not a raw transient pointer. */
        g_object_set_data_full(window, "umicom-studio-quick-open-window", g_object_ref(dialog),
                               RuntimeQuickOpenWindowFree);
        if (UmiGtk4WindowIsOpen(GTK_WINDOW(window)) && UmiGtk4WindowIsOpen(dialog))
            gtk_window_present(dialog);
        g_object_unref(dialog);
    }
    g_object_unref(window);
}

#endif
static void RuntimeShowQuickOpen(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL) return;
    GObject *window = G_OBJECT(g_object_ref(runtime->owner->window));
    if (g_object_get_data(window, "studio-quick-open-creating") != NULL) { g_object_unref(window); return; }
    g_object_set_data(window, "studio-quick-open-creating", GINT_TO_POINTER(1));
    RuntimeQuickOpenRequest *request = g_new0(RuntimeQuickOpenRequest, 1);
    GtkWindow *dialog = NULL;
    GObject *resolvedWindow = NULL;
    int transferred = 0;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &request->workspace) != UMI_STATUS_OK ||
        !request->workspace.open || !umi_path_is_absolute(request->workspace.root)) {
        runtime_show_notification(runtime, "Open a workspace before using Quick Open. Explorer Refresh discovers its files.");
        goto done;
    }
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    request->index = umi_studio_services_file_index(services);
    if (request->owner == NULL) goto done;
/* Studio opts into shared filename-and-position parsing; the original literal callback remains the plain-open path. Retained for engineering review. */
#if 0
    UmiStatus status = UmiGtk4QuickOpenCreate(GTK_WINDOW(window), RuntimeQuickOpenRead,
        RuntimeQuickOpenSelected, request, RuntimeQuickOpenFree, &dialog);
#endif
    UmiStatus status = UmiGtk4QuickOpenCreateAt(GTK_WINDOW(window), RuntimeQuickOpenRead,
        RuntimeQuickOpenSelectedAt, request, RuntimeQuickOpenFree, &dialog);
    if (status != UMI_STATUS_OK) {
        runtime = RuntimeQuickOpenOwner(request, &resolvedWindow);
        if (runtime != NULL) {
            char message[256];
            snprintf(message, sizeof message, "Quick Open could not open: %s. Reopen it for the current workspace.", umi_status_text(status));
            runtime_show_notification(runtime, message);
        }
        goto done;
    }
    transferred = 1;
    /* The previous picker is destroyed when replaced. Its callbacks can close
     * Studio or select another workspace, so recheck before and after replacing
     * the owner's reference and before presenting the new picker. */
    runtime = RuntimeQuickOpenOwner(request, &resolvedWindow);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(dialog)) goto cancelled;
    g_object_set_data_full(window, "umicom-studio-quick-open-window", g_object_ref(dialog), RuntimeQuickOpenWindowFree);
    g_clear_object(&resolvedWindow);
    runtime = RuntimeQuickOpenOwner(request, &resolvedWindow);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(dialog) ||
        g_object_get_data(window, "umicom-studio-quick-open-window") != dialog) goto cancelled;
    gtk_window_present(dialog);
    goto done;
cancelled:
    gtk_window_destroy(dialog);
done:
    g_object_set_data(window, "studio-quick-open-creating", NULL);
    g_clear_object(&resolvedWindow);
    if (dialog != NULL) g_object_unref(dialog);
    if (!transferred) RuntimeQuickOpenFree(request);
    g_object_unref(window);
}
static void RuntimeQuickOpenClicked(GtkButton *button, gpointer data)
{
    (void)button;
    RuntimeShowQuickOpen(data);
}
