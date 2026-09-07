/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_workspace_host.inc
 *
 * PURPOSE:
 *   Bind Studio's real tools and retained editor to one Framework workspace
 *   host. The existing professional workspace owns every panel transaction.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
static void runtime_workspace_update_selector(UmiStudioGtkRuntimeChrome *runtime);

/* Surface builders and readable catalogue labels follow in the unity unit. */
static GtkWidget *runtime_build_surface_for_kind(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind);
static void runtime_surface_location_refresh(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind);
static void runtime_workspace_update_lock_button(UmiStudioGtkRuntimeChrome *runtime);

/* Borrow the sole outer-panel model from Studio's existing service facade. */
static UmiUiWorkspaceCustomisation *runtime_workspace_model(
    UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL
        ? umi_studio_professional_workspace_model(runtime->professional_workspace)
        : NULL;
}

/* Expose a model snapshot to presentation without inventing another layout. */
static const UmiUiWorkspaceLayout *runtime_workspace_active(
    UmiStudioGtkRuntimeChrome *runtime)
{
    return umi_ui_workspace_customisation_active_const(runtime_workspace_model(runtime));
}

/* Translate existing native surface metadata into a default dock destination. */
static const char *runtime_workspace_default_placement(UmiStudioRuntimeSurfaceKind kind)
{
    const UmiStudioRuntimeSurfaceBinding *binding = umi_studio_surface_catalogue_for_kind(kind);
    if (binding == NULL) return "centre";
    switch (binding->region) {
    case UMI_APPLICATION_SHELL_REGION_PRIMARY_SIDEBAR: return "left";
    case UMI_APPLICATION_SHELL_REGION_SECONDARY_SIDEBAR: return "right";
    case UMI_APPLICATION_SHELL_REGION_BOTTOM_PANEL: return "bottom";
    default: return "centre";
    }
}

/* Keep compatibility arrays as read-only projections for catalogue labels.
 * No GTK command writes these arrays to change placement or roll back edits. */
static void runtime_workspace_update_cache(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    const UmiUiWorkspaceLayout *layout = runtime_workspace_active(runtime);
    int kind;

    if (model == NULL || layout == NULL) return;
    runtime->layout_locked = layout->locked;
    runtime->layout_baseline_valid = model->edit_active;
    for (kind = UMI_STUDIO_SURFACE_EXPLORER; kind <= UMI_STUDIO_SURFACE_LAST; ++kind) {
        const char *id = umi_studio_runtime_workspace_surface_id((UmiStudioRuntimeSurfaceKind)kind);
        const UmiUiWorkspaceWindow *window = umi_ui_workspace_layout_find_window(layout, id);
        int region = UMI_STUDIO_GTK_REGION_EDITOR;

        if (window != NULL) {
            const char *placement = window->placement_id;
            /* A collapsed tool still belongs to its saved dock edge. This is
             * a read-only label projection, never an implicit rail reveal. */
            if (strncmp(placement, "auto-hide:", sizeof("auto-hide:") - 1U) == 0)
                placement += sizeof("auto-hide:") - 1U;
            if (strcmp(placement, "left") == 0) region = UMI_STUDIO_GTK_REGION_PRIMARY;
            else if (strcmp(placement, "right") == 0) region = UMI_STUDIO_GTK_REGION_SECONDARY;
            else if (strcmp(placement, "bottom") == 0) region = UMI_STUDIO_GTK_REGION_BOTTOM;
        }
        runtime->surface_regions[kind] = region;
        runtime->surface_visible[kind] = window != NULL && window->visible;
        runtime->surface_floating[kind] = window != NULL && window->floating;
        runtime->surface_state_initialised[kind] = window != NULL;
        runtime_surface_location_refresh(runtime, (UmiStudioRuntimeSurfaceKind)kind);
    }
    runtime_workspace_update_lock_button(runtime);
    runtime_workspace_update_selector(runtime);
    runtime_workspace_update_storage_controls(runtime);
}

/* Render one accepted model revision; persistent bodies keep native drafts. */
static UmiStatus runtime_workspace_rebuild(UmiStudioGtkRuntimeChrome *runtime)
{
    const UmiUiWorkspaceLayout *layout = runtime_workspace_active(runtime);
    UmiStatus status;

    if (runtime == NULL || runtime->workspace_host == NULL || layout == NULL)
        return UMI_STATUS_INVALID_STATE;
    status = umi_gtk4_workspace_layout_host_rebuild(runtime->workspace_host, layout);
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_workspace_project_shell(
            runtime_workspace_model(runtime), runtime->shell_registry,
            &runtime->shell_state, &runtime->shell_layout);
        runtime_workspace_update_cache(runtime);
    }
    return status;
}

/* Build native presentation from a validated heap candidate before publishing
 * its model. A failed rebuild keeps the old authority and attempts to restore
 * its widgets; policy failures never reach this presentation boundary. */
static UmiStatus runtime_workspace_publish_candidate(
    UmiStudioGtkRuntimeChrome *runtime, const UmiUiWorkspaceCustomisation *candidate)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    const UmiUiWorkspaceLayout *previous = runtime_workspace_active(runtime);
    const UmiUiWorkspaceLayout *next = candidate != NULL
        ? umi_ui_workspace_customisation_active_const(candidate) : NULL;
    UmiStatus status;
    if (model == NULL || previous == NULL || next == NULL || runtime->workspace_host == NULL)
        return UMI_STATUS_INVALID_STATE;
    if (candidate->revision == model->revision) return UMI_STATUS_OK;
    status = umi_gtk4_workspace_layout_host_rebuild(runtime->workspace_host, next);
    /* The Framework projection validates all bindings before updating its
     * compatibility outputs. Complete it before publishing native authority. */
    if (status == UMI_STATUS_OK)
        status = umi_studio_runtime_workspace_project_shell(candidate, runtime->shell_registry,
            &runtime->shell_state, &runtime->shell_layout);
    if (status != UMI_STATUS_OK) {
        (void)umi_gtk4_workspace_layout_host_rebuild(runtime->workspace_host, previous);
        return status;
    }
    *model = *candidate;
    (void)umi_gtk4_workspace_layout_host_set_context_groups(runtime->workspace_host, &model->groups);
    runtime_workspace_update_cache(runtime);
    return status;
}

/* Factory output is the real operational body, never the old chrome wrapper.
 * In particular the Editor is the original Framework adapter, not its summary
 * card from runtime_build_surface_editor. The host retains these bodies. */
static GtkWidget *runtime_workspace_panel_factory(
    const UmiUiWorkspaceWindow *window, void *user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiStudioRuntimeSurfaceKind kind;
    GtkWidget *body;

    if (runtime == NULL || window == NULL ||
        umi_studio_runtime_workspace_surface_kind(window->tool_id, &kind) != UMI_STATUS_OK)
        return NULL;
    body = kind == UMI_STUDIO_SURFACE_EDITOR
        ? runtime->centre_stack : runtime_build_surface_for_kind(runtime, kind);
    runtime->surface_panels[kind] = body;
    return body;
}

/* Apply one finished gesture to the current edit, including a stale-revision
 * guard. GTK has already restored its preview and may safely rebuild now. */
static UmiStatus runtime_workspace_on_geometry(
    const char *window_id, const UmiApplicationSuiteLayoutRect *rect,
    uint64_t expected_revision, void *user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiStatus status = umi_studio_runtime_workspace_place_canvas(
        runtime_workspace_model(runtime), window_id, rect, expected_revision);

    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "Panel arrangement rejected; check layout revision and locks.");
    return status;
}

/* Focus only a visible model panel. Periodic shell synchronization uses this
 * non-mutating path so it cannot reopen panels hidden in a custom canvas. */
static void runtime_workspace_focus_surface(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind)
{
    const char *id = umi_studio_runtime_workspace_surface_id(kind);
    const UmiUiWorkspaceLayout *layout = runtime_workspace_active(runtime);
    const UmiUiWorkspaceWindow *window = layout != NULL && id != NULL
        ? umi_ui_workspace_layout_find_window(layout, id) : NULL;
    if (runtime == NULL || runtime->workspace_host == NULL || id == NULL) return;
    /* Periodic command/status focus is not explicit Open. Even an adapter
     * capable of revealing rails must never receive that implicit request. */
    if (window == NULL || !window->visible ||
        umi_ui_workspace_customisation_window_is_auto_hidden(runtime_workspace_model(runtime), id)) return;
    if (umi_gtk4_workspace_layout_host_focus_window(runtime->workspace_host, id) == UMI_STATUS_OK) {
        const UmiStudioRuntimeSurfaceBinding *binding = umi_studio_surface_catalogue_for_kind(kind);
        UmiApplicationShellContribution contribution;
        /* Focus is compatibility selection, never a placement/visibility import. */
        if (binding != NULL && umi_studio_runtime_surface_resolve(
                runtime->shell_registry, binding, &contribution) == UMI_STATUS_OK) {
            (void)umi_application_shell_state_activate(&runtime->shell_state,
                binding->activity_id, contribution.contribution_id);
            (void)umi_application_shell_state_focus(&runtime->shell_state,
                contribution.contribution_id);
        }
        runtime->last_presented_surface = kind;
    }
}

/* Open a native catalogue singleton through the model, optionally placing it
 * in an explicit dock/canvas destination during the existing edit session. */
static UmiStatus runtime_workspace_open_surface(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind,
    const char *placement_id)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    const UmiUiWorkspaceLayout *layout;
    const UmiUiWorkspaceWindow *window;
    const char *id = umi_studio_runtime_workspace_surface_id(kind);
    UmiUiWorkspacePanelSettings settings;
    UmiUiWorkspaceCustomisation *candidate;
    UmiStatus status;

    if (model == NULL || id == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (placement_id != NULL && !model->edit_active) return UMI_STATUS_PERMISSION_DENIED;
    layout = runtime_workspace_active(runtime);
    window = umi_ui_workspace_layout_find_window(layout, id);
    if (window != NULL && strcmp(window->tool_id, id) != 0) return UMI_STATUS_INVALID_STATE;
    /* Explicit Open on an existing rail is a transient reveal. It must not
     * set visible=true on an auto-hide record, change a revision or rebuild
     * unrelated native tools. Periodic focus never uses this explicit path. */
    if (placement_id == NULL && window != NULL &&
        umi_ui_workspace_customisation_window_is_auto_hidden(model, id))
        return umi_gtk4_workspace_layout_host_reveal_tool_window(runtime->workspace_host, id);
    /* Ordinary focus must not rebuild provider widgets or dirty an unchanged
     * layout. The shared host also restores an internally maximised sibling. */
    if (placement_id == NULL && window != NULL && window->visible) {
        status = umi_gtk4_workspace_layout_host_focus_window(runtime->workspace_host, id);
        if (status == UMI_STATUS_OK) runtime_workspace_focus_surface(runtime, kind);
        return status;
    }
    candidate = g_try_new(UmiUiWorkspaceCustomisation, 1);
    if (candidate == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    *candidate = *model;
    status = placement_id == NULL
        ? umi_studio_runtime_workspace_set_tool_presentation(candidate, kind, true, false)
        : umi_studio_runtime_workspace_set_visible(candidate, kind, true);
    if (status != UMI_STATUS_OK) { g_free(candidate); return status; }
    layout = umi_ui_workspace_customisation_active_const(candidate);
    window = umi_ui_workspace_layout_find_window(layout, id);
    if (window == NULL) { g_free(candidate); return UMI_STATUS_NOT_FOUND; }
    if (placement_id != NULL) {
        settings = umi_ui_workspace_panel_settings_default(id);
        settings.floating = strcmp(placement_id, "floating") == 0;
        settings.placement_id = settings.floating
            ? runtime_workspace_default_placement(kind) : placement_id;
        settings.stack_id = strcmp(placement_id, UMI_UI_WORKSPACE_CANVAS_PLACEMENT) == 0
            ? id : settings.placement_id;
        settings.context_group_id = window->context_group_id;
        (void)umi_ui_window_group_for_window(&candidate->groups, id, &settings.context_role);
        settings.x = window->x;
        settings.y = window->y;
        settings.width = window->width;
        settings.height = window->height;
        if (strcmp(placement_id, UMI_UI_WORKSPACE_CANVAS_PLACEMENT) == 0) {
            settings.x = 0.10;
            settings.y = 0.10;
            settings.width = 0.60;
            settings.height = 0.65;
        }
        status = umi_studio_runtime_workspace_apply_panel_settings(candidate, &settings);
        if (status != UMI_STATUS_OK) { g_free(candidate); return status; }
    }
    /* Publish only after visibility, placement and native host validation all
     * succeed. A rejected placement never partially opens a hidden panel. */
    status = runtime_workspace_publish_candidate(runtime, candidate);
    if (status == UMI_STATUS_OK) runtime_workspace_focus_surface(runtime, kind);
    g_free(candidate);
    return status;
}

/* Make real panel chrome use the same model commands as the New Window UI. */
static void runtime_workspace_on_panel_action(
    const char *window_id, UmiWsPanelAction action, void *user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    const UmiUiWorkspaceLayout *layout = runtime_workspace_active(runtime);
    const UmiUiWorkspaceWindow *window;
    UmiStudioRuntimeSurfaceKind kind;
    UmiUiWorkspacePanelSettings settings;
    UmiUiWorkspaceCustomisation *candidate;
    UmiStatus status = UMI_STATUS_OK;

    if (model == NULL || layout == NULL ||
        umi_studio_runtime_workspace_surface_kind(window_id, &kind) != UMI_STATUS_OK) return;
    window = umi_ui_workspace_layout_find_window(layout, window_id);
    if (window == NULL) return;
    if (action == UMI_WS_PANEL_ACTION_MOVE || action == UMI_WS_PANEL_ACTION_SETTINGS) {
        runtime_show_window_catalogue(runtime);
        return;
    }
    if (action == UMI_WS_PANEL_ACTION_CONTEXT_GROUP) {
        runtime_show_notification(runtime, "Use the linked-context strip in the editor to group document contexts.");
        return;
    }
    /* The flyout's X normally stays inside the Framework host. Keep this
     * defensive route transient too if an adapter forwards the action. */
    if (action == UMI_WS_PANEL_ACTION_CLOSE &&
        umi_ui_workspace_customisation_window_is_auto_hidden(model, window_id)) {
        umi_gtk4_workspace_layout_host_collapse_tool_windows(runtime->workspace_host);
        return;
    }
    if (action == UMI_WS_PANEL_ACTION_MAXIMISE_TOGGLE && !window->floating) {
        runtime_show_notification(runtime, "Maximise is currently available for detached native windows only.");
        return;
    }
    candidate = g_try_new(UmiUiWorkspaceCustomisation, 1);
    if (candidate == NULL) {
        runtime_show_notification(runtime, "The panel change could not allocate workspace state.");
        return;
    }
    *candidate = *model;
    if (action == UMI_WS_PANEL_ACTION_CLOSE) {
        status = !window->floating && strcmp(window->placement_id, UMI_UI_WORKSPACE_CANVAS_PLACEMENT) != 0
            ? umi_studio_runtime_workspace_set_tool_presentation(candidate, kind, false, false)
            : umi_studio_runtime_workspace_set_visible(candidate, kind, false);
    } else if (action == UMI_WS_PANEL_ACTION_AUTO_HIDE_TOGGLE) {
        status = umi_studio_runtime_workspace_set_tool_presentation(candidate, kind, true,
            !umi_ui_workspace_customisation_window_is_auto_hidden(model, window_id));
    } else if (action == UMI_WS_PANEL_ACTION_PIN_TOGGLE) {
        status = umi_studio_runtime_workspace_set_pinned(candidate, window_id, !window->pinned);
    } else if (action == UMI_WS_PANEL_ACTION_MAXIMISE_TOGGLE) {
        status = umi_studio_runtime_workspace_set_maximised(candidate, window_id, !window->maximised);
    } else if (action == UMI_WS_PANEL_ACTION_FLOAT_TOGGLE) {
        const char *id = window_id;
        settings = umi_ui_workspace_panel_settings_default(window_id);
        settings.placement_id = runtime_workspace_default_placement(kind);
        settings.stack_id = settings.placement_id;
        settings.context_group_id = window->context_group_id;
        (void)umi_ui_window_group_for_window(&model->groups, id, &settings.context_role);
        settings.floating = !window->floating;
        settings.x = 0.10;
        settings.y = 0.10;
        settings.width = 0.60;
        settings.height = 0.65;
        status = umi_studio_runtime_workspace_apply_panel_settings(candidate, &settings);
    } else {
        g_free(candidate);
        return;
    }
    if (status == UMI_STATUS_OK) status = runtime_workspace_publish_candidate(runtime, candidate);
    g_free(candidate);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "Panel change was not applied. Check whether the panel is protected or the action needs Edit Layout.");
}

/* Initialise the professional model and one empty shared host before enabling
 * opt-in body retention. The following rebuild adopts Studio's real surfaces. */
static UmiStatus runtime_workspace_create(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceLayout *empty;
    UmiStatus status;

    status = umi_studio_professional_workspace_create(
        umi_studio_ui_services(runtime->owner->ui), &runtime->professional_workspace);
    if (status == UMI_STATUS_OK)
        status = umi_studio_professional_workspace_seed_native(runtime->professional_workspace);
    if (status != UMI_STATUS_OK) return status;
    empty = g_try_new0(UmiUiWorkspaceLayout, 1);
    if (empty == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_ui_workspace_layout_init(empty, "umicom.studio.layout.host-empty", "Studio workspace");
    if (status == UMI_STATUS_OK)
        status = umi_gtk4_workspace_layout_host_create_with_options(empty,
            runtime_workspace_panel_factory, runtime,
            runtime_workspace_on_panel_action, runtime,
            &(const UmiGtk4WorkspaceLayoutHostOptions){ .tool_presentation_actions = 1 },
            &runtime->workspace_host);
    g_free(empty);
    if (status != UMI_STATUS_OK) return status;
    status = umi_gtk4_workspace_layout_host_set_content_retention(runtime->workspace_host, 1);
    if (status == UMI_STATUS_OK)
        status = umi_gtk4_workspace_layout_host_set_context_groups(runtime->workspace_host,
            &runtime_workspace_model(runtime)->groups);
    if (status == UMI_STATUS_OK)
        status = umi_gtk4_workspace_layout_host_set_canvas_geometry_handler(
            runtime->workspace_host, runtime_workspace_on_geometry, runtime);
    umi_gtk4_workspace_layout_host_set_transient_parent(runtime->workspace_host, runtime->owner->window);
    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    return status;
}

/* Use the existing transaction for a named blank canvas and expose it in GTK. */
static UmiStatus runtime_workspace_create_blank(UmiStudioGtkRuntimeChrome *runtime,
    const char *layout_id, const char *name)
{
    UmiStatus status = umi_studio_runtime_workspace_create_blank(
        runtime_workspace_model(runtime), layout_id, name);
    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    return status;
}

/* Begin one reversible panel edit without copying a private GTK baseline. */
static UmiStatus runtime_workspace_begin_edit(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status = umi_ui_workspace_customisation_begin_edit(runtime_workspace_model(runtime));
    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    return status;
}

/* Commit the current model edit. Durable canvas checkpoints are not silently
 * confused with the older semantic shell session format. */
static UmiStatus runtime_workspace_commit_edit(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status = umi_ui_workspace_customisation_commit_edit(runtime_workspace_model(runtime));
    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    if (status == UMI_STATUS_OK)
        runtime_show_notification(runtime, "Arrangement applied. Use Save layout to keep an explicit checkpoint.");
    return status;
}

/* Restore the canonical transaction baseline and then its native presentation. */
static UmiStatus runtime_workspace_cancel_edit(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status = umi_ui_workspace_customisation_cancel_edit(runtime_workspace_model(runtime));
    if (status == UMI_STATUS_OK) status = runtime_workspace_rebuild(runtime);
    return status;
}

/* Explicit designer refresh invalidates only the changing authoring bodies;
 * Explorer, terminal, source-control and other typed drafts remain retained. */
static UmiStatus runtime_workspace_refresh_designer(UmiStudioGtkRuntimeChrome *runtime)
{
    int kind;
    if (runtime == NULL || runtime->workspace_host == NULL) return UMI_STATUS_INVALID_STATE;
    for (kind = UMI_STUDIO_SURFACE_VISUAL_DESIGNER;
         kind <= UMI_STUDIO_SURFACE_DESIGN_PREVIEW; ++kind) {
        (void)umi_gtk4_workspace_layout_host_invalidate_content(runtime->workspace_host,
            umi_studio_runtime_workspace_surface_id((UmiStudioRuntimeSurfaceKind)kind));
        runtime_invalidate_surface_tree(runtime->surface_panels[kind], runtime);
        runtime->surface_panels[kind] = NULL;
    }
    return runtime_workspace_rebuild(runtime);
}
