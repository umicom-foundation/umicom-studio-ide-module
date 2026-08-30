/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_application_bar.inc
 *
 * PURPOSE:
 *   Create a compact professional application bar with command/layout search.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_commands_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_command_palette(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

static void runtime_on_layouts_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

static void runtime_on_new_window_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_window_catalogue(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

static GtkWidget *runtime_build_application_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *spacer;
    GtkWidget *commands;
    GtkWidget *layouts;
    GtkWidget *new_window;

    if (runtime == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    commands = gtk_button_new_with_label("Search Commands");
    layouts = gtk_button_new_with_label("Layouts");
    new_window = gtk_button_new_with_label("New Window");

    if (bar == NULL || spacer == NULL ||
        commands == NULL || layouts == NULL || new_window == NULL) {
        return bar;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-appbar");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_widget_set_tooltip_text(
        commands, "Search commands and actions (Ctrl+Shift+P)");
    gtk_widget_set_tooltip_text(
        layouts, "Switch or manage layouts (Ctrl+Shift+L)");
    gtk_widget_set_tooltip_text(
        new_window,
        "Add, show, hide or move Framework panels in the current layout");
    gtk_widget_add_css_class(commands, "flat");
    gtk_widget_add_css_class(layouts, "flat");
    gtk_widget_add_css_class(new_window, "flat");

    g_signal_connect(
        commands,
        "clicked",
        G_CALLBACK(runtime_on_commands_clicked),
        runtime);
    g_signal_connect(
        layouts,
        "clicked",
        G_CALLBACK(runtime_on_layouts_clicked),
        runtime);
    g_signal_connect(
        new_window,
        "clicked",
        G_CALLBACK(runtime_on_new_window_clicked),
        runtime);

    runtime->workflow_badge = gtk_label_new("Workflow");
    runtime->notification_label = gtk_label_new("");
    if (runtime->workflow_badge != NULL) {
        gtk_widget_add_css_class(
            runtime->workflow_badge,
            "umicom-runtime-muted");
    }
    if (runtime->notification_label != NULL) {
        gtk_widget_add_css_class(
            runtime->notification_label,
            "umicom-runtime-muted");
        gtk_widget_set_visible(runtime->notification_label, FALSE);
    }

    runtime->menu_bar = runtime_build_menu_bar(runtime);

    if (runtime->menu_bar != NULL) {
        gtk_box_append(GTK_BOX(bar), runtime->menu_bar);
    }
    if (runtime->notification_label != NULL) {
        gtk_box_append(GTK_BOX(bar), runtime->notification_label);
    }
    gtk_box_append(GTK_BOX(bar), spacer);
    if (runtime->workflow_badge != NULL) {
        gtk_box_append(GTK_BOX(bar), runtime->workflow_badge);
    }
    gtk_box_append(GTK_BOX(bar), commands);
    gtk_box_append(GTK_BOX(bar), new_window);
    gtk_box_append(GTK_BOX(bar), layouts);

    return bar;
}
