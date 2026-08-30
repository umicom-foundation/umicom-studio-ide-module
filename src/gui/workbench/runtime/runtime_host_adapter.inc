/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_host_adapter.inc
 *
 * PURPOSE:
 *   Implement the thin GTK4 callbacks required by UmiStudioRuntimeHostAdapter.
 *   These callbacks contain presentation only; IDE behavior remains Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_host_set_window_title(
    void *context,
    const char *title)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    gtk_window_set_title(
        runtime->owner->window,
        title != NULL && title[0] != '\0'
            ? title
            : "Umicom Studio IDE");
    return UMI_STATUS_OK;
}

static UmiStatus runtime_host_present_surface(
    void *context,
    const UmiApplicationShellState *state)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;
    UmiStudioRuntimeSurfaceKind active;

    if (runtime == NULL || state == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    gtk_revealer_set_reveal_child(
        GTK_REVEALER(runtime->primary_revealer),
        state->primary_sidebar_visible != 0);
    gtk_revealer_set_reveal_child(
        GTK_REVEALER(runtime->secondary_revealer),
        state->secondary_sidebar_visible != 0);
    gtk_revealer_set_reveal_child(
        GTK_REVEALER(runtime->bottom_revealer),
        state->bottom_panel_visible != 0);

    gtk_widget_set_visible(
        runtime->status_bar,
        state->status_bar_visible != 0);
    gtk_widget_set_visible(
        runtime->activity_bar,
        state->zen_mode == 0);
    gtk_widget_set_visible(
        runtime->application_bar,
        state->zen_mode == 0);
    gtk_widget_set_visible(
        runtime->toolbar,
        state->zen_mode == 0);
    gtk_widget_set_visible(
        runtime->layout_tabs,
        state->zen_mode == 0);

    active = runtime_active_surface_kind(
        runtime,
        state->active_view_id);

    runtime_show_surface(runtime, active);
    runtime_update_activity_selection(runtime, active);
    return UMI_STATUS_OK;
}

static UmiStatus runtime_host_update_workflow(
    void *context,
    const UmiIdeWorkflowReport *workflow)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;
    char text[160];

    if (runtime == NULL || workflow == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    if (runtime->workflow_badge == NULL) return UMI_STATUS_OK;

    (void)snprintf(
        text,
        sizeof(text),
        "%s  %zu pass / %zu warn / %zu block",
        workflow->ready ? "Ready" : "Blocked",
        workflow->passed_count,
        workflow->warning_count,
        workflow->blocked_count);

    gtk_label_set_text(GTK_LABEL(runtime->workflow_badge), text);

    if (workflow->ready) {
        gtk_widget_remove_css_class(
            runtime->workflow_badge,
            "umicom-runtime-badge-blocked");
    } else {
        gtk_widget_add_css_class(
            runtime->workflow_badge,
            "umicom-runtime-badge-blocked");
    }

    return UMI_STATUS_OK;
}

static void runtime_host_adapter_init(
    UmiStudioGtkRuntimeChrome *runtime)
{
    (void)memset(&runtime->host_adapter, 0, sizeof(runtime->host_adapter));
    runtime->host_adapter.set_window_title =
        runtime_host_set_window_title;
    runtime->host_adapter.present_surface =
        runtime_host_present_surface;
    runtime->host_adapter.update_status =
        runtime_update_status;
    runtime->host_adapter.update_document_tabs =
        runtime_update_document_tabs;
    runtime->host_adapter.update_workflow =
        runtime_host_update_workflow;
    runtime->host_adapter.notify =
        runtime_host_notify;
    runtime->host_adapter.user_data = runtime;
}
