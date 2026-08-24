/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_layout_browser.inc
 *
 * PURPOSE:
 *   Present and switch the seven Framework semantic Studio layouts.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_layout_button(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-id");

    if (runtime == NULL || command_id == NULL) return;

    gtk_widget_set_visible(runtime->layout_window, FALSE);
    (void)runtime_execute_command(runtime, command_id, NULL);
}

static UmiStatus runtime_build_layout_browser(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    size_t index;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->layout_window = GTK_WIDGET(gtk_window_new());
    runtime->layout_list = gtk_list_box_new();
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    if (runtime->layout_window == NULL ||
        runtime->layout_list == NULL ||
        root == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(runtime->layout_window),
        "Studio Layout Browser");
    gtk_window_set_default_size(
        GTK_WINDOW(runtime->layout_window), 560, 480);
    gtk_window_set_modal(GTK_WINDOW(runtime->layout_window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(runtime->layout_window),
        runtime->owner->window);

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);

    gtk_box_append(
        GTK_BOX(root),
        runtime_section_title("Framework semantic layouts"));

    for (index = 0U;
         index < sizeof(RUNTIME_LAYOUT_PROFILES) /
                 sizeof(RUNTIME_LAYOUT_PROFILES[0]);
         ++index) {
        const UmiStudioGtkLayoutProfile *profile =
            &RUNTIME_LAYOUT_PROFILES[index];
        GtkWidget *button = gtk_button_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        GtkWidget *title = runtime_label(profile->label, 0.0F);
        GtkWidget *detail = runtime_label(profile->description, 0.0F);

        if (button == NULL || box == NULL ||
            title == NULL || detail == NULL) {
            continue;
        }

        gtk_widget_add_css_class(detail, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(box), title);
        gtk_box_append(GTK_BOX(box), detail);
        gtk_button_set_child(GTK_BUTTON(button), box);

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(profile->command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_layout_button),
            runtime);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, profile->command_id));

        gtk_list_box_append(
            GTK_LIST_BOX(runtime->layout_list), button);
    }

    gtk_box_append(GTK_BOX(root), runtime_scroll(runtime->layout_list));
    gtk_window_set_child(GTK_WINDOW(runtime->layout_window), root);

    g_signal_connect(
        runtime->layout_window,
        "close-request",
        G_CALLBACK(runtime_on_palette_close),
        runtime);

    gtk_widget_set_visible(runtime->layout_window, FALSE);
    return UMI_STATUS_OK;
}

static void runtime_show_layout_browser(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime != NULL && runtime->layout_window != NULL) {
        gtk_window_present(GTK_WINDOW(runtime->layout_window));
    }
}
