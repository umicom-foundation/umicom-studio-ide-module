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
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (profile == NULL) return UMI_WS_COMMAND_SCOPE_ALL;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(profile->profile_id, "ai") == 0 ||
        strcmp(profile->profile_id, "ai_review") == 0) {
        return UMI_WS_COMMAND_SCOPE_AI;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(profile->profile_id, "explorer") == 0 ||
        strcmp(profile->profile_id, "problems") == 0 ||
        strcmp(profile->profile_id, "terminal") == 0 ||
        strcmp(profile->profile_id, "tests") == 0 ||
        strcmp(profile->profile_id, "debug_view") == 0) {
        return UMI_WS_COMMAND_SCOPE_PANEL;
    }
    return UMI_WS_COMMAND_SCOPE_COMMAND;
}

/* A panel already receives a normal Open/Focus entry from the Framework
 * surface catalogue; avoid a second search row for its toolbar alias. */
static int runtime_command_has_surface(const char *command_id)
{
    size_t index;
    for (index = 0U; index < sizeof(RUNTIME_SURFACE_PROFILES) /
            sizeof(RUNTIME_SURFACE_PROFILES[0]); ++index) {
        const char *surface_command = RUNTIME_SURFACE_PROFILES[index].command_id;
        if (surface_command[0] != '\0' && strcmp(surface_command, command_id) == 0) return 1;
    }
    return 0;
}

/* Build Studio's portable command catalogue from the existing profile tables.
 * Those tables remain the single source for labels, commands and help text. */
static UmiStatus runtime_build_command_bar_model(
    UmiStudioGtkRuntimeChrome *runtime)
{
    size_t index;
    UmiStatus status = UMI_STATUS_OK;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_ws_command_bar_model_init(&runtime->command_bar_model);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_COMMAND_PROFILES) /
                     sizeof(RUNTIME_COMMAND_PROFILES[0]);
         ++index) {
        const UmiStudioGtkCommandProfile *profile =
            &RUNTIME_COMMAND_PROFILES[index];
        if (runtime_command_has_surface(profile->command_id)) continue;
        status = umi_ws_command_bar_model_add(
            &runtime->command_bar_model,
            profile->command_id,
            profile->label,
            profile->tooltip,
            profile->command_id,
            profile->category,
            runtime_command_bar_scope(profile),
            80U);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;

        /* Disabled commands stay searchable and documented, but the shared
         * renderer prevents activation until Framework context enables them. */
        (void)umi_ws_command_bar_model_set_enabled(
            &runtime->command_bar_model,
            profile->command_id,
            runtime_command_enabled(runtime, profile->command_id) != 0);
    }

    /* Every real native surface, including Editor, is reachable in normal
     * mode. Placement remains a separate transaction in the existing editor. */
    for (index = 0U; index < umi_studio_surface_catalogue_count(); ++index) {
        const UmiStudioRuntimeSurfaceBinding *binding = umi_studio_surface_catalogue_at(index);
        const char *id;
        char item_id[UMI_UI_ID_CAPACITY];
        int written;
        if (binding == NULL || runtime_surface_profile(binding->kind) == NULL) continue;
        id = umi_studio_runtime_workspace_surface_id(binding->kind);
        if (id == NULL) continue;
        written = snprintf(item_id, sizeof(item_id), "studio.window.%s", id);
        if (written < 0 || (size_t)written >= sizeof(item_id)) return UMI_STATUS_CAPACITY_EXCEEDED;
        status = umi_ws_command_bar_model_add(&runtime->command_bar_model,
            item_id, binding->title,
            "Open or focus this existing Studio panel. No layout unlock is needed.",
            id, "window tool panel open focus restore", UMI_WS_COMMAND_SCOPE_PANEL, 95U);
        if (status != UMI_STATUS_OK) return status;
    }

    /* Visit each bounded item once so every record receives the same rule. */
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
        /* Preserve the original failure result so the caller can respond to the correct cause. */
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || item == NULL || !item->enabled) return;
    if (strncmp(item->item_id, "studio.window.", sizeof("studio.window.") - 1U) == 0) {
        UmiStudioRuntimeSurfaceKind kind;
        UmiStatus status = umi_studio_runtime_workspace_surface_kind(item->command_id, &kind);
        if (status == UMI_STATUS_OK) status = runtime_workspace_open_surface(runtime, kind, NULL);
        if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
        return;
    }
    (void)runtime_route_command(runtime, item->command_id, NULL);
}

/* Give the reusable component a proportional part of the header. Its portable
 * policy chooses full, compact or button presentation for the resulting width. */
static void runtime_update_application_bar_width(
    UmiStudioGtkRuntimeChrome *runtime)
{
    int width;
    int command_width;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->command_bar == NULL ||
        runtime->application_bar == NULL) {
        return;
    }
    width = gtk_widget_get_width(runtime->application_bar);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (width <= 0 || width == runtime->application_bar_width) return;
    runtime->application_bar_width = width;
    command_width = width / 4;
    /* Apply this branch only when its contract condition is satisfied. */
    if (command_width > 420) command_width = 420;
    (void)umi_gtk4_ws_command_bar_set_available_width(
        runtime->command_bar, command_width);
    /* Keep menus and search reachable when there is less room. The
     * complete workflow report remains available in the status surfaces. */
    if (runtime->workflow_badge != NULL) {
        gtk_widget_set_visible(runtime->workflow_badge, width >= 900);
    }
}

/* Create a compact icon button while keeping a readable accessible name and
 * tooltip for keyboard, screen-reader and first-time users. */
static GtkWidget *runtime_application_bar_icon_button(
    const char *icon_name,
    const char *accessible_name,
    const char *tooltip)
{
    GtkWidget *button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (icon_name == NULL || accessible_name == NULL || tooltip == NULL) {
        return NULL;
    }
    button = gtk_button_new_from_icon_name(icon_name);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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

/* Refresh the shared top-level title bar from the existing appearance model.
 * The command strip contains actions only; it never owns application branding. */
static UmiStatus runtime_refresh_brand_appearance(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_studio_gtk_workbench_refresh_titlebar(runtime->owner);
}

/* Keep menus directly below the real window title bar. Search and workspace
 * actions share this row; scrolling the menu does not widen the editor. */
static GtkWidget *runtime_build_application_bar(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *menu_scroll;
    GtkWidget *commands;
    GtkWidget *layouts;
    GtkWidget *new_window;
    UmiGtk4WorkstationCommandBarConfig config;
    UmiStatus status;
    if (runtime == NULL) return NULL;
    status = runtime_build_command_bar_model(runtime);
    if (status != UMI_STATUS_OK) return NULL;
    config = umi_gtk4_ws_command_bar_config_default();
    config.placeholder = "Search Studio commands and windows";
    config.compact_placeholder = "Quick action";
    config.initial_available_width = 220;
    status = umi_gtk4_ws_command_bar_create_managed(&config,
        &runtime->command_bar_model, &runtime->command_bar);
    if (status != UMI_STATUS_OK) return NULL;
    status = umi_gtk4_ws_command_bar_set_activated_handler(runtime->command_bar,
        runtime_on_command_bar_activated, runtime);
    if (status != UMI_STATUS_OK) {
        umi_gtk4_ws_command_bar_destroy(runtime->command_bar);
        runtime->command_bar = NULL;
        return NULL;
    }
    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    menu_scroll = gtk_scrolled_window_new();
    gtk_widget_add_css_class(bar, "umicom-runtime-appbar");
    (void)umi_gtk4_automation_tag_widget(bar, "studio.workspace.command-strip");
    runtime->menu_bar = runtime_build_menu_bar(runtime);
    /* GTK owns these ordinary widgets; the menu's natural width never becomes
     * a minimum width for the whole application. Keyboard navigation remains. */
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(menu_scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(menu_scroll), FALSE);
    gtk_widget_set_hexpand(menu_scroll, TRUE);
    gtk_widget_add_css_class(menu_scroll, "umicom-runtime-menu-scroll");
    if (runtime->menu_bar != NULL)
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(menu_scroll), runtime->menu_bar);
    gtk_box_append(GTK_BOX(bar), menu_scroll);
    gtk_box_append(GTK_BOX(bar), umi_gtk4_ws_command_bar_widget(runtime->command_bar));
    runtime->notification_label = gtk_label_new("");
    gtk_widget_set_visible(runtime->notification_label, FALSE);
    gtk_label_set_ellipsize(GTK_LABEL(runtime->notification_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(runtime->notification_label), 20);
    gtk_box_append(GTK_BOX(bar), runtime->notification_label);
    runtime->workflow_badge = gtk_label_new("Workflow");
    gtk_label_set_ellipsize(GTK_LABEL(runtime->workflow_badge), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(runtime->workflow_badge), 20);
    gtk_widget_add_css_class(runtime->workflow_badge, "umicom-runtime-muted");
    gtk_box_append(GTK_BOX(bar), runtime->workflow_badge);
    commands = runtime_application_bar_icon_button("system-search-symbolic",
        "Search commands", "Search commands and actions (Ctrl+Shift+P)");
    layouts = runtime_application_bar_icon_button("view-grid-symbolic",
        "Manage layouts", "Switch or manage layouts (Ctrl+Shift+L)");
    new_window = runtime_application_bar_icon_button("window-new-symbolic",
        "Add a window", "Add or restore Framework panels");
    /* Every action still uses the command registry or the existing catalogue. */
    g_signal_connect(commands, "clicked", G_CALLBACK(runtime_on_commands_clicked), runtime);
    g_signal_connect(layouts, "clicked", G_CALLBACK(runtime_on_layouts_clicked), runtime);
    g_signal_connect(new_window, "clicked", G_CALLBACK(runtime_on_new_window_clicked), runtime);
    gtk_box_append(GTK_BOX(bar), commands);
    gtk_box_append(GTK_BOX(bar), new_window);
    gtk_box_append(GTK_BOX(bar), layouts);
    return bar;
}
