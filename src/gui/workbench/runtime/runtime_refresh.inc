/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_refresh.inc
 *
 * PURPOSE:
 *   Keep GTK presentation synchronized with the existing Studio UI adapter and
 *   the final Framework Studio Runtime.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_services(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status;
    UmiStatus snapshot_status;

    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Preserve the established Studio refresh path first. The new runtime is a
     * presentation/composition layer around it, not a replacement service graph.
     */
    status = umi_studio_gtk_workbench_refresh(runtime->owner);
    if (status != UMI_STATUS_OK) return status;

    snapshot_status = runtime_refresh_service_snapshots(runtime);

    if (runtime->runtime_bootstrap != NULL) {
        status = umi_studio_runtime_bootstrap_refresh(
            runtime->runtime_bootstrap);
        if (status != UMI_STATUS_OK) return status;
    }

    runtime->panel_refresh_tick += 1U;
    if (runtime->panel_refresh_tick >= 4U) {
        runtime->panel_refresh_tick = 0U;
        status = runtime_rebuild_surface_panels(runtime);
        if (status != UMI_STATUS_OK) return status;
    }

    if (runtime->runtime_bootstrap != NULL &&
        runtime->runtime_bootstrap->platform != NULL) {
        status = umi_studio_host_sync(
            runtime->runtime_bootstrap->platform,
            &runtime->host_adapter);
        if (status != UMI_STATUS_OK) return status;
    }

    runtime_status_override_legacy_services(runtime);
    runtime_refresh_command_sensitivity(runtime, runtime->toolbar);
    runtime_refresh_command_sensitivity(runtime, runtime->activity_bar);

    return snapshot_status == UMI_STATUS_OK
        ? UMI_STATUS_OK
        : snapshot_status;
}

static gboolean runtime_refresh_timer(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;

    if (runtime == NULL || !runtime->runtime_ready) {
        return G_SOURCE_REMOVE;
    }

    (void)runtime_refresh_services(runtime);
    return G_SOURCE_CONTINUE;
}
