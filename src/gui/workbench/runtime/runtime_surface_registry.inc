/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_surface_registry.inc
 *
 * PURPOSE:
 *   Build real GTK surface bodies from the Framework Studio catalogue and
 *   project model placement into readable labels. The shared workspace host
 *   alone arranges them; the established adapter remains the real editor.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Catalogue labels are projections of the authoritative Framework layout. */
static void runtime_surface_location_refresh(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind)
{
    const UmiUiWorkspaceLayout *layout = runtime_workspace_active(runtime);
    const char *id = umi_studio_runtime_workspace_surface_id(kind);
    const UmiUiWorkspaceWindow *window = NULL;
    char text[160];
    size_t index;
    if (runtime == NULL || id == NULL || runtime->surface_location_labels[kind] == NULL)
        return;
    if (layout != NULL)
        for (index = 0U; index < layout->window_count; ++index)
            if (strcmp(layout->windows[index].window_id, id) == 0) {
                window = &layout->windows[index];
                break;
            }
    if (window == NULL) {
        gtk_label_set_text(GTK_LABEL(runtime->surface_location_labels[kind]), "Not on this layout");
        return;
    }
    (void)snprintf(text, sizeof(text), "%s · %s%s",
        window->floating ? "Native window" :
            (strcmp(window->placement_id, "canvas") == 0 ? "Canvas" : window->placement_id),
        window->visible ? "open" : "hidden", window->pinned ? " · pinned" : "");
    gtk_label_set_text(GTK_LABEL(runtime->surface_location_labels[kind]), text);
}

/* Named command selection uses the same model as the native layout selector. */
static UmiStatus runtime_workspace_select_layout(UmiStudioGtkRuntimeChrome *runtime, const char *layout_id)
{
    UmiStatus status;
    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_runtime_workspace_select(
        runtime_workspace_model(runtime), layout_id);
    return status == UMI_STATUS_OK ? runtime_workspace_rebuild(runtime) : status;
}

/* Select each established operational provider without a summary replacement. */
static GtkWidget *runtime_build_surface_for_kind(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (kind) {
        case UMI_STUDIO_SURFACE_EXPLORER:
            return runtime_build_surface_explorer(runtime);
        case UMI_STUDIO_SURFACE_EDITOR:
            return runtime_build_surface_editor(runtime);
        case UMI_STUDIO_SURFACE_OUTLINE:
            return runtime_build_surface_outline(runtime);
        case UMI_STUDIO_SURFACE_SEARCH:
            return runtime_build_surface_search(runtime);
        case UMI_STUDIO_SURFACE_PROBLEMS:
            return runtime_build_surface_problems(runtime);
        case UMI_STUDIO_SURFACE_OUTPUT:
            return runtime_build_surface_output(runtime);
        case UMI_STUDIO_SURFACE_TERMINAL:
            return runtime_build_surface_terminal(runtime);
        case UMI_STUDIO_SURFACE_SOURCE_CONTROL:
            return runtime_build_surface_source_control(runtime);
        case UMI_STUDIO_SURFACE_TEST_EXPLORER:
            return runtime_build_surface_test_explorer(runtime);
        case UMI_STUDIO_SURFACE_DEBUG:
            return runtime_build_surface_debug(runtime);
        case UMI_STUDIO_SURFACE_BUILD:
            return runtime_build_surface_build(runtime);
        case UMI_STUDIO_SURFACE_AI_CHAT:
            return runtime_build_surface_ai_chat(runtime);
        case UMI_STUDIO_SURFACE_AI_REVIEW:
            return runtime_build_surface_ai_review(runtime);
        case UMI_STUDIO_SURFACE_AI_APPROVALS:
            return runtime_build_surface_ai_approvals(runtime);
        case UMI_STUDIO_SURFACE_AI_TOOLS:
            return runtime_build_surface_ai_tools(runtime);
        case UMI_STUDIO_SURFACE_AI_VALIDATION:
            return runtime_build_surface_ai_validation(runtime);
        case UMI_STUDIO_SURFACE_AI_MODEL_COMPARISON:
            return runtime_build_surface_ai_model_comparison(runtime);
        case UMI_STUDIO_SURFACE_LEARNING:
            return runtime_build_surface_learning(runtime);
        case UMI_STUDIO_SURFACE_DOCUMENTATION:
            return runtime_build_surface_documentation(runtime);
        case UMI_STUDIO_SURFACE_VISUAL_DESIGNER:
            return runtime_build_surface_visual_designer(runtime);
        case UMI_STUDIO_SURFACE_COMPONENT_PALETTE:
            return runtime_build_surface_component_palette(runtime);
        case UMI_STUDIO_SURFACE_COMPONENT_STRUCTURE:
            return runtime_build_surface_component_structure(runtime);
        case UMI_STUDIO_SURFACE_OBJECT_INSPECTOR:
            return runtime_build_surface_object_inspector(runtime);
        case UMI_STUDIO_SURFACE_DESIGN_CODE:
            return runtime_build_surface_design_code(runtime);
        case UMI_STUDIO_SURFACE_DESIGN_MIXED:
            return runtime_build_surface_design_mixed(runtime);
        case UMI_STUDIO_SURFACE_DESIGN_PREVIEW:
            return runtime_build_surface_design_preview(runtime);
        case UMI_STUDIO_SURFACE_LIVE_PREVIEW:
            return runtime_build_surface_live_preview(runtime);
        default:
            return NULL;
    }
}

/*
 * Provide the runtime active surface kind operation used by this module and its client
 * applications.
 */
static UmiStudioRuntimeSurfaceKind runtime_active_surface_kind(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *active_view_id)
{
    UmiStudioRuntimeBindings *bindings;
    int kind;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL ||
        active_view_id == NULL || active_view_id[0] == '\0') {
        return UMI_STUDIO_SURFACE_EDITOR;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL) return UMI_STUDIO_SURFACE_EDITOR;

    /* Visit each bounded item once so every record receives the same rule. */
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        const UmiStudioRuntimeSurfaceBinding *surface =
            umi_studio_surface_catalogue_for_kind(
                (UmiStudioRuntimeSurfaceKind)kind);
        UmiApplicationShellContribution contribution;

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (surface == NULL) continue;

        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_studio_runtime_surface_resolve(
                bindings->shell_registry,
                surface,
                &contribution) == UMI_STATUS_OK &&
            strcmp(contribution.contribution_id, active_view_id) == 0) {
            return (UmiStudioRuntimeSurfaceKind)kind;
        }
    }

    return UMI_STUDIO_SURFACE_EDITOR;
}

/*
 * Provide the runtime show surface operation used by this module and its client
 * applications.
 */
/* Navigation opens the canonical singleton, then focuses its retained body. */
static void runtime_show_surface(
    UmiStudioGtkRuntimeChrome *runtime, UmiStudioRuntimeSurfaceKind kind)
{
    UmiStatus status = runtime_workspace_open_surface(runtime, kind, NULL);
    if (runtime != NULL &&
        kind >= UMI_STUDIO_SURFACE_VISUAL_DESIGNER &&
        kind <= UMI_STUDIO_SURFACE_DESIGN_PREVIEW)
        runtime->designer_active_surface = kind;
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "The requested surface could not be opened.");
}

/* Explicit chrome rebuilds reuse the host's retained real surface bodies. */
static UmiStatus runtime_rebuild_surface_panels(UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime_workspace_rebuild(runtime);
}
