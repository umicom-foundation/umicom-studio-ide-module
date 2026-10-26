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

#include "umicom/studio/test_execution.h"
#include "umicom/build/job_history.h"

/* Read a copied trust/profile boundary, never borrow it in a worker. */
/* Test Explorer captures the same per-project tool folder as the build service; the worker never reads GTK or a changing profile. The previous implementation is retained for engineering review. */
#if 0
static int RuntimeTestRunContext(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioTestRunContext *context)
{
    memset(context, 0, sizeof(*context));
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return 0;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    const UmiBuildProfile *profile = services != NULL
        ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
    UmiStudioWorkspaceSnapshot workspace;
    size_t sourceLength;
    size_t buildLength;
    size_t configurationLength;

    if (profile == NULL || umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
        !workspace.graph.open || !workspace.graph.trusted) return 0;

    sourceLength = strlen(profile->source_directory);
    buildLength = strlen(profile->build_directory);
    configurationLength = strlen(profile->configuration);

#if 0
    /*
     * The original validation and formatted copies are retained for review.
     * They already reject values that do not fit, but GCC diagnoses the
     * snprintf calls from the larger source-array capacities before considering
     * those runtime length checks. The active implementation below keeps the
     * same fit-or-reject behaviour and copies the already measured bytes.
     */
    if (strlen(profile->source_directory) >= sizeof(context->source_root) ||
        strlen(profile->build_directory) >= sizeof(context->build_root) ||
        strlen(profile->configuration) >= sizeof(context->configuration)) return 0;
    context->trusted = 1;
    context->workspace_generation = workspace.graph.revision;
    (void)snprintf(context->source_root, sizeof(context->source_root), "%s", profile->source_directory);
    (void)snprintf(context->build_root, sizeof(context->build_root), "%s", profile->build_directory);
    (void)snprintf(context->configuration, sizeof(context->configuration), "%s", profile->configuration);
#endif

    if (sourceLength >= sizeof(context->source_root) ||
        buildLength >= sizeof(context->build_root) ||
        configurationLength >= sizeof(context->configuration)) return 0;

    /* The shared profile fingerprint has the same meaning in build and test
     * history. This reads settings only; source files and compiler binaries are
     * deliberately not read on the GUI refresh path. */
    if (UmiBuildProfileJobIdentity(profile, &context->identity) != UMI_STATUS_OK)
        return 0;
    context->trusted = 1;
    context->workspace_generation = workspace.graph.revision;
    (void)memcpy(context->source_root, profile->source_directory, sourceLength + 1U);
    (void)memcpy(context->build_root, profile->build_directory, buildLength + 1U);
    (void)memcpy(context->configuration, profile->configuration, configurationLength + 1U);
    return 1;
}
#endif
/* Test execution now shares Framework build-folder resolution with discovery. The previous literal profile copy is retained for engineering review. */
#if 0
static int RuntimeTestRunContext(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioTestRunContext *context)
{
    memset(context, 0, sizeof(*context));
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return 0;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    const UmiBuildProfile *profile = services != NULL
        ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
    UmiStudioWorkspaceSnapshot workspace;
    size_t sourceLength;
    size_t buildLength;
    size_t configurationLength;

    if (profile == NULL || umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
        !workspace.graph.open || !workspace.graph.trusted) return 0;

    sourceLength = strlen(profile->source_directory);
    buildLength = strlen(profile->build_directory);
    configurationLength = strlen(profile->configuration);

#if 0
    /*
     * The original validation and formatted copies are retained for review.
     * They already reject values that do not fit, but GCC diagnoses the
     * snprintf calls from the larger source-array capacities before considering
     * those runtime length checks. The active implementation below keeps the
     * same fit-or-reject behaviour and copies the already measured bytes.
     */
    if (strlen(profile->source_directory) >= sizeof(context->source_root) ||
        strlen(profile->build_directory) >= sizeof(context->build_root) ||
        strlen(profile->configuration) >= sizeof(context->configuration)) return 0;
    context->trusted = 1;
    context->workspace_generation = workspace.graph.revision;
    (void)snprintf(context->source_root, sizeof(context->source_root), "%s", profile->source_directory);
    (void)snprintf(context->build_root, sizeof(context->build_root), "%s", profile->build_directory);
    (void)snprintf(context->configuration, sizeof(context->configuration), "%s", profile->configuration);
#endif

    if (sourceLength >= sizeof(context->source_root) ||
        buildLength >= sizeof(context->build_root) ||
        configurationLength >= sizeof(context->configuration)) return 0;

    /* The shared profile fingerprint has the same meaning in build and test
     * history. This reads settings only; source files and compiler binaries are
     * deliberately not read on the GUI refresh path. */
    if (UmiBuildProfileJobIdentity(profile, &context->identity) != UMI_STATUS_OK)
        return 0;
    if (strlen(profile->tool_directory) >= sizeof context->tool_directory) return 0;
    memcpy(context->tool_directory,profile->tool_directory,strlen(profile->tool_directory)+1U);
    context->trusted = 1;
    context->workspace_generation = workspace.graph.revision;
    (void)memcpy(context->source_root, profile->source_directory, sourceLength + 1U);
    (void)memcpy(context->build_root, profile->build_directory, buildLength + 1U);
    (void)memcpy(context->configuration, profile->configuration, configurationLength + 1U);
    return 1;
}

#endif
static int RuntimeTestRunContext(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioTestRunContext *context)
{
    memset(context, 0, sizeof(*context));
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return 0;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    const UmiBuildProfile *profile = services != NULL
        ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
    UmiStudioWorkspaceSnapshot workspace;
    if (profile == NULL || umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
        !workspace.graph.open || !workspace.graph.trusted ||
        !umi_path_equal(profile->source_directory, workspace.graph.root)) return 0;
    /* Match discovery and execution using the same absolute folder. Relative
     * settings remain useful to the developer, while queued jobs own a complete
     * path independent of Studio's process directory. */
    if (UmiBuildProfileBuildDirectory(profile, workspace.graph.root,
        context->build_root, sizeof context->build_root) != UMI_STATUS_OK) return 0;
    size_t sourceLength = strlen(profile->source_directory);
    size_t configurationLength = strlen(profile->configuration);
    size_t toolLength = strlen(profile->tool_directory);
    if (sourceLength >= sizeof context->source_root ||
        configurationLength >= sizeof context->configuration ||
        toolLength >= sizeof context->tool_directory) return 0;
    if (UmiBuildProfileJobIdentity(profile, &context->identity) != UMI_STATUS_OK) return 0;
    memcpy(context->source_root, profile->source_directory, sourceLength + 1U);
    memcpy(context->configuration, profile->configuration, configurationLength + 1U);
    memcpy(context->tool_directory, profile->tool_directory, toolLength + 1U);
    context->trusted = 1;
    context->workspace_generation = workspace.graph.revision;
    return 1;
}

/* Completion notifications can run GTK observers which close or replace the
 * workbench. Reacquire its existing weak binding before the polling caller
 * touches host-owned state again. Holding the window alone cannot keep the
 * Studio runtime alive. */
static UmiStatus RuntimeNotifyAndContinue(UmiStudioGtkRuntimeChrome *runtime,
    const char *message)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    runtime_show_notification(runtime, message);
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *current = runtime_file_dialog_owner(request, &window);
    UmiStatus status = current != NULL ? UMI_STATUS_OK : UMI_STATUS_CANCELLED;
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Native pane, menu and output updates can retire the host during refresh. The replacement rechecks captured application identity after each update and returns cancellation; preserve the prior service sequence for review. */
#if 0
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

    /* Publish a completed private catalogue on the owning GUI thread before
     * the existing refresh reads it. Poll never waits for CTest to complete. */
    if (runtime->owner->ui != NULL) {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        UmiStudioTestService *tests = umi_studio_services_tests(services);
        if (UmiStudioTestDiscoveryPending(tests)) {
            UmiStudioWorkspaceSnapshot workspace;
            const UmiBuildProfile *profile = umi_studio_build_service_profile(
                umi_studio_services_build(services));
            int allowed = umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
                workspace.graph.open && workspace.graph.trusted;
            int published = 0;
            char message[512];
/* Discovery publication now checks the selected CTest folder as well as workspace paths and trust. The previous implementation is retained for engineering review. */
#if 0
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPoll(tests,
                profile != NULL ? profile->source_directory : NULL,
                profile != NULL ? profile->build_directory : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
#endif
/* Compare the resolved build root captured at submission, so relative profile settings do not invalidate their own discovery result. The previous comparison is retained for review. */
#if 0
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPollWithToolDirectory(tests,
                profile != NULL ? profile->tool_directory : NULL,
                profile != NULL ? profile->source_directory : NULL,
                profile != NULL ? profile->build_directory : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
#endif
            char buildRoot[UMI_BUILD_PATH_CAPACITY];
            allowed = allowed && profile != NULL &&
                umi_path_equal(profile->source_directory, workspace.graph.root) &&
                UmiBuildProfileBuildDirectory(profile, workspace.graph.root,
                    buildRoot, sizeof buildRoot) == UMI_STATUS_OK;
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPollWithToolDirectory(tests,
                profile != NULL ? profile->tool_directory : NULL,
                profile != NULL ? profile->source_directory : NULL,
                allowed ? buildRoot : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
            if (discoveryStatus != UMI_STATUS_BUSY && discoveryStatus != UMI_STATUS_NOT_FOUND)
                runtime_show_notification(runtime, message[0] != '\0' ? message :
                    umi_status_text(discoveryStatus));
#endif

            if (discoveryStatus != UMI_STATUS_BUSY && discoveryStatus != UMI_STATUS_NOT_FOUND) {
                status = RuntimeNotifyAndContinue(runtime, message[0] != '\0' ? message :
                    umi_status_text(discoveryStatus));
                if (status != UMI_STATUS_OK) return status;
            }
        }
    }

    if (runtime->owner->ui != NULL) {
        UmiStudioTestService *tests = umi_studio_services_tests(umi_studio_ui_services(runtime->owner->ui));
        if (UmiStudioTestRunPending(tests)) {
            UmiStudioTestRunContext context;
            (void)RuntimeTestRunContext(runtime, &context);
            char message[512];
            UmiStatus runStatus = UmiStudioTestRunPoll(tests, &context, message, sizeof(message));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
            if (runStatus != UMI_STATUS_BUSY && runStatus != UMI_STATUS_NOT_FOUND)
                runtime_show_notification(runtime, message[0] ? message : umi_status_text(runStatus));
#endif

            if (runStatus != UMI_STATUS_BUSY && runStatus != UMI_STATUS_NOT_FOUND) {
                status = RuntimeNotifyAndContinue(runtime, message[0] ? message : umi_status_text(runStatus));
                if (status != UMI_STATUS_OK) return status;
            }
        }
    }

    /*
     * Preserve the established Studio refresh path first. The new runtime is a
     * presentation/composition layer around it, not a replacement service graph.
     */
    RuntimeTerminalPoll(runtime);
    status = umi_studio_gtk_workbench_refresh(runtime->owner);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Synchronize project identity after all open/close routes, including menus
     * and session restore; do not depend on which folder chooser was used. */
    status = runtime_prepare_workspace_identity(runtime);
    if (status != UMI_STATUS_OK) return status;

    UmiStudioServices *developmentServices = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioDebuggerService *nativeDebugger = umi_studio_services_debugger(developmentServices);
    UmiStudioWorkspaceSnapshot developmentWorkspace;
    if (UmiStudioDebuggerNativeBusy(nativeDebugger) &&
        (umi_studio_workspace_snapshot(developmentServices, &developmentWorkspace) != UMI_STATUS_OK ||
         !developmentWorkspace.graph.open || !developmentWorkspace.graph.trusted)) {
        (void)umi_studio_debugger_service_stop(nativeDebugger, 0);
    } else {
        UmiStatus debugStatus = UmiStudioDebuggerPollNative(nativeDebugger,
            umi_studio_services_build(developmentServices));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
        if (debugStatus != UMI_STATUS_OK && debugStatus != UMI_STATUS_NOT_FOUND)
            runtime_show_notification(runtime, umi_status_text(debugStatus));
#endif

        if (debugStatus != UMI_STATUS_OK && debugStatus != UMI_STATUS_NOT_FOUND) {
            status = RuntimeNotifyAndContinue(runtime, umi_status_text(debugStatus));
            if (status != UMI_STATUS_OK) return status;
        }
    }

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
    /* This call is cheap when the document-model revision is unchanged. It
     * gives the preview a short natural debounce without rebuilding widgets. */
    (void)runtime_live_preview_refresh(runtime, 0);
    RuntimeRefreshBuildOutput(runtime);
    RuntimeFileSearchRefresh(runtime);
    runtime_refresh_command_sensitivity(runtime, runtime->menu_bar);
    runtime_refresh_command_sensitivity(runtime, runtime->surface_panels[UMI_STUDIO_SURFACE_BUILD]);
    runtime_refresh_command_sensitivity(runtime, runtime->toolbar);
    runtime_refresh_command_sensitivity(runtime, runtime->activity_bar);

    RuntimeTestLiveOutputRefresh(runtime);
    runtime_terminal_refresh_transcript(runtime);

    if (runtime->test_summary_label != NULL && runtime->owner->ui != NULL) {
        UmiStudioTestService *tests = umi_studio_services_tests(umi_studio_ui_services(runtime->owner->ui));
        UmiCtestJobSnapshot progress;
        if (UmiStudioTestRunPending(tests) &&
            UmiStudioTestRunSnapshot(tests, &progress) == UMI_STATUS_OK) {
            char *name = g_utf8_make_valid(progress.active_name, -1);
            const char *phase = progress.state == UMI_TASK_QUEUED ? "Queued" :
                progress.state == UMI_TASK_RUNNING ? "Running " : "Publishing results";
            char *summary = g_strdup_printf(
                "%zu / %zu attempts complete — %zu passed, %zu failed — %s%s",
                progress.completed, progress.planned, progress.passed, progress.failed, phase, name);
            gtk_label_set_text(GTK_LABEL(runtime->test_summary_label), summary);
            g_free(summary);
            g_free(name);
        }
    }

    return snapshot_status == UMI_STATUS_OK
        ? UMI_STATUS_OK
        : snapshot_status;
}

#endif
static UmiStatus runtime_refresh_services(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status = UMI_STATUS_OK;
    UmiStatus snapshot_status = UMI_STATUS_OK;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;

    /* Publish a completed private catalogue on the owning GUI thread before
     * the existing refresh reads it. Poll never waits for CTest to complete. */
    if (runtime->owner->ui != NULL) {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        UmiStudioTestService *tests = umi_studio_services_tests(services);
        if (UmiStudioTestDiscoveryPending(tests)) {
            UmiStudioWorkspaceSnapshot workspace;
            const UmiBuildProfile *profile = umi_studio_build_service_profile(
                umi_studio_services_build(services));
            int allowed = umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
                workspace.graph.open && workspace.graph.trusted;
            int published = 0;
            char message[512];
/* Discovery publication now checks the selected CTest folder as well as workspace paths and trust. The previous implementation is retained for engineering review. */
#if 0
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPoll(tests,
                profile != NULL ? profile->source_directory : NULL,
                profile != NULL ? profile->build_directory : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
#endif
/* Compare the resolved build root captured at submission, so relative profile settings do not invalidate their own discovery result. The previous comparison is retained for review. */
#if 0
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPollWithToolDirectory(tests,
                profile != NULL ? profile->tool_directory : NULL,
                profile != NULL ? profile->source_directory : NULL,
                profile != NULL ? profile->build_directory : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
#endif
            char buildRoot[UMI_BUILD_PATH_CAPACITY];
            allowed = allowed && profile != NULL &&
                umi_path_equal(profile->source_directory, workspace.graph.root) &&
                UmiBuildProfileBuildDirectory(profile, workspace.graph.root,
                    buildRoot, sizeof buildRoot) == UMI_STATUS_OK;
            UmiStatus discoveryStatus = UmiStudioTestDiscoveryPollWithToolDirectory(tests,
                profile != NULL ? profile->tool_directory : NULL,
                profile != NULL ? profile->source_directory : NULL,
                allowed ? buildRoot : NULL,
                profile != NULL ? profile->configuration : NULL,
                allowed ? workspace.graph.revision : 0U, allowed,
                &published, message, sizeof(message));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
            if (discoveryStatus != UMI_STATUS_BUSY && discoveryStatus != UMI_STATUS_NOT_FOUND)
                runtime_show_notification(runtime, message[0] != '\0' ? message :
                    umi_status_text(discoveryStatus));
#endif

            if (discoveryStatus != UMI_STATUS_BUSY && discoveryStatus != UMI_STATUS_NOT_FOUND) {
                status = RuntimeNotifyAndContinue(runtime, message[0] != '\0' ? message :
                    umi_status_text(discoveryStatus));
                if (status != UMI_STATUS_OK) goto refresh_done;
            }
        }
    }

    if (runtime->owner->ui != NULL) {
        UmiStudioTestService *tests = umi_studio_services_tests(umi_studio_ui_services(runtime->owner->ui));
        if (UmiStudioTestRunPending(tests)) {
            UmiStudioTestRunContext context;
            (void)RuntimeTestRunContext(runtime, &context);
            char message[512];
            UmiStatus runStatus = UmiStudioTestRunPoll(tests, &context, message, sizeof(message));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
            if (runStatus != UMI_STATUS_BUSY && runStatus != UMI_STATUS_NOT_FOUND)
                runtime_show_notification(runtime, message[0] ? message : umi_status_text(runStatus));
#endif

            if (runStatus != UMI_STATUS_BUSY && runStatus != UMI_STATUS_NOT_FOUND) {
                status = RuntimeNotifyAndContinue(runtime, message[0] ? message : umi_status_text(runStatus));
                if (status != UMI_STATUS_OK) goto refresh_done;
            }
        }
    }

    /*
     * Preserve the established Studio refresh path first. The new runtime is a
     * presentation/composition layer around it, not a replacement service graph.
     */
    RuntimeTerminalPoll(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    status = umi_studio_gtk_workbench_refresh(runtime->owner);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto refresh_done;

    /* Synchronize project identity after all open/close routes, including menus
     * and session restore; do not depend on which folder chooser was used. */
    status = runtime_prepare_workspace_identity(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    if (status != UMI_STATUS_OK) goto refresh_done;

    UmiStudioServices *developmentServices = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioDebuggerService *nativeDebugger = umi_studio_services_debugger(developmentServices);
    UmiStudioWorkspaceSnapshot developmentWorkspace;
    if (UmiStudioDebuggerNativeBusy(nativeDebugger) &&
        (umi_studio_workspace_snapshot(developmentServices, &developmentWorkspace) != UMI_STATUS_OK ||
         !developmentWorkspace.graph.open || !developmentWorkspace.graph.trusted)) {
        (void)umi_studio_debugger_service_stop(nativeDebugger, 0);
    } else {
        UmiStatus debugStatus = UmiStudioDebuggerPollNative(nativeDebugger,
            umi_studio_services_build(developmentServices));
/* Completion feedback now rechecks the workbench lifetime before continuing refresh. The original notification path is retained for review. */
#if 0
        if (debugStatus != UMI_STATUS_OK && debugStatus != UMI_STATUS_NOT_FOUND)
            runtime_show_notification(runtime, umi_status_text(debugStatus));
#endif

        if (debugStatus != UMI_STATUS_OK && debugStatus != UMI_STATUS_NOT_FOUND) {
            status = RuntimeNotifyAndContinue(runtime, umi_status_text(debugStatus));
            if (status != UMI_STATUS_OK) goto refresh_done;
        }
    }

    /* Keep the shared header's contrast-aware SVG aligned with the active
     * Studio appearance before refreshing the rest of the workbench panels. */
    status = runtime_refresh_brand_appearance(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) goto refresh_done;

    snapshot_status = runtime_refresh_service_snapshots(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->runtime_bootstrap != NULL) {
        status = umi_studio_runtime_bootstrap_refresh(
            runtime->runtime_bootstrap);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) goto refresh_done;
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
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) goto refresh_done;
    }

    runtime->panel_refresh_tick += 1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (runtime->panel_refresh_tick >= 4U) {
        runtime->panel_refresh_tick = 0U;
        runtime_refresh_interactive_panels(runtime);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    }

    runtime_status_override_legacy_services(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_welcome_update_mode(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    /* This call is cheap when the document-model revision is unchanged. It
     * gives the preview a short natural debounce without rebuilding widgets. */
    (void)runtime_live_preview_refresh(runtime, 0);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    RuntimeRefreshBuildOutput(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    RuntimeFileSearchRefresh(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_refresh_command_sensitivity(runtime, runtime->menu_bar);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_refresh_command_sensitivity(runtime, runtime->surface_panels[UMI_STUDIO_SURFACE_BUILD]);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_refresh_command_sensitivity(runtime, runtime->toolbar);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_refresh_command_sensitivity(runtime, runtime->activity_bar);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    /* Editor actions live inside the retained centre body, outside the four
     * chrome containers above. Refresh their existing command policy too. */
    runtime_refresh_command_sensitivity(runtime, runtime->centre_stack);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }

    RuntimeTestLiveOutputRefresh(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
    runtime_terminal_refresh_transcript(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }

    if (runtime->test_summary_label != NULL && runtime->owner->ui != NULL) {
        UmiStudioTestService *tests = umi_studio_services_tests(umi_studio_ui_services(runtime->owner->ui));
        UmiCtestJobSnapshot progress;
        if (UmiStudioTestRunPending(tests) &&
            UmiStudioTestRunSnapshot(tests, &progress) == UMI_STATUS_OK) {
            char *name = g_utf8_make_valid(progress.active_name, -1);
            const char *phase = progress.state == UMI_TASK_QUEUED ? "Queued" :
                progress.state == UMI_TASK_RUNNING ? "Running " : "Publishing results";
            char *summary = g_strdup_printf(
                "%zu / %zu attempts complete — %zu passed, %zu failed — %s%s",
                progress.completed, progress.planned, progress.passed, progress.failed, phase, name);
            gtk_label_set_text(GTK_LABEL(runtime->test_summary_label), summary);
            g_free(summary);
            g_free(name);
        }
    }

    status = snapshot_status == UMI_STATUS_OK ? UMI_STATUS_OK : snapshot_status;
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto refresh_done; }
refresh_done:
    g_clear_object(&window); runtime_file_dialog_request_free(request);
    return status;
}


/*
 * Provide the runtime refresh timer operation used by this module and its client
 * applications.
 */
/* A width or evidence callback can retire Studio during a timer tick. Reacquire the owner and retire the timer when the host has gone; retain the earlier callback for review. */
#if 0
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

    /* GTK4 has no widget-width notify property. Reuse the existing timer to
     * adapt search after allocation instead of connecting an inert signal. */
    runtime_update_application_bar_width(runtime);
    (void)runtime_refresh_services(runtime);
    return G_SOURCE_CONTINUE;
}

#endif
static gboolean runtime_refresh_timer(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    if (runtime == NULL || !runtime->runtime_ready) return G_SOURCE_REMOVE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return G_SOURCE_CONTINUE;
    GObject *window = NULL;
    runtime_update_application_bar_width(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime != NULL) (void)runtime_refresh_services(runtime);
    runtime = RuntimeResolveOwner(request, &window);
    gboolean keep = runtime != NULL && runtime->runtime_ready;
    g_clear_object(&window); runtime_file_dialog_request_free(request);
    return keep ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}
