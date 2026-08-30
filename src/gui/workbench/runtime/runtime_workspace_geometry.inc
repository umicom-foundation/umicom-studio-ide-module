/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_workspace_geometry.inc
 *
 * PURPOSE:
 *   Bind Studio's native GTK splitters to the toolkit-neutral Framework
 *   workbench state, including remembered sizes and an explicit edit lock.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static int runtime_clamp_dimension(int value, int minimum, int maximum)
{
    if (maximum < minimum) return minimum;
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static void runtime_workspace_update_lock_button(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->layout_lock_button == NULL) return;
    gtk_button_set_label(
        GTK_BUTTON(runtime->layout_lock_button),
        runtime->layout_locked ? "Layout Locked" : "Editing Layout");
    gtk_widget_set_tooltip_text(
        runtime->layout_lock_button,
        runtime->layout_locked
            ? "Unlock before moving or resizing panels"
            : "Lock this arrangement to prevent accidental changes");
    if (runtime->layout_cancel_button != NULL) {
        gtk_widget_set_visible(
            runtime->layout_cancel_button,
            !runtime->layout_locked && runtime->layout_baseline_valid);
    }
}

static void runtime_workspace_apply_geometry(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkbenchState state;
    int width;
    int height;
    int primary;
    int secondary;
    int bottom;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->primary_paned == NULL ||
        runtime->secondary_paned == NULL ||
        runtime->workspace_paned == NULL) {
        return;
    }

    umi_ui_workbench_state_init(&state);
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    width = gtk_widget_get_width(runtime->secondary_paned);
    height = gtk_widget_get_height(runtime->workspace_paned);
    primary = runtime_clamp_dimension(
        state.sidebar_size,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        480) + UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH;
    secondary = runtime_clamp_dimension(
        state.auxiliary_sidebar_size,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        480);
    bottom = runtime_clamp_dimension(
        state.bottom_panel_size,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT,
        420);

    runtime->applying_workspace_geometry = 1;
    gtk_paned_set_position(GTK_PANED(runtime->primary_paned), primary);
    if (width > secondary + 420) {
        gtk_paned_set_position(
            GTK_PANED(runtime->secondary_paned), width - secondary);
    }
    if (height > bottom + 260) {
        gtk_paned_set_position(
            GTK_PANED(runtime->workspace_paned), height - bottom);
    }
    runtime->applying_workspace_geometry = 0;
}

static void runtime_on_workspace_mapped(
    GtkWidget *widget,
    gpointer user_data)
{
    (void)widget;
    runtime_workspace_apply_geometry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

static void runtime_on_workspace_position_changed(
    GObject *object,
    GParamSpec *property,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiUiWorkbenchState state;
    GtkWidget *paned = GTK_WIDGET(object);
    int available;
    int position;

    (void)property;
    if (runtime == NULL || runtime->applying_workspace_geometry ||
        runtime->owner == NULL) {
        return;
    }
    if (runtime->layout_locked) {
        runtime_workspace_apply_geometry(runtime);
        return;
    }

    umi_ui_workbench_state_init(&state);
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    position = gtk_paned_get_position(GTK_PANED(paned));
    if (paned == runtime->primary_paned) {
        state.sidebar_size = runtime_clamp_dimension(
            position - UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
            480);
    } else if (paned == runtime->secondary_paned) {
        available = gtk_widget_get_width(paned);
        state.auxiliary_sidebar_size = runtime_clamp_dimension(
            available - position,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
            480);
    } else if (paned == runtime->workspace_paned) {
        available = gtk_widget_get_height(paned);
        state.bottom_panel_size = runtime_clamp_dimension(
            available - position,
            UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT,
            420);
    } else {
        return;
    }

    state.active_activity[0] = '\0';
    state.active_view_container[0] = '\0';
    state.active_perspective[0] = '\0';
    state.active_document[0] = '\0';
    state.active_workspace_profile[0] = '\0';
    (void)umi_ui_workbench_state_apply(
        umi_studio_ui_workbench(runtime->owner->ui), &state);
}

static void runtime_on_layout_lock_toggled(
    GtkToggleButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int was_locked;
    int kind;
    if (runtime == NULL) return;
    was_locked = runtime->layout_locked;
    runtime->layout_locked = gtk_toggle_button_get_active(button) ? 1 : 0;
    if (was_locked && !runtime->layout_locked) {
        umi_ui_workbench_state_init(&runtime->layout_edit_baseline);
        if (umi_ui_workbench_state_snapshot(
                umi_studio_ui_workbench(runtime->owner->ui),
                &runtime->layout_edit_baseline) == UMI_STATUS_OK) {
            for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
                 kind <= (int)UMI_STUDIO_SURFACE_AI_VALIDATION;
                 ++kind) {
                runtime->layout_surface_region_baseline[kind] =
                    runtime->surface_regions[kind];
                runtime->layout_surface_visible_baseline[kind] =
                    runtime->surface_visible[kind];
            }
            runtime->layout_baseline_valid = 1;
        }
    } else if (!was_locked && runtime->layout_locked) {
        runtime->layout_baseline_valid = 0;
    }
    runtime_workspace_update_lock_button(runtime);
    if (runtime->layout_locked) runtime_workspace_apply_geometry(runtime);
}

static void runtime_on_layout_cancel_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int kind;
    (void)button;
    if (runtime == NULL || !runtime->layout_baseline_valid) return;

    (void)umi_ui_workbench_state_apply(
        umi_studio_ui_workbench(runtime->owner->ui),
        &runtime->layout_edit_baseline);
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_AI_VALIDATION;
         ++kind) {
        UmiStudioRuntimeSurfaceKind surface =
            (UmiStudioRuntimeSurfaceKind)kind;
        UmiStudioGtkRuntimeRegion region =
            (UmiStudioGtkRuntimeRegion)
                runtime->layout_surface_region_baseline[kind];
        if (surface == UMI_STUDIO_SURFACE_EDITOR ||
            region < UMI_STUDIO_GTK_REGION_PRIMARY ||
            region > UMI_STUDIO_GTK_REGION_BOTTOM) {
            continue;
        }
        (void)runtime_surface_place(runtime, surface, region);
        if (!runtime->layout_surface_visible_baseline[kind]) {
            (void)runtime_surface_hide(runtime, surface);
        }
    }
    runtime->layout_baseline_valid = 0;
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(runtime->layout_lock_button), TRUE);
    runtime_workspace_apply_geometry(runtime);
    runtime_show_notification(runtime, "Layout changes were cancelled.");
}
