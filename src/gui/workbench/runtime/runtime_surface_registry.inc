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

/* The catalogue implementation is included later in the same translation
 * unit. These declarations let panel-header actions call its shared commands. */
static UmiStatus runtime_surface_place(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind,
    UmiStudioGtkRuntimeRegion region);
static UmiStatus runtime_surface_float(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind);
static UmiStatus runtime_surface_hide(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind);

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
        case UMI_STUDIO_GTK_REGION_EDITOR: return runtime->centre_stack;
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
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        if (runtime->surface_state_initialised[kind] &&
            runtime->surface_regions[kind] == (int)region &&
            runtime->surface_visible[kind] &&
            !runtime->surface_floating[kind]) {
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
        kind > UMI_STUDIO_SURFACE_LAST) {
        return;
    }
    label = runtime->surface_location_labels[kind];
    if (label == NULL) return;
    (void)snprintf(
        text,
        sizeof(text),
        "%s · %s",
        runtime->surface_floating[kind]
            ? "Floating"
            : runtime_region_text(
                  (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind]),
        runtime->surface_visible[kind] ? "open" : "hidden");
    gtk_label_set_text(GTK_LABEL(label), text);
}

static UmiStudioRuntimeSurfaceKind runtime_surface_kind_from_id(
    const char *surface_id)
{
    int kind;
    if (surface_id == NULL) return UMI_STUDIO_SURFACE_EDITOR;
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        const char *candidate = umi_studio_runtime_surface_kind_text(
            (UmiStudioRuntimeSurfaceKind)kind);
        if (candidate != NULL && strcmp(candidate, surface_id) == 0)
            return (UmiStudioRuntimeSurfaceKind)kind;
    }
    return UMI_STUDIO_SURFACE_EDITOR;
}

static void runtime_on_panel_chrome_action(
    UmiWsPanelAction action,
    const UmiWsPanelChrome *chrome,
    void *user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioRuntimeSurfaceKind kind;
    UmiStatus status = UMI_STATUS_OK;
    if (runtime == NULL || chrome == NULL) return;
    kind = runtime_surface_kind_from_id(chrome->panel_id);
    if (kind == UMI_STUDIO_SURFACE_EDITOR ||
        runtime->surface_panels[kind] == NULL)
        return;
    switch (action) {
        case UMI_WS_PANEL_ACTION_CLOSE:
            status = runtime_surface_hide(runtime, kind);
            break;
        case UMI_WS_PANEL_ACTION_FLOAT_TOGGLE:
            status = runtime->surface_floating[kind]
                ? runtime_surface_place(
                      runtime,
                      kind,
                      (UmiStudioGtkRuntimeRegion)
                          runtime->surface_regions[kind])
                : runtime_surface_float(runtime, kind);
            break;
        case UMI_WS_PANEL_ACTION_MOVE:
        case UMI_WS_PANEL_ACTION_SETTINGS:
            runtime_show_window_catalogue(runtime);
            return;
        case UMI_WS_PANEL_ACTION_CONTEXT_GROUP:
            /* The context strip above the workbench is the single visible
             * editor for source, destination and bidirectional link roles. */
            runtime_show_notification(
                runtime,
                "Use the linked-context strip above the workbench to group panels.");
            return;
        default:
            return;
    }
    if (status == UMI_STATUS_PERMISSION_DENIED) {
        runtime_show_notification(
            runtime, "Unlock the layout before changing this panel.");
    }
}

/* Return the default linked-context group displayed by a Studio panel. The
 * Context Host remains authoritative and can replace this assignment when the
 * user chooses another group. */
static const char *runtime_surface_default_context_group(
    UmiStudioRuntimeSurfaceKind kind)
{
    switch (kind) {
        case UMI_STUDIO_SURFACE_TEST_EXPLORER:
            return "studio.context.testing";
        case UMI_STUDIO_SURFACE_AI_CHAT:
        case UMI_STUDIO_SURFACE_AI_REVIEW:
        case UMI_STUDIO_SURFACE_AI_APPROVALS:
        case UMI_STUDIO_SURFACE_AI_TOOLS:
        case UMI_STUDIO_SURFACE_AI_VALIDATION:
        case UMI_STUDIO_SURFACE_AI_MODEL_COMPARISON:
            return "studio.context.ai";
        default:
            return "studio.context.development";
    }
}

/* Context groups receive a restrained header stripe. The colour is a visual
 * aid only and never changes how selections or commands are routed. */
static const char *runtime_surface_default_context_colour(
    UmiStudioRuntimeSurfaceKind kind)
{
    switch (kind) {
        case UMI_STUDIO_SURFACE_TEST_EXPLORER:
            return "umicom-context-green";
        case UMI_STUDIO_SURFACE_AI_CHAT:
        case UMI_STUDIO_SURFACE_AI_REVIEW:
        case UMI_STUDIO_SURFACE_AI_APPROVALS:
        case UMI_STUDIO_SURFACE_AI_TOOLS:
        case UMI_STUDIO_SURFACE_AI_VALIDATION:
        case UMI_STUDIO_SURFACE_AI_MODEL_COMPARISON:
            return "umicom-context-purple";
        default:
            return "umicom-context-blue";
    }
}

static GtkWidget *runtime_wrap_surface_panel(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind,
    const UmiStudioGtkSurfaceProfile *profile,
    GtkWidget *content)
{
    UmiWsPanelChrome chrome;
    const char *surface_id = umi_studio_runtime_surface_kind_text(kind);
    const char *placement;
    if (runtime == NULL || profile == NULL || content == NULL ||
        surface_id == NULL)
        return content;
    placement = runtime_region_text(
        (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind]);
    if (umi_ws_panel_chrome_init(&chrome, profile->title) != UMI_STATUS_OK)
        return content;
    (void)umi_ws_panel_chrome_set_identity(&chrome, surface_id, placement);
    (void)umi_ws_panel_chrome_set_context(
        &chrome,
        runtime_surface_default_context_group(kind),
        runtime_surface_default_context_colour(kind));
    chrome.show_pin = false;
    chrome.show_menu = false;
    chrome.show_context = true;
    chrome.show_move = true;
    chrome.show_float = true;
    chrome.show_maximise = false;
    chrome.show_settings = true;
    chrome.show_close = true;
    chrome.locked = runtime->layout_locked != 0;
    chrome.floating = runtime->surface_floating[kind] != 0;
    umi_ws_panel_chrome_set_compact(&chrome, true);
    return umi_gtk4_ws_panel_frame_create_interactive(
        &chrome, content, runtime_on_panel_chrome_action, runtime);
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
        case UMI_STUDIO_SURFACE_AI_MODEL_COMPARISON:
            return runtime_build_surface_ai_model_comparison(runtime);
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
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
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
    if (runtime->surface_floating[kind] &&
        runtime->surface_windows[kind] != NULL) {
        runtime->surface_visible[kind] = 1;
        gtk_widget_set_visible(runtime->surface_windows[kind], TRUE);
        gtk_window_present(GTK_WINDOW(runtime->surface_windows[kind]));
        runtime_surface_location_refresh(runtime, kind);
        return;
    }
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
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
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
            } else if (GTK_IS_WINDOW(parent)) {
                /* A refresh replaces the panel body without destroying its
                 * detached native window or losing monitor placement. */
                gtk_window_set_child(GTK_WINDOW(parent), NULL);
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
        panel = runtime_wrap_surface_panel(
            runtime, (UmiStudioRuntimeSurfaceKind)kind, profile, panel);
        runtime->surface_panels[kind] = panel;
        if (runtime->surface_floating[kind] &&
            runtime->surface_windows[kind] != NULL) {
            gtk_window_set_child(
                GTK_WINDOW(runtime->surface_windows[kind]), panel);
        } else {
            runtime_stack_add(
                runtime_region_stack(
                    runtime,
                    (UmiStudioGtkRuntimeRegion)runtime->surface_regions[kind]),
                panel,
                name);
        }
        gtk_widget_set_visible(panel, runtime->surface_visible[kind] != 0);
        runtime_surface_location_refresh(
            runtime, (UmiStudioRuntimeSurfaceKind)kind);
    }

    return UMI_STATUS_OK;
}
