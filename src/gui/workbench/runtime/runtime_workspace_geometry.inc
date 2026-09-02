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

/* Keep a saved panel size inside the range supported by the current monitor. */
static int runtime_clamp_dimension(int value, int minimum, int maximum)
{
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (maximum < minimum) return minimum;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (value < minimum) return minimum;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (value > maximum) return maximum;
    return value;
}

/* Keep the lock control's words, help text and visual state in agreement. */
static void runtime_workspace_update_lock_button(
    UmiStudioGtkRuntimeChrome *runtime)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->layout_lock_button == NULL) return;
    gtk_button_set_label(
        GTK_BUTTON(runtime->layout_lock_button),
        runtime->layout_locked ? "Layout Locked" : "Editing Layout");
    gtk_widget_set_tooltip_text(
        runtime->layout_lock_button,
        runtime->layout_locked
            ? "Unlock before moving or resizing panels"
            : "Lock this arrangement to prevent accidental changes");
    /* A distinct class makes edit mode visible without relying on the label
     * alone, which helps users notice when panel movement is enabled. */
    gtk_widget_remove_css_class(
        runtime->layout_lock_button, "umicom-layout-locked");
    gtk_widget_remove_css_class(
        runtime->layout_lock_button, "umicom-layout-editing");
    gtk_widget_add_css_class(
        runtime->layout_lock_button,
        runtime->layout_locked
            ? "umicom-layout-locked"
            : "umicom-layout-editing");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->layout_cancel_button != NULL) {
        gtk_widget_set_visible(
            runtime->layout_cancel_button,
            !runtime->layout_locked && runtime->layout_baseline_valid);
    }
}

/* Apply saved splitter sizes while reserving usable room for the editor. */
static void runtime_workspace_apply_geometry(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkbenchState state;
    int total_width;
    int secondary_width;
    int height;
    int side_maximum;
    int bottom_maximum;
    int primary;
    int secondary;
    int bottom;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->primary_paned == NULL ||
        runtime->secondary_paned == NULL ||
        runtime->workspace_paned == NULL) {
        return;
    }

    umi_ui_workbench_state_init(&state);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    total_width = gtk_widget_get_width(runtime->primary_paned);
    secondary_width = gtk_widget_get_width(runtime->secondary_paned);
    height = gtk_widget_get_height(runtime->workspace_paned);

    /* Reserve a useful editor area before dividing the remaining width between
     * the two optional sidebars. This prevents a layout saved on a large
     * monitor from making Studio wider than a smaller screen. */
    side_maximum =
        (total_width - UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH -
         UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_WIDTH) / 2;
    side_maximum = runtime_clamp_dimension(
        side_maximum,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MAX_WIDTH);
    bottom_maximum = (height - UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_HEIGHT) / 2;
    bottom_maximum = runtime_clamp_dimension(
        bottom_maximum,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_MAX_HEIGHT);
    primary = runtime_clamp_dimension(
        state.sidebar_size,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        side_maximum) + UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH;
    secondary = runtime_clamp_dimension(
        state.auxiliary_sidebar_size,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        side_maximum);
    bottom = runtime_clamp_dimension(
        state.bottom_panel_size,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT,
        bottom_maximum);

    runtime->applying_workspace_geometry = 1;
    gtk_paned_set_position(GTK_PANED(runtime->primary_paned), primary);
    /* Only apply a split when the editor can keep its guaranteed centre area. */
    if (secondary_width >
        secondary + UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_WIDTH) {
        gtk_paned_set_position(
            GTK_PANED(runtime->secondary_paned),
            secondary_width - secondary);
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (height > bottom + UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_HEIGHT) {
        gtk_paned_set_position(
            GTK_PANED(runtime->workspace_paned), height - bottom);
    }
    runtime->applying_workspace_geometry = 0;
}

/* Delay geometry restoration until GTK knows the window's real dimensions. */
static void runtime_on_workspace_mapped(
    GtkWidget *widget,
    gpointer user_data)
{
    (void)widget;
    runtime_workspace_apply_geometry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

/* Store a user-adjusted splitter position only while layout editing is open. */
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
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->applying_workspace_geometry ||
        runtime->owner == NULL) {
        return;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime->layout_locked) {
        runtime_workspace_apply_geometry(runtime);
        return;
    }

    umi_ui_workbench_state_init(&state);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    position = gtk_paned_get_position(GTK_PANED(paned));
    /* Apply this branch only when its contract condition is satisfied. */
    if (paned == runtime->primary_paned) {
        state.sidebar_size = runtime_clamp_dimension(
            position - UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MAX_WIDTH);
    } else /* Apply this branch only when its contract condition is satisfied. */ if (paned == runtime->secondary_paned) {
        available = gtk_widget_get_width(paned);
        state.auxiliary_sidebar_size = runtime_clamp_dimension(
            available - position,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
            UMI_STUDIO_GTK_RUNTIME_TOOL_MAX_WIDTH);
    } else /* Apply this branch only when its contract condition is satisfied. */ if (paned == runtime->workspace_paned) {
        available = gtk_widget_get_height(paned);
        state.bottom_panel_size = runtime_clamp_dimension(
            available - position,
            UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT,
            UMI_STUDIO_GTK_RUNTIME_BOTTOM_MAX_HEIGHT);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
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

/* Start or finish a reversible layout-editing session. */
static void runtime_on_layout_lock_toggled(
    GtkToggleButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int was_locked;
    int kind;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;
    was_locked = runtime->layout_locked;
    runtime->layout_locked = gtk_toggle_button_get_active(button) ? 1 : 0;
    /* Apply this branch only when its contract condition is satisfied. */
    if (was_locked && !runtime->layout_locked) {
        umi_ui_workbench_state_init(&runtime->layout_edit_baseline);
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_ui_workbench_state_snapshot(
                umi_studio_ui_workbench(runtime->owner->ui),
                &runtime->layout_edit_baseline) == UMI_STATUS_OK) {
            /* Visit each bounded item once so every record receives the same rule. */
            for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
                 kind <= (int)UMI_STUDIO_SURFACE_LAST;
                 ++kind) {
                runtime->layout_surface_region_baseline[kind] =
                    runtime->surface_regions[kind];
                runtime->layout_surface_visible_baseline[kind] =
                    runtime->surface_visible[kind];
                runtime->layout_surface_floating_baseline[kind] =
                    runtime->surface_floating[kind];
            }
            runtime->layout_baseline_valid = 1;
        }
    } else /* Apply this branch only when its contract condition is satisfied. */ if (!was_locked && runtime->layout_locked) {
        runtime->layout_baseline_valid = 0;
    }
    runtime_workspace_update_lock_button(runtime);
    /* Rebuild only presentation wrappers so panel-header buttons immediately
     * reflect the new edit lock while their Framework-backed content persists. */
    (void)runtime_rebuild_surface_panels(runtime);
    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime->layout_locked) runtime_workspace_apply_geometry(runtime);
}

/* Restore the layout snapshot captured when editing began. */
static void runtime_on_layout_cancel_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int kind;
    (void)button;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || !runtime->layout_baseline_valid) return;

    (void)umi_ui_workbench_state_apply(
        umi_studio_ui_workbench(runtime->owner->ui),
        &runtime->layout_edit_baseline);
    /* Visit each bounded item once so every record receives the same rule. */
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        UmiStudioRuntimeSurfaceKind surface =
            (UmiStudioRuntimeSurfaceKind)kind;
        UmiStudioGtkRuntimeRegion region =
            (UmiStudioGtkRuntimeRegion)
                runtime->layout_surface_region_baseline[kind];
        /* Apply this branch only when its contract condition is satisfied. */
        if (surface == UMI_STUDIO_SURFACE_EDITOR ||
            region < UMI_STUDIO_GTK_REGION_PRIMARY ||
            region > UMI_STUDIO_GTK_REGION_EDITOR) {
            continue;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (runtime->layout_surface_floating_baseline[kind]) {
            (void)runtime_surface_float(runtime, surface);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            (void)runtime_surface_place(runtime, surface, region);
        }
        /* Apply this operation only while the related capability or state is available. */
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
