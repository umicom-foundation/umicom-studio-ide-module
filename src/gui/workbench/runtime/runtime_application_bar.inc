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

#ifdef G_OS_WIN32
#include <glib/gwin32.h>
#endif

/* Open the searchable command list when the compact search button is used. */
static void runtime_on_commands_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_command_palette(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

/* Open saved layout choices without placing a long text button in the bar. */
static void runtime_on_layouts_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

/* Open the window catalogue where panels can be added, moved or restored. */
static void runtime_on_new_window_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_window_catalogue(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

/* Create a compact icon button while keeping a readable accessible name and
 * tooltip for keyboard, screen-reader and first-time users. */
static GtkWidget *runtime_application_bar_icon_button(
    const char *icon_name,
    const char *accessible_name,
    const char *tooltip)
{
    GtkWidget *button;

    if (icon_name == NULL || accessible_name == NULL || tooltip == NULL) {
        return NULL;
    }
    button = gtk_button_new_from_icon_name(icon_name);
    if (button == NULL) return NULL;
    gtk_widget_add_css_class(button, "flat");
    gtk_widget_add_css_class(button, "umicom-runtime-appbar-action");
    gtk_widget_set_tooltip_text(button, tooltip);
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(button),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        accessible_name,
        -1);
    return button;
}

/* Find the directory that contains the running Studio executable. Packaged
 * brand resources live beside that executable rather than beside the user's
 * current terminal directory. */
static char *runtime_executable_directory(void)
{
#ifdef G_OS_WIN32
    return g_win32_get_package_installation_directory_of_module(NULL);
#else
    GError *error = NULL;
    char *executable = g_file_read_link("/proc/self/exe", &error);
    char *directory = NULL;

    if (executable != NULL) directory = g_path_get_dirname(executable);
    g_free(executable);
    if (error != NULL) g_error_free(error);
    return directory;
#endif
}

/* Resolve a logical relative resource against the executable when it is not
 * already available from the current directory. The caller owns the result. */
static char *runtime_resolve_brand_resource(const char *resource)
{
    char *directory;
    char *candidate;

    if (resource == NULL || resource[0] == '\0') return NULL;
    if (g_file_test(resource, G_FILE_TEST_IS_REGULAR)) {
        return g_strdup(resource);
    }

    directory = runtime_executable_directory();
    candidate = directory != NULL
        ? g_build_filename(directory, resource, NULL)
        : NULL;
    g_free(directory);
    if (candidate != NULL &&
        g_file_test(candidate, G_FILE_TEST_IS_REGULAR)) {
        return candidate;
    }
    g_free(candidate);
    return NULL;
}

/* Compose the application identity from the reusable mark and a native text
 * label. Keeping these separate makes the name accessible and translatable. */
static GtkWidget *runtime_build_brand_identity(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiAppearanceProfile appearance;
    UmiUiWorkbench *workbench;
    GtkWidget *identity;
    GtkWidget *title;
    char *icon_path = NULL;

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
        appearance.icon_resource[0] != '\0' &&
        (icon_path = runtime_resolve_brand_resource(
            appearance.icon_resource)) != NULL) {
        GtkWidget *icon = gtk_picture_new_for_filename(icon_path);
        g_free(icon_path);
        if (icon != NULL) {
            /* The menu identity uses only the SVG mark; the adjacent native
             * label renders the application name as real text. */
            gtk_widget_set_size_request(icon, 18, 18);
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

/* Build the top-level menu strip and its compact workspace actions. The menu
 * receives flexible width so the central editor is never forced off-screen. */
static GtkWidget *runtime_build_application_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *menu_scroll;
    GtkWidget *commands;
    GtkWidget *layouts;
    GtkWidget *new_window;
    GtkWidget *brand;

    if (runtime == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    menu_scroll = gtk_scrolled_window_new();
    commands = runtime_application_bar_icon_button(
        "system-search-symbolic",
        "Search commands",
        "Search commands and actions (Ctrl+Shift+P)");
    layouts = runtime_application_bar_icon_button(
        "view-grid-symbolic",
        "Manage layouts",
        "Switch or manage layouts (Ctrl+Shift+L)");
    new_window = runtime_application_bar_icon_button(
        "window-new-symbolic",
        "Add a window",
        "Add, show, hide or move Framework panels in the current layout");
    brand = runtime_build_brand_identity(runtime);

    if (bar == NULL || menu_scroll == NULL || brand == NULL ||
        commands == NULL || layouts == NULL || new_window == NULL) {
        /* None are parented yet, so release every successful allocation. */
        if (bar != NULL) g_object_unref(bar);
        if (menu_scroll != NULL) g_object_unref(menu_scroll);
        if (brand != NULL) g_object_unref(brand);
        if (commands != NULL) g_object_unref(commands);
        if (layouts != NULL) g_object_unref(layouts);
        if (new_window != NULL) g_object_unref(new_window);
        return NULL;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-appbar");

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
        gtk_label_set_ellipsize(
            GTK_LABEL(runtime->notification_label), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(
            GTK_LABEL(runtime->notification_label), 24);
    }

    runtime->menu_bar = runtime_build_menu_bar(runtime);

    gtk_box_append(GTK_BOX(bar), brand);
    if (runtime->menu_bar != NULL) {
        /* The menu strip may be wider than a small monitor. A horizontal
         * scroller keeps actions reachable without widening the whole window. */
        gtk_scrolled_window_set_policy(
            GTK_SCROLLED_WINDOW(menu_scroll),
            GTK_POLICY_AUTOMATIC,
            GTK_POLICY_NEVER);
        gtk_scrolled_window_set_propagate_natural_width(
            GTK_SCROLLED_WINDOW(menu_scroll), FALSE);
        gtk_scrolled_window_set_child(
            GTK_SCROLLED_WINDOW(menu_scroll), runtime->menu_bar);
        gtk_widget_set_hexpand(menu_scroll, TRUE);
        gtk_widget_add_css_class(
            menu_scroll, "umicom-runtime-menu-scroll");
        gtk_box_append(GTK_BOX(bar), menu_scroll);
    } else {
        /* A missing menu registry should not leak the unparented scroller. */
        g_object_unref(menu_scroll);
    }
    if (runtime->notification_label != NULL) {
        gtk_box_append(GTK_BOX(bar), runtime->notification_label);
    }
    if (runtime->workflow_badge != NULL) {
        gtk_box_append(GTK_BOX(bar), runtime->workflow_badge);
    }
    gtk_box_append(GTK_BOX(bar), commands);
    gtk_box_append(GTK_BOX(bar), new_window);
    gtk_box_append(GTK_BOX(bar), layouts);

    return bar;
}
