/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_refresh.inc
 *
 * PURPOSE:
 *   Keep GTK presentation synchronized with the existing Studio UI adapter and
 *   the final Framework Studio Runtime while preserving live user input.
 *
 * PHASE 2:
 *   Operational widgets are created once and refreshed in place. This replaces
 *   the Phase 1 periodic panel reconstruction that was acceptable for snapshot
 *   cards but would destroy typed text in an interactive IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_services(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status;
    UmiStatus snapshot_status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Preserve the established Studio refresh path first. The new runtime is a
     * presentation/composition layer around it, not a replacement service graph.
     */
    status = umi_studio_gtk_workbench_refresh(runtime->owner);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Keep the shared header's contrast-aware SVG aligned with the active
     * Studio appearance before refreshing the rest of the workbench panels. */
    status = runtime_refresh_brand_appearance(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    snapshot_status = runtime_refresh_service_snapshots(runtime);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->runtime_bootstrap != NULL) {
        status = umi_studio_runtime_bootstrap_refresh(
            runtime->runtime_bootstrap);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->runtime_bootstrap != NULL &&
        runtime->runtime_bootstrap->platform != NULL) {
        status = umi_studio_host_sync(
            runtime->runtime_bootstrap->platform,
            &runtime->host_adapter);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    runtime->panel_refresh_tick += 1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (runtime->panel_refresh_tick >= 4U) {
        runtime->panel_refresh_tick = 0U;
        runtime_refresh_interactive_panels(runtime);
    }

    runtime_status_override_legacy_services(runtime);
    runtime_welcome_update_mode(runtime);
    runtime_refresh_command_sensitivity(runtime, runtime->toolbar);
    runtime_refresh_command_sensitivity(runtime, runtime->activity_bar);

    return snapshot_status == UMI_STATUS_OK
        ? UMI_STATUS_OK
        : snapshot_status;
}

/*
 * Provide the runtime refresh timer operation used by this module and its client
 * applications.
 */
static gboolean runtime_refresh_timer(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || !runtime->runtime_ready) {
        return G_SOURCE_REMOVE;
    }

    (void)runtime_refresh_services(runtime);
    return G_SOURCE_CONTINUE;
}
