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

static GtkWidget *runtime_build_brand_identity(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiAppearanceProfile appearance;
    UmiUiWorkbench *workbench;
    GtkWidget *identity;
    GtkWidget *title;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) return NULL;
    identity = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    title = gtk_label_new("Umicom Studio IDE");
    if (identity == NULL || title == NULL) {
        /* Release whichever independently allocated widget did succeed. */
        if (title != NULL) g_object_unref(title);
        if (identity != NULL) g_object_unref(identity);
        return NULL;
    }

    /* Resolve the icon through Framework appearance state, not a source path. */
    workbench = umi_studio_ui_workbench(runtime->owner->ui);
    if (workbench != NULL &&
        umi_ui_appearance_model_active(
            umi_ui_workbench_appearance(workbench), &appearance) ==
            UMI_STATUS_OK &&
        appearance.icon_resource[0] != '\0') {
        GtkWidget *icon = gtk_picture_new_for_filename(
            appearance.icon_resource);
        if (icon != NULL) {
            gtk_widget_set_size_request(icon, 20, 20);
            gtk_picture_set_can_shrink(GTK_PICTURE(icon), TRUE);
            gtk_widget_set_tooltip_text(icon, "Umicom Studio IDE");
            gtk_box_append(GTK_BOX(identity), icon);
        }
    }

    /* The name remains visible when a package is missing its optional image. */
    gtk_widget_add_css_class(identity, "umicom-runtime-brand");
    gtk_widget_add_css_class(title, "umicom-runtime-brand-title");
    gtk_box_append(GTK_BOX(identity), title);
    return identity;
}

static GtkWidget *runtime_build_application_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *spacer;
    GtkWidget *commands;
    GtkWidget *layouts;
    GtkWidget *new_window;
    GtkWidget *brand;

    if (runtime == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    commands = gtk_button_new_with_label("Search Commands");
    layouts = gtk_button_new_with_label("Layouts");
    new_window = gtk_button_new_with_label("New Window");
    brand = runtime_build_brand_identity(runtime);

    if (bar == NULL || spacer == NULL || brand == NULL ||
        commands == NULL || layouts == NULL || new_window == NULL) {
        /* None are parented yet, so release every successful allocation. */
        if (bar != NULL) g_object_unref(bar);
        if (spacer != NULL) g_object_unref(spacer);
        if (brand != NULL) g_object_unref(brand);
        if (commands != NULL) g_object_unref(commands);
        if (layouts != NULL) g_object_unref(layouts);
        if (new_window != NULL) g_object_unref(new_window);
        return NULL;
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
    gtk_box_append(GTK_BOX(bar), new_window);
    gtk_box_append(GTK_BOX(bar), layouts);

    return bar;
}
