/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_application_bar.inc
 *
 * PURPOSE:
 *   Create a compact professional application bar with command/layout search.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
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

static GtkWidget *runtime_build_application_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *brand;
    GtkWidget *spacer;
    GtkWidget *commands;
    GtkWidget *layouts;

    if (runtime == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    brand = gtk_label_new("Umicom Studio");
    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    commands = gtk_button_new_with_label("Commands  Ctrl+Shift+P");
    layouts = gtk_button_new_with_label("Layouts  Ctrl+Shift+L");

    if (bar == NULL || brand == NULL || spacer == NULL ||
        commands == NULL || layouts == NULL) {
        return bar;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-appbar");
    gtk_widget_add_css_class(brand, "umicom-runtime-accent");
    gtk_widget_set_hexpand(spacer, TRUE);

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

    gtk_box_append(GTK_BOX(bar), brand);
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
    gtk_box_append(GTK_BOX(bar), layouts);

    return bar;
}
