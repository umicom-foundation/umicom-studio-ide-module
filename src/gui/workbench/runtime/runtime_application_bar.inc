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

/* Classify commands by the objects they reveal. Prefix search can then narrow
 * a large IDE inventory without Studio implementing another search language. */
static UmiWsCommandScope runtime_command_bar_scope(
    const UmiStudioGtkCommandProfile *profile)
{
    if (profile == NULL) return UMI_WS_COMMAND_SCOPE_ALL;
    if (strcmp(profile->profile_id, "ai") == 0 ||
        strcmp(profile->profile_id, "ai_review") == 0) {
        return UMI_WS_COMMAND_SCOPE_AI;
    }
    if (strcmp(profile->profile_id, "explorer") == 0 ||
        strcmp(profile->profile_id, "problems") == 0 ||
        strcmp(profile->profile_id, "terminal") == 0 ||
        strcmp(profile->profile_id, "tests") == 0 ||
        strcmp(profile->profile_id, "debug_view") == 0) {
        return UMI_WS_COMMAND_SCOPE_PANEL;
    }
    return UMI_WS_COMMAND_SCOPE_COMMAND;
}

/* Build Studio's portable command catalogue from the existing profile tables.
 * Those tables remain the single source for labels, commands and help text. */
static UmiStatus runtime_build_command_bar_model(
    UmiStudioGtkRuntimeChrome *runtime)
{
    size_t index;
    UmiStatus status = UMI_STATUS_OK;

    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_ws_command_bar_model_init(&runtime->command_bar_model);

    for (index = 0U;
         index < sizeof(RUNTIME_COMMAND_PROFILES) /
                     sizeof(RUNTIME_COMMAND_PROFILES[0]);
         ++index) {
        const UmiStudioGtkCommandProfile *profile =
            &RUNTIME_COMMAND_PROFILES[index];
        status = umi_ws_command_bar_model_add(
            &runtime->command_bar_model,
            profile->command_id,
            profile->label,
            profile->tooltip,
            profile->command_id,
            profile->category,
            runtime_command_bar_scope(profile),
            80U);
        if (status != UMI_STATUS_OK) return status;

        /* Disabled commands stay searchable and documented, but the shared
         * renderer prevents activation until Framework context enables them. */
        (void)umi_ws_command_bar_model_set_enabled(
            &runtime->command_bar_model,
            profile->command_id,
            runtime_command_enabled(runtime, profile->command_id) != 0);
    }

    for (index = 0U;
         index < sizeof(RUNTIME_LAYOUT_PROFILES) /
                     sizeof(RUNTIME_LAYOUT_PROFILES[0]);
         ++index) {
        const UmiStudioGtkLayoutProfile *profile =
            &RUNTIME_LAYOUT_PROFILES[index];
        status = umi_ws_command_bar_model_add(
            &runtime->command_bar_model,
            profile->command_id,
            profile->label,
            profile->description,
            profile->command_id,
            "layout workspace perspective",
            UMI_WS_COMMAND_SCOPE_COMMAND,
            60U);
        if (status != UMI_STATUS_OK) return status;
        (void)umi_ws_command_bar_model_set_enabled(
            &runtime->command_bar_model,
            profile->command_id,
            runtime_command_enabled(runtime, profile->command_id) != 0);
    }
    return UMI_STATUS_OK;
}

/* The shared component reports semantic command identifiers. Studio routes
 * them through its established registry so shortcuts, menus and search behave
 * exactly like the same action selected from the new inline command bar. */
static void runtime_on_command_bar_activated(
    const UmiWsCommandBarItem *item,
    void *user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;

    if (runtime == NULL || item == NULL || !item->enabled) return;
    (void)runtime_route_command(runtime, item->command_id, NULL);
}

/* Give the reusable component a proportional part of the header. Its portable
 * policy chooses full, compact or button presentation for the resulting width. */
static void runtime_on_application_bar_width_changed(
    GObject *object,
    GParamSpec *property,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int width;
    int command_width;
    (void)property;

    if (runtime == NULL || runtime->command_bar == NULL || object == NULL) {
        return;
    }
    width = gtk_widget_get_width(GTK_WIDGET(object));
    if (width <= 0) return;
    command_width = width / 4;
    if (command_width > 420) command_width = 420;
    (void)umi_gtk4_ws_command_bar_set_available_width(
        runtime->command_bar, command_width);
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

/* Ask the Framework-owned identity component to compose the SVG mark and
 * native application name. Studio supplies only its product identity. */
static GtkWidget *runtime_build_brand_identity(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiAppearanceProfile appearance;
    UmiUiWorkbench *workbench;
    UmiGtk4WorkstationShellHeaderConfig config;
    GtkWidget *identity;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) return NULL;
    config = umi_gtk4_ws_shell_header_config_default(
        "org.umicom.studio", "Umicom Studio IDE");
    config.compact = true;
    status = umi_gtk4_ws_shell_header_create_managed(
        &config, &runtime->brand_identity);
    if (status != UMI_STATUS_OK) return NULL;
    identity = umi_gtk4_ws_shell_header_widget(runtime->brand_identity);

    /* The workbench owns the appearance model; the shared header copies the
     * selected SVG resource and safely falls back to text if it is absent. */
    workbench = umi_studio_ui_workbench(runtime->owner->ui);
    if (workbench != NULL &&
        umi_ui_appearance_model_active(
            umi_ui_workbench_appearance(workbench), &appearance) ==
            UMI_STATUS_OK) {
        (void)umi_gtk4_ws_shell_header_apply_appearance(
            runtime->brand_identity, &appearance);
        runtime->appearance_revision = umi_ui_appearance_model_revision(
            umi_ui_workbench_appearance(workbench));
    }

    /* Keep Studio's spacing hook while all identity content uses Framework
     * classes shared with the suite workstations. */
    gtk_widget_add_css_class(identity, "umicom-runtime-brand");
    return identity;
}

/* Refresh the icon only when Studio's toolkit-neutral appearance model has
 * changed. The regular service timer makes direct and restored choices visible
 * without adding a second product-specific theme event system. */
static UmiStatus runtime_refresh_brand_appearance(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiAppearanceModel *model;
    UmiUiAppearanceProfile appearance;
    UmiUiWorkbench *workbench;
    uint64_t revision;
    UmiStatus status;

    if (runtime == NULL || runtime->brand_identity == NULL ||
        runtime->owner == NULL || runtime->owner->ui == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    workbench = umi_studio_ui_workbench(runtime->owner->ui);
    if (workbench == NULL) return UMI_STATUS_INVALID_STATE;
    model = umi_ui_workbench_appearance(workbench);
    if (model == NULL) return UMI_STATUS_INVALID_STATE;
    revision = umi_ui_appearance_model_revision(model);
    if (revision == runtime->appearance_revision) return UMI_STATUS_OK;

    status = umi_ui_appearance_model_active(model, &appearance);
    if (status == UMI_STATUS_OK) {
        status = umi_gtk4_ws_shell_header_apply_appearance(
            runtime->brand_identity, &appearance);
    }
    if (status == UMI_STATUS_OK) runtime->appearance_revision = revision;
    return status;
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
    UmiGtk4WorkstationCommandBarConfig command_config;
    UmiStatus command_status;

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
    command_status = runtime_build_command_bar_model(runtime);
    if (command_status == UMI_STATUS_OK) {
        command_config = umi_gtk4_ws_command_bar_config_default();
        command_config.placeholder = "Search Studio commands and windows";
        command_config.compact_placeholder = "Quick action";
        command_config.initial_available_width = 220;
        command_status = umi_gtk4_ws_command_bar_create_managed(
            &command_config,
            &runtime->command_bar_model,
            &runtime->command_bar);
    }
    if (command_status == UMI_STATUS_OK) {
        command_status = umi_gtk4_ws_command_bar_set_activated_handler(
            runtime->command_bar,
            runtime_on_command_bar_activated,
            runtime);
    }

    if (bar == NULL || menu_scroll == NULL || brand == NULL ||
        commands == NULL || layouts == NULL || new_window == NULL ||
        command_status != UMI_STATUS_OK || runtime->command_bar == NULL) {
        /* None are parented yet, so release every successful allocation. */
        if (bar != NULL) g_object_unref(bar);
        if (menu_scroll != NULL) g_object_unref(menu_scroll);
        if (brand != NULL) g_object_unref(brand);
        if (commands != NULL) g_object_unref(commands);
        if (layouts != NULL) g_object_unref(layouts);
        if (new_window != NULL) g_object_unref(new_window);
        umi_gtk4_ws_command_bar_destroy(runtime->command_bar);
        runtime->command_bar = NULL;
        return NULL;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-appbar");
    g_signal_connect(
        bar,
        "notify::width",
        G_CALLBACK(runtime_on_application_bar_width_changed),
        runtime);

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
    /* Search sits after the menus, following the reading order while keeping
     * project content and status messages to its right. */
    gtk_box_append(
        GTK_BOX(bar),
        umi_gtk4_ws_command_bar_widget(runtime->command_bar));
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
