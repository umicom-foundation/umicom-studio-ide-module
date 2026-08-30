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
} UmiStudioGtkPromptContext;

static void runtime_prompt_context_destroy(gpointer data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)data;
    free(context);
}

static void runtime_prompt_cancelled(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)user_data;
    (void)button;

    if (context != NULL && GTK_IS_WINDOW(context->window)) {
        gtk_window_destroy(GTK_WINDOW(context->window));
    }
}

static void runtime_prompt_accepted(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkPromptContext *context =
        (UmiStudioGtkPromptContext *)user_data;
    const char *value;
    UmiStatus status;
    (void)button;

    if (context == NULL || context->runtime == NULL ||
        context->entry == NULL || context->accepted == NULL) {
        return;
    }

    value = gtk_editable_get_text(GTK_EDITABLE(context->entry));
    if (value == NULL || value[0] == '\0') {
        runtime_show_notification(
            context->runtime,
            "A value is required.");
        return;
    }

    status = context->accepted(context->runtime, value);
    if (status == UMI_STATUS_OK && GTK_IS_WINDOW(context->window)) {
        gtk_window_destroy(GTK_WINDOW(context->window));
    }
}

static UmiStatus runtime_prompt_text(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *title,
    const char *message,
    const char *initial_value,
    const char *accept_label,
    UmiStudioGtkPromptAccepted accepted)
{
    UmiStudioGtkPromptContext *context;
    GtkWidget *window;
    GtkWidget *root;
    GtkWidget *buttons;
    GtkWidget *cancel;
    GtkWidget *accept;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL || accepted == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    context = (UmiStudioGtkPromptContext *)calloc(
        1U,
        sizeof(*context));
    if (context == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    window = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    context->entry = gtk_entry_new();
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    accept = gtk_button_new_with_label(
        accept_label != NULL ? accept_label : "OK");

    if (window == NULL || root == NULL || context->entry == NULL ||
        buttons == NULL || cancel == NULL || accept == NULL) {
        free(context);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    context->runtime = runtime;
    context->window = window;
    context->accepted = accepted;

    gtk_window_set_title(
        GTK_WINDOW(window),
        title != NULL ? title : "Umicom Studio");
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        runtime->owner->window);
    gtk_window_set_default_size(GTK_WINDOW(window), 540, 220);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    if (message != NULL && message[0] != '\0') {
        gtk_box_append(GTK_BOX(root), runtime_label(message, 0.0F));
    }

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
