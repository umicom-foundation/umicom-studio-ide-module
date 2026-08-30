/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_surface_registry.inc
 *
 * PURPOSE:
 *   Compose the visible GTK tool stacks from the Framework Studio surface
 *   catalogue while retaining the existing GTK4 adapter as the central editor/
 *   workbench renderer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_stack_add(
    GtkWidget *stack,
    GtkWidget *child,
    const char *name)
{
    if (stack == NULL || child == NULL || name == NULL) return;

    gtk_stack_add_named(GTK_STACK(stack), child, name);
}

static GtkWidget *runtime_build_surface_for_kind(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
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
        default:
            return NULL;
    }
}

static UmiStudioRuntimeSurfaceKind runtime_active_surface_kind(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *active_view_id)
{
    UmiStudioRuntimeBindings *bindings;
    int kind;

    if (runtime == NULL || runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL ||
        active_view_id == NULL || active_view_id[0] == '\0') {
        return UMI_STUDIO_SURFACE_EDITOR;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    if (bindings == NULL) return UMI_STUDIO_SURFACE_EDITOR;

    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_AI_VALIDATION;
         ++kind) {
        const UmiStudioRuntimeSurfaceBinding *surface =
            umi_studio_surface_catalogue_for_kind(
                (UmiStudioRuntimeSurfaceKind)kind);
        UmiApplicationShellContribution contribution;

        if (surface == NULL) continue;

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

static void runtime_show_surface(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
    const char *name = umi_studio_runtime_surface_kind_text(kind);

    if (runtime == NULL || name == NULL) return;

    switch (kind) {
        case UMI_STUDIO_SURFACE_EXPLORER:
        case UMI_STUDIO_SURFACE_SEARCH:
        case UMI_STUDIO_SURFACE_SOURCE_CONTROL:
        case UMI_STUDIO_SURFACE_TEST_EXPLORER:
        case UMI_STUDIO_SURFACE_DEBUG:
            gtk_stack_set_visible_child_name(
                GTK_STACK(runtime->primary_stack),
                name);
            gtk_revealer_set_reveal_child(
                GTK_REVEALER(runtime->primary_revealer),
                TRUE);
            break;

        case UMI_STUDIO_SURFACE_OUTLINE:
        case UMI_STUDIO_SURFACE_AI_CHAT:
        case UMI_STUDIO_SURFACE_AI_REVIEW:
        case UMI_STUDIO_SURFACE_AI_APPROVALS:
            gtk_stack_set_visible_child_name(
                GTK_STACK(runtime->secondary_stack),
                name);
            gtk_revealer_set_reveal_child(
                GTK_REVEALER(runtime->secondary_revealer),
                TRUE);
            break;

        case UMI_STUDIO_SURFACE_PROBLEMS:
        case UMI_STUDIO_SURFACE_OUTPUT:
        case UMI_STUDIO_SURFACE_TERMINAL:
        case UMI_STUDIO_SURFACE_BUILD:
        case UMI_STUDIO_SURFACE_AI_TOOLS:
        case UMI_STUDIO_SURFACE_AI_VALIDATION:
            gtk_stack_set_visible_child_name(
                GTK_STACK(runtime->bottom_stack),
                name);
            gtk_revealer_set_reveal_child(
                GTK_REVEALER(runtime->bottom_revealer),
                TRUE);
            break;

        case UMI_STUDIO_SURFACE_EDITOR:
        default:
            break;
    }
}

static UmiStatus runtime_rebuild_surface_panels(
    UmiStudioGtkRuntimeChrome *runtime)
{
    int kind;

    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_AI_VALIDATION;
         ++kind) {
        GtkWidget *old_panel = runtime->surface_panels[kind];
        GtkWidget *panel = runtime_build_surface_for_kind(
            runtime,
            (UmiStudioRuntimeSurfaceKind)kind);
        const char *name =
            umi_studio_runtime_surface_kind_text(
                (UmiStudioRuntimeSurfaceKind)kind);

        if (old_panel != NULL) {
            GtkWidget *parent = gtk_widget_get_parent(old_panel);
            if (GTK_IS_STACK(parent)) {
                gtk_stack_remove(GTK_STACK(parent), old_panel);
            }
        }

        runtime->surface_panels[kind] = panel;

        if (panel == NULL || kind == (int)UMI_STUDIO_SURFACE_EDITOR) {
            continue;
        }

        switch ((UmiStudioRuntimeSurfaceKind)kind) {
            case UMI_STUDIO_SURFACE_EXPLORER:
            case UMI_STUDIO_SURFACE_SEARCH:
            case UMI_STUDIO_SURFACE_SOURCE_CONTROL:
            case UMI_STUDIO_SURFACE_TEST_EXPLORER:
            case UMI_STUDIO_SURFACE_DEBUG:
                runtime_stack_add(runtime->primary_stack, panel, name);
                break;

            case UMI_STUDIO_SURFACE_OUTLINE:
            case UMI_STUDIO_SURFACE_AI_CHAT:
            case UMI_STUDIO_SURFACE_AI_REVIEW:
            case UMI_STUDIO_SURFACE_AI_APPROVALS:
                runtime_stack_add(runtime->secondary_stack, panel, name);
                break;

            case UMI_STUDIO_SURFACE_PROBLEMS:
            case UMI_STUDIO_SURFACE_OUTPUT:
            case UMI_STUDIO_SURFACE_TERMINAL:
            case UMI_STUDIO_SURFACE_BUILD:
            case UMI_STUDIO_SURFACE_AI_TOOLS:
            case UMI_STUDIO_SURFACE_AI_VALIDATION:
                runtime_stack_add(runtime->bottom_stack, panel, name);
                break;

            default:
                break;
        }
    }

    return UMI_STATUS_OK;
}
