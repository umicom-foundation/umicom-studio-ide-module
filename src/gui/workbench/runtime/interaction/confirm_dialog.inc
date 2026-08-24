/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/confirm_dialog.inc
 *
 * PURPOSE:
 *   Provide a small GTK confirmation presenter for destructive UI decisions.
 *   The underlying close/discard behavior continues to live in Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

typedef UmiStatus (*UmiStudioGtkConfirmAccepted)(
    UmiStudioGtkRuntimeChrome *runtime);

typedef struct UmiStudioGtkConfirmContext {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *window;
    UmiStudioGtkConfirmAccepted accepted;
} UmiStudioGtkConfirmContext;

static void runtime_confirm_context_destroy(gpointer data)
{
    free(data);
}

static void runtime_confirm_cancel(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkConfirmContext *context =
        (UmiStudioGtkConfirmContext *)user_data;
    (void)button;

    if (context != NULL && GTK_IS_WINDOW(context->window)) {
        gtk_window_destroy(GTK_WINDOW(context->window));
    }
}

static void runtime_confirm_accept(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkConfirmContext *context =
        (UmiStudioGtkConfirmContext *)user_data;
    UmiStatus status;
    (void)button;

    if (context == NULL || context->runtime == NULL ||
        context->accepted == NULL) {
        return;
    }

    status = context->accepted(context->runtime);
    if (status == UMI_STATUS_OK && GTK_IS_WINDOW(context->window)) {
        gtk_window_destroy(GTK_WINDOW(context->window));
    }
}

static UmiStatus runtime_confirm(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *title,
    const char *message,
    const char *accept_label,
    UmiStudioGtkConfirmAccepted accepted)
{
    UmiStudioGtkConfirmContext *context;
    GtkWidget *window;
    GtkWidget *root;
    GtkWidget *buttons;
    GtkWidget *cancel;
    GtkWidget *accept;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL || accepted == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    context = (UmiStudioGtkConfirmContext *)calloc(
        1U,
        sizeof(*context));
    if (context == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    window = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    accept = gtk_button_new_with_label(
        accept_label != NULL ? accept_label : "Continue");

    if (window == NULL || root == NULL || buttons == NULL ||
        cancel == NULL || accept == NULL) {
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    context->runtime = runtime;
    context->window = window;
    context->accepted = accepted;

    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        runtime->owner->window);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 200);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    gtk_box_append(
        GTK_BOX(root),
        runtime_label(message, 0.0F));
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
    g_object_set_data_full(
        G_OBJECT(window),
        "umicom-confirm-context",
        context,
        runtime_confirm_context_destroy);

    gtk_window_present(GTK_WINDOW(window));
    return UMI_STATUS_OK;
}
