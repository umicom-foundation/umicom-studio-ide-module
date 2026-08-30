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

static const char *runtime_region_text(UmiStudioGtkRuntimeRegion region)
{
    switch (region) {
        case UMI_STUDIO_GTK_REGION_PRIMARY: return "Left";
        case UMI_STUDIO_GTK_REGION_SECONDARY: return "Right";
        case UMI_STUDIO_GTK_REGION_BOTTOM: return "Bottom";
        case UMI_STUDIO_GTK_REGION_EDITOR: return "Editor";
        default: return "Not placed";
    }
}

static GtkWidget *runtime_region_stack(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioGtkRuntimeRegion region)
{
    if (runtime == NULL) return NULL;
    switch (region) {
        case UMI_STUDIO_GTK_REGION_PRIMARY: return runtime->primary_stack;
        case UMI_STUDIO_GTK_REGION_SECONDARY: return runtime->secondary_stack;
        case UMI_STUDIO_GTK_REGION_BOTTOM: return runtime->bottom_stack;
        default: return NULL;
    }
}

static GtkWidget *runtime_region_revealer(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioGtkRuntimeRegion region)
{
    if (runtime == NULL) return NULL;
    switch (region) {
        case UMI_STUDIO_GTK_REGION_PRIMARY: return runtime->primary_revealer;
        case UMI_STUDIO_GTK_REGION_SECONDARY: return runtime->secondary_revealer;
        case UMI_STUDIO_GTK_REGION_BOTTOM: return runtime->bottom_revealer;
        default: return NULL;
    }
}

static void runtime_region_visibility_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioGtkRuntimeRegion region)
{
    GtkWidget *revealer = runtime_region_revealer(runtime, region);
    GtkWidget *stack = runtime_region_stack(runtime, region);
    GtkWidget *first_visible = NULL;
    int kind;
    int has_visible_surface = 0;
    if (runtime == NULL || revealer == NULL) return;
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_AI_VALIDATION;
         ++kind) {
        if (runtime->surface_state_initialised[kind] &&
            runtime->surface_regions[kind] == (int)region &&
            runtime->surface_visible[kind]) {
            has_visible_surface = 1;
            if (first_visible == NULL) {
                first_visible = runtime->surface_panels[kind];
            }
        }
    }
    if (has_visible_surface && stack != NULL) {
        GtkWidget *current = gtk_stack_get_visible_child(GTK_STACK(stack));
        if (current == NULL || !gtk_widget_get_visible(current)) {
            gtk_stack_set_visible_child(GTK_STACK(stack), first_visible);
        }
    }
    gtk_revealer_set_reveal_child(
        GTK_REVEALER(revealer), has_visible_surface != 0);
}

static void runtime_surface_location_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
    GtkWidget *label;
    char text[96];
    if (runtime == NULL || kind < UMI_STUDIO_SURFACE_EXPLORER ||
        kind > UMI_STUDIO_SURFACE_AI_VALIDATION) {
        return;
    }
    label = runtime->surface_location_labels[kind];
    if (label == NULL) return;
    (void)snprintf(
        text,
        sizeof(text),
        "%s · %s",
        runtime_region_text(
            (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind]),
        runtime->surface_visible[kind] ? "open" : "hidden");
    gtk_label_set_text(GTK_LABEL(label), text);
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
    UmiStudioGtkRuntimeRegion region;
    GtkWidget *stack;
    GtkWidget *revealer;

    if (runtime == NULL || name == NULL) return;
    if (kind == UMI_STUDIO_SURFACE_EDITOR) return;
    region = (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind];
    stack = runtime_region_stack(runtime, region);
    revealer = runtime_region_revealer(runtime, region);
    if (stack == NULL || revealer == NULL) return;
    runtime->surface_visible[kind] = 1;
    if (runtime->surface_panels[kind] != NULL) {
        gtk_widget_set_visible(runtime->surface_panels[kind], TRUE);
    }
    gtk_stack_set_visible_child_name(GTK_STACK(stack), name);
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
    runtime_surface_location_refresh(runtime, kind);
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
        const UmiStudioGtkSurfaceProfile *profile =
            runtime_surface_profile((UmiStudioRuntimeSurfaceKind)kind);

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

        if (!runtime->surface_state_initialised[kind] && profile != NULL) {
            runtime->surface_regions[kind] = (int)profile->region;
            runtime->surface_visible[kind] = profile->default_visible;
            runtime->surface_state_initialised[kind] = 1;
        }
        runtime_stack_add(
            runtime_region_stack(
                runtime,
                (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind]),
            panel,
            name);
        gtk_widget_set_visible(panel, runtime->surface_visible[kind] != 0);
        runtime_surface_location_refresh(
            runtime, (UmiStudioRuntimeSurfaceKind)kind);
    }

    return UMI_STATUS_OK;
}
