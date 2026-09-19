/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/confirm_dialog.inc
 *
 * PURPOSE:
 *   Provide a small GTK confirmation presenter for destructive UI decisions.
 *   The underlying close/discard behavior continues to live in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

typedef UmiStatus (*UmiStudioGtkConfirmAccepted)(
    UmiStudioGtkRuntimeChrome *runtime);

typedef UmiStatus (*UmiStudioGtkDocumentConfirmAccepted)(
    UmiStudioGtkRuntimeChrome *runtime, UmiDocumentId documentId);

typedef UmiStatus (*UmiStudioGtkOwnedConfirmAccepted)(
    UmiStudioGtkRuntimeChrome *runtime, void *data);

typedef struct UmiStudioGtkConfirmContext {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *window;
    UmiStudioGtkConfirmAccepted accepted;
    UmiStudioGtkDocumentConfirmAccepted documentAccepted;
    UmiStudioFileDialogRequest *ownerRequest;
    UmiStudioGtkOwnedConfirmAccepted ownedAccepted;
    void *ownedData;
    GDestroyNotify ownedDestroy;
} UmiStudioGtkConfirmContext;

/*
 * Release or reset state held by runtime confirm context so the same storage can be reused
 * safely.
 */
static void runtime_confirm_context_destroy(gpointer data)
{
    UmiStudioGtkConfirmContext *context = data;
    if (context == NULL) return;
    runtime_file_dialog_request_free(context->ownerRequest);
    if (context->ownedDestroy != NULL) context->ownedDestroy(context->ownedData);
    free(context);
}

/*
 * Provide the runtime confirm cancel operation used by this module and its client
 * applications.
 */
static void runtime_confirm_cancel(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkConfirmContext *context =
        (UmiStudioGtkConfirmContext *)user_data;
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
 * Provide the runtime confirm accept operation used by this module and its client
 * applications.
 */
static void runtime_confirm_accept(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkConfirmContext *context =
        (UmiStudioGtkConfirmContext *)user_data;
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL || (context->accepted == NULL && context->documentAccepted == NULL && context->ownedAccepted == NULL)) return;
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context->ownerRequest, &ownerWindow);
    GtkWindow *dialog = g_object_ref(GTK_WINDOW(context->window));
    if (runtime == NULL) {
        gtk_window_destroy(dialog);
        g_clear_object(&ownerWindow);
        g_object_unref(dialog);
        return;
    }
    status = context->ownedAccepted != NULL
        ? context->ownedAccepted(runtime, context->ownedData)
        : context->documentAccepted != NULL
            ? context->documentAccepted(runtime, context->ownerRequest->documentId)
            : context->accepted(runtime);
    /* A reviewed payload may become stale; close that question after one
     * attempt so the user must request and inspect a fresh plan. */
    if (context->ownedAccepted != NULL && status != UMI_STATUS_OK)
        runtime_show_notification(runtime, status == UMI_STATUS_INVALID_STATE
            ? "The document or saved file changed. Request Reload again to review the current version."
            : umi_status_text(status));
    if (status == UMI_STATUS_OK || context->ownedAccepted != NULL ||
        (context->documentAccepted != NULL && status == UMI_STATUS_NOT_FOUND))
        gtk_window_destroy(dialog);
    else runtime_show_notification(runtime, umi_status_text(status));
    g_object_unref(ownerWindow);
    g_object_unref(dialog);
}

/* Provide the runtime confirm operation used by this module and its client applications. */
static UmiStatus runtime_confirm_full(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *title,
    const char *message,
    const char *accept_label,
    UmiStudioGtkConfirmAccepted accepted,
    UmiStudioGtkDocumentConfirmAccepted documentAccepted,
    UmiDocumentId documentId,
    UmiStudioGtkOwnedConfirmAccepted ownedAccepted,
    void *ownedData, GDestroyNotify ownedDestroy, GtkWidget *reviewContent)
{
    UmiStudioGtkConfirmContext *context;
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
        runtime->owner->window == NULL || (accepted == NULL && documentAccepted == NULL && ownedAccepted == NULL)) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    context = (UmiStudioGtkConfirmContext *)calloc(
        1U,
        sizeof(*context));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    window = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    accept = gtk_button_new_with_label(
        accept_label != NULL ? accept_label : "Continue");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (window == NULL || root == NULL || buttons == NULL ||
        cancel == NULL || accept == NULL) {
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    context->runtime = runtime;
    context->window = window;
    context->accepted = accepted;
    context->documentAccepted = documentAccepted;
    context->ownerRequest = runtime_file_dialog_request_new(runtime, documentId);
    if (context->ownerRequest == NULL) {
        gtk_window_destroy(GTK_WINDOW(window));
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        runtime->owner->window);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), reviewContent != NULL ? 1100 : 500,
        reviewContent != NULL ? 700 : 200);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    gtk_box_append(
        GTK_BOX(root),
        runtime_label(message, 0.0F));
    if (reviewContent != NULL) {
        gtk_widget_set_vexpand(reviewContent, TRUE);
        gtk_box_append(GTK_BOX(root), reviewContent);
    }
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), accept);
    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(GTK_WINDOW(window), root);

    g_signal_connect(
        cancel,
        "clicked",
        G_CALLBACK(runtime_confirm_cancel),
        context);
    g_signal_connect(
        accept,
        "clicked",
        G_CALLBACK(runtime_confirm_accept),
        context);
    context->ownedAccepted = ownedAccepted;
    context->ownedData = ownedData;
    context->ownedDestroy = ownedDestroy;
    gtk_window_set_default_widget(GTK_WINDOW(window), cancel);
    (void)umi_gtk4_automation_tag_widget(accept, "studio.confirm.accept");
    (void)umi_gtk4_automation_tag_widget(cancel, "studio.confirm.cancel");
    g_object_set_data_full(
        G_OBJECT(window),
        "umicom-confirm-context",
        context,
        runtime_confirm_context_destroy);

    gtk_window_present(GTK_WINDOW(window));
    return UMI_STATUS_OK;
}

/* Existing callers retain the same confirmation interface. */
static UmiStatus runtime_confirm(UmiStudioGtkRuntimeChrome *runtime,
    const char *title, const char *message, const char *accept_label,
    UmiStudioGtkConfirmAccepted accepted)
{
    return runtime_confirm_full(runtime, title, message, accept_label, accepted, NULL, 0U, NULL, NULL, NULL, NULL);
}

/* Capture the document identity when the question is asked, not when answered. */
static UmiStatus runtime_confirm_document(UmiStudioGtkRuntimeChrome *runtime,
    const char *title, const char *message, const char *accept_label,
    UmiDocumentId documentId, UmiStudioGtkDocumentConfirmAccepted accepted)
{
    return runtime_confirm_full(runtime, title, message, accept_label, NULL, accepted, documentId, NULL, NULL, NULL, NULL);
}

/* On success the dialog owns data until acceptance, cancellation or teardown.
 * On failure the caller retains it and must perform the matching cleanup. */
static UmiStatus runtime_confirm_owned(UmiStudioGtkRuntimeChrome *runtime,
    const char *title, const char *message, const char *acceptLabel,
    UmiDocumentId documentId, UmiStudioGtkOwnedConfirmAccepted accepted,
    void *data, GDestroyNotify destroy, GtkWidget *reviewContent)
{
    return runtime_confirm_full(runtime, title, message, acceptLabel,
        NULL, NULL, documentId, accepted, data, destroy, reviewContent);
}
