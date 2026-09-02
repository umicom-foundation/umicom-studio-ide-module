/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_attach.inc
 *
 * PURPOSE:
 *   Attach/detach the first visible Framework Studio Runtime GTK4 composition
 *   around the established Framework GTK4 adapter.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_attach(UmiStudioGtkWorkbench *workbench)
{
    UmiStudioGtkRuntimeChrome *runtime;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || workbench->window == NULL ||
        workbench->ui == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench->runtime_chrome != NULL) return UMI_STATUS_OK;

    runtime = (UmiStudioGtkRuntimeChrome *)calloc(1U, sizeof(*runtime));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    runtime->owner = workbench;
    workbench->runtime_chrome = runtime;

    status = runtime_create_framework(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    /*
     * Read current Studio service state before building panel widgets so the
     * first frame already contains useful information instead of zero values.
     */
    (void)runtime_refresh_service_snapshots(runtime);

    status = runtime_build_shell(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_apply_theme(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_build_command_palette(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_build_layout_browser(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_build_window_catalogue(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_install_keyboard(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    status = runtime_rebuild_surface_panels(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    runtime_host_adapter_init(runtime);

    status = umi_studio_host_controller_init(
        &runtime->host_controller,
        runtime->runtime_bootstrap,
        &runtime->host_adapter);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto fail;

    g_signal_connect(
        workbench->window,
        "close-request",
        G_CALLBACK(runtime_on_close_request),
        runtime);

    runtime->refresh_source = g_timeout_add(
        UMI_STUDIO_GTK_RUNTIME_REFRESH_MS,
        runtime_refresh_timer,
        runtime);

    runtime->runtime_ready = 1;

    status = runtime_refresh_services(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        /*
         * A presentation refresh failure should remain visible to the operator,
         * but the established Framework adapter remains functional underneath.
         */
        runtime_show_notification(
            runtime,
            "Studio Runtime attached; one presentation snapshot could not refresh.");
        status = UMI_STATUS_OK;
    }

    return status;

fail:
    runtime_destroy_framework(runtime);
    free(runtime);
    workbench->runtime_chrome = NULL;
    return status;
}

/* Provide the runtime detach operation used by this module and its client applications. */
static void runtime_detach(UmiStudioGtkWorkbench *workbench)
{
    UmiStudioGtkRuntimeChrome *runtime;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return;

    runtime = workbench->runtime_chrome;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    runtime->runtime_ready = 0;
    runtime_destroy_framework(runtime);
    free(runtime);
    workbench->runtime_chrome = NULL;
}
