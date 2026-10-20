/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/prompt_dialog.inc
 *
 * PURPOSE:
 *   Present GTK-only text prompts while routing accepted values back into the
 *   Framework command/service boundary. The prompt owns no developer behavior.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

typedef UmiStatus (*UmiStudioGtkPromptAccepted)(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *value);

typedef struct UmiStudioGtkPromptContext {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *window;
    GtkWidget *entry;
    UmiStudioGtkPromptAccepted accepted;
    UmiStudioFileDialogRequest *ownerRequest;
    UmiWorkspaceGraphSnapshot workspace;
    int guardWorkspace;
} UmiStudioGtkPromptContext;

/*
 * Release or reset state held by runtime prompt context so the same storage can be reused
 * safely.
 */
static void runtime_prompt_context_destroy(gpointer data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)data;
    if (context != NULL) runtime_file_dialog_request_free(context->ownerRequest);
    free(context);
}

/*
 * Provide the runtime prompt cancelled operation used by this module and its client
 * applications.
 */
static void runtime_prompt_cancelled(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)user_data;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context != NULL && GTK_IS_WINDOW(context->window)) {
        gtk_window_destroy(GTK_WINDOW(context->window));
    }
}

/*
 * Provide the runtime prompt accepted operation used by this module and its client
 * applications.
 */
/* Prompt acceptance now owns the submitted text and resolves the host after operation callbacks. Retain the former borrowed continuation for engineering review. */
#if 0
static void runtime_prompt_accepted(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)user_data;
    const char *value;
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL || context->runtime == NULL ||
        context->entry == NULL || context->accepted == NULL) {
        return;
    }

    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context->ownerRequest, &ownerWindow);
    if (runtime == NULL) {
        gtk_window_destroy(GTK_WINDOW(context->window));
        g_clear_object(&ownerWindow);
        return;
    }
    if (context->guardWorkspace) {
        UmiWorkspaceGraphSnapshot current;
        UmiStatus workspaceStatus = umi_workspace_graph_snapshot(
            umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)),
            &current);
        if (workspaceStatus != UMI_STATUS_OK || !current.open ||
            current.revision != context->workspace.revision ||
            !umi_path_equal(current.root, context->workspace.root)) {
            runtime_show_notification(runtime,
                "The workspace changed while this form was open. Cancel and open the form again.");
            g_object_unref(ownerWindow);
            return;
        }
    }
    value = gtk_editable_get_text(GTK_EDITABLE(context->entry));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (value == NULL || value[0] == '\0') {
        runtime_show_notification(
            runtime,
            "A value is required.");
        g_object_unref(ownerWindow);
        return;
    }

    GtkWindow *dialog = g_object_ref(GTK_WINDOW(context->window));
    status = context->accepted(runtime, value);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) gtk_window_destroy(dialog);
    else runtime_show_notification(runtime, umi_status_text(status));
    g_object_unref(ownerWindow);
    g_object_unref(dialog);
}

#endif
static void runtime_prompt_accepted(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)user_data;
    const char *value;
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL || context->runtime == NULL ||
        context->entry == NULL || context->accepted == NULL) {
        return;
    }

    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context->ownerRequest, &ownerWindow);
    if (runtime == NULL) {
        gtk_window_destroy(GTK_WINDOW(context->window));
        g_clear_object(&ownerWindow);
        return;
    }
    if (context->guardWorkspace) {
        UmiWorkspaceGraphSnapshot current;
        UmiStatus workspaceStatus = umi_workspace_graph_snapshot(
            umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)),
            &current);
        if (workspaceStatus != UMI_STATUS_OK || !current.open ||
            current.revision != context->workspace.revision ||
            !umi_path_equal(current.root, context->workspace.root)) {
            runtime_show_notification(runtime,
                "The workspace changed while this form was open. Cancel and open the form again.");
            g_object_unref(ownerWindow);
            return;
        }
    }
    value = gtk_editable_get_text(GTK_EDITABLE(context->entry));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (value == NULL || value[0] == '\0') {
        runtime_show_notification(
            runtime,
            "A value is required.");
        g_object_unref(ownerWindow);
        return;
    }

    UmiStudioFileDialogRequest *continuation = runtime_file_dialog_request_new(runtime, 0U);
    if (continuation == NULL) {
        g_object_unref(ownerWindow);
        return;
    }
    GtkWindow *dialog = g_object_ref(GTK_WINDOW(context->window));
    char *acceptedValue = g_strdup(value);
    UmiStudioGtkPromptAccepted accepted = context->accepted;
    uint64_t notificationRevision = runtime->notification_revision;
    status = accepted(runtime, acceptedValue);
    g_free(acceptedValue);
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(continuation, &ownerWindow);
    /* Completion callbacks can close both windows. Never reuse their borrowed
     * runtime or entry text, and preserve specific feedback from the operation. */
    if (status == UMI_STATUS_OK || runtime == NULL) gtk_window_destroy(dialog);
    else if (runtime->notification_revision == notificationRevision)
        runtime_show_notification(runtime, umi_status_text(status));
    runtime_file_dialog_request_free(continuation);
    g_clear_object(&ownerWindow);
    g_object_unref(dialog);
}

/*
 * Provide the runtime prompt text operation used by this module and its client
 * applications.
 */
/* Prompt buttons and presentation now follow native window lifetime. The previous borrowed callbacks and post-presentation context access are retained for review. */
#if 0
static UmiStatus runtime_prompt_text_internal(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *title,
    const char *message,
    const char *initial_value,
    const char *accept_label,
    UmiStudioGtkPromptAccepted accepted,
    int guardWorkspace)
{
    UmiStudioGtkPromptContext *context;
    GtkWidget *window;
    GtkWidget *root;
    GtkWidget *buttons;
    GtkWidget *cancel;
    GtkWidget *accept;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL || accepted == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    context = (UmiStudioGtkPromptContext *)calloc(
        1U,
        sizeof(*context));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    context->guardWorkspace = guardWorkspace;
    if (guardWorkspace) {
        UmiStatus workspaceStatus = umi_workspace_graph_snapshot(
            umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)),
            &context->workspace);
        if (workspaceStatus != UMI_STATUS_OK || !context->workspace.open ||
            !context->workspace.trusted) {
            free(context);
            return workspaceStatus != UMI_STATUS_OK ? workspaceStatus : UMI_STATUS_PERMISSION_DENIED;
        }
    }
    window = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    context->entry = gtk_entry_new();
    g_object_set_data(G_OBJECT(context->entry), "umicom-automation-id", "studio.prompt.value");
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    accept = gtk_button_new_with_label(
        accept_label != NULL ? accept_label : "OK");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (window == NULL || root == NULL || context->entry == NULL ||
        buttons == NULL || cancel == NULL || accept == NULL) {
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    g_object_set_data(G_OBJECT(accept), "umicom-automation-id", "studio.prompt.accept");
    context->runtime = runtime;
    context->window = window;
    context->accepted = accepted;
    context->ownerRequest = runtime_file_dialog_request_new(runtime, 0U);
    if (context->ownerRequest == NULL) {
        gtk_window_destroy(GTK_WINDOW(window));
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(window),
        title != NULL ? title : "Umicom Studio");
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        runtime->owner->window);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), 540, 220);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message != NULL && message[0] != '\0') {
        gtk_box_append(GTK_BOX(root), runtime_label(message, 0.0F));
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (initial_value != NULL) {
        gtk_editable_set_text(
            GTK_EDITABLE(context->entry),
            initial_value);
    }

    gtk_box_append(GTK_BOX(root), context->entry);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), accept);
    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(GTK_WINDOW(window), root);

    g_signal_connect(
        cancel,
        "clicked",
        G_CALLBACK(runtime_prompt_cancelled),
        context);
    g_signal_connect(
        accept,
        "clicked",
        G_CALLBACK(runtime_prompt_accepted),
        context);
    g_object_set_data_full(
        G_OBJECT(window),
        "umicom-prompt-context",
        context,
        runtime_prompt_context_destroy);

    gtk_window_present(GTK_WINDOW(window));
    gtk_widget_grab_focus(context->entry);
    return UMI_STATUS_OK;
}

#endif
/* The native window is the signal lifetime owner. Check attachment as well as
 * object lifetime because automation or accessibility clients may retain a button
 * after its dialog has been destroyed. Such clicks have no remaining authority. */
static void RuntimePromptAcceptClicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    if (!gtk_widget_get_visible(GTK_WIDGET(window)) ||
        gtk_widget_get_root(GTK_WIDGET(button)) != GTK_ROOT(window)) return;
    UmiStudioGtkPromptContext *context = g_object_get_data(G_OBJECT(window), "umicom-prompt-context");
    if (context != NULL) runtime_prompt_accepted(button, context);
}

static void RuntimePromptCancelClicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    if (!gtk_widget_get_visible(GTK_WIDGET(window)) ||
        gtk_widget_get_root(GTK_WIDGET(button)) != GTK_ROOT(window)) return;
    UmiStudioGtkPromptContext *context = g_object_get_data(G_OBJECT(window), "umicom-prompt-context");
    if (context != NULL) runtime_prompt_cancelled(button, context);
}

static UmiStatus runtime_prompt_text_internal(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *title,
    const char *message,
    const char *initial_value,
    const char *accept_label,
    UmiStudioGtkPromptAccepted accepted,
    int guardWorkspace)
{
    UmiStudioGtkPromptContext *context;
    GtkWidget *window;
    GtkWidget *root;
    GtkWidget *buttons;
    GtkWidget *cancel;
    GtkWidget *accept;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL || accepted == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    context = (UmiStudioGtkPromptContext *)calloc(
        1U,
        sizeof(*context));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    context->guardWorkspace = guardWorkspace;
    if (guardWorkspace) {
        UmiStatus workspaceStatus = umi_workspace_graph_snapshot(
            umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)),
            &context->workspace);
        if (workspaceStatus != UMI_STATUS_OK || !context->workspace.open ||
            !context->workspace.trusted) {
            free(context);
            return workspaceStatus != UMI_STATUS_OK ? workspaceStatus : UMI_STATUS_PERMISSION_DENIED;
        }
    }
    window = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    context->entry = gtk_entry_new();
    g_object_set_data(G_OBJECT(context->entry), "umicom-automation-id", "studio.prompt.value");
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    accept = gtk_button_new_with_label(
        accept_label != NULL ? accept_label : "OK");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (window == NULL || root == NULL || context->entry == NULL ||
        buttons == NULL || cancel == NULL || accept == NULL) {
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    g_object_set_data(G_OBJECT(accept), "umicom-automation-id", "studio.prompt.accept");
    context->runtime = runtime;
    context->window = window;
    context->accepted = accepted;
    context->ownerRequest = runtime_file_dialog_request_new(runtime, 0U);
    if (context->ownerRequest == NULL) {
        gtk_window_destroy(GTK_WINDOW(window));
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(window),
        title != NULL ? title : "Umicom Studio");
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        runtime->owner->window);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), 540, 220);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message != NULL && message[0] != '\0') {
        gtk_box_append(GTK_BOX(root), runtime_label(message, 0.0F));
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (initial_value != NULL) {
        gtk_editable_set_text(
            GTK_EDITABLE(context->entry),
            initial_value);
    }

    gtk_box_append(GTK_BOX(root), context->entry);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), accept);
    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(GTK_WINDOW(window), root);

    /* Button closures follow the dialog object, not a freed context pointer.
     * A retained button cannot operate on a dialog that has been dismissed. */
    g_signal_connect_object(cancel, "clicked", G_CALLBACK(RuntimePromptCancelClicked), window, 0);
    g_signal_connect_object(accept, "clicked", G_CALLBACK(RuntimePromptAcceptClicked), window, 0);
    g_object_set_data_full(
        G_OBJECT(window),
        "umicom-prompt-context",
        context,
        runtime_prompt_context_destroy);

    UmiStudioFileDialogRequest *presentation = runtime_file_dialog_request_new(runtime, 0U);
    if (presentation == NULL) {
        gtk_window_destroy(GTK_WINDOW(window));
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    /* Present and focus can notify observers that close the dialog or host.
     * Retain only the GTK objects and resolve the host again after each call. */
    GtkWidget *entry = g_object_ref(context->entry);
    g_object_ref(window);
    gtk_window_present(GTK_WINDOW(window));
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *current = runtime_file_dialog_owner(presentation, &ownerWindow);
    if (current != NULL && gtk_widget_get_root(entry) == GTK_ROOT(window) &&
        gtk_widget_get_visible(window)) gtk_widget_grab_focus(entry);
    g_clear_object(&ownerWindow);
    current = runtime_file_dialog_owner(presentation, &ownerWindow);
    UmiStatus status = current != NULL && gtk_widget_get_root(entry) == GTK_ROOT(window) &&
        gtk_widget_get_visible(window) ? UMI_STATUS_OK : UMI_STATUS_CANCELLED;
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(presentation);
    g_object_unref(entry);
    g_object_unref(window);
    return status;
}

/* Existing prompts retain their original policy. Project-entry prompts opt in
 * to the captured-workspace check without changing unrelated operations. */
static UmiStatus runtime_prompt_text(UmiStudioGtkRuntimeChrome *runtime,
    const char *title, const char *message, const char *initial_value,
    const char *accept_label, UmiStudioGtkPromptAccepted accepted)
{
    return runtime_prompt_text_internal(runtime, title, message, initial_value,
        accept_label, accepted, 0);
}
