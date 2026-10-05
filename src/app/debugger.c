/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/debugger.c
 *
 * PURPOSE:
 *   Implement Studio debug-adapter protocol state and source breakpoint ownership.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include "umicom/build/launch_plan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioDebuggerService {
    UmiProtocolTransport *transport;
    UmiProtocolClient *client;
    UmiDapClient dap;
    UmiDapBreakpointRegistry *breakpoints;
    UmiDebugService *model;
    UmiDebugController *controller;
    UmiDebugWorkspace *workspace;
    UmiDebugRuntimePlatform *native;
    UmiBuildProfile launchProfile;
    uint64_t launchOperation;
    char nativeKind[16];
    char nativeExecutable[1024];
    int nativeMode;
    int launchPending;
    int inspectedStop;
    UmiStatus nativeStatus;
};

/*
 * Initialise studio debugger service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debugger_service_create(
    UmiStudioDebuggerService **out_service)
{
    UmiStudioDebuggerService *service;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_service = NULL;
    service = (UmiStudioDebuggerService *)calloc(1U, sizeof(*service));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_protocol_transport_create_memory(256U, &service->transport);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_protocol_client_create(service->transport,
                                            &service->client);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_protocol_client_start(service->client);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_dap_client_init(&service->dap, service->client);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_dap_breakpoint_registry_create(&service->breakpoints);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    /* The Debug Runtime now owns the single Debug Service. The previous
     * allocation is retained below; legacy protocol tests share this same model.
     * if (status == UMI_STATUS_OK) status = umi_debug_service_create(&service->model); */
    if (status == UMI_STATUS_OK) status = umi_debug_runtime_platform_create(&service->native);
    if (status == UMI_STATUS_OK) service->model = umi_debug_runtime_platform_service(service->native);
#ifdef _WIN32
    (void)snprintf(service->nativeKind, sizeof(service->nativeKind), "gdb");
#else
    (void)snprintf(service->nativeKind, sizeof(service->nativeKind), "lldb");
#endif
    service->nativeMode = 1;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_debug_controller_create(&service->dap, service->model,
                                             &service->controller);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_debug_workspace_create(service->model,
                                            service->controller,
                                            &service->workspace);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_debugger_service_destroy(service);
        return status;
    }
    *out_service = service;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio debugger service so the same storage can be reused
 * safely.
 */
void umi_studio_debugger_service_destroy(UmiStudioDebuggerService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_debug_workspace_destroy(service->workspace);
    umi_debug_controller_destroy(service->controller);
    /* Former umi_debug_service_destroy(service->model) is now performed by
     * Framework's platform destructor after the borrowed workspace is gone. */
    umi_debug_runtime_platform_destroy(service->native);
    umi_dap_breakpoint_registry_destroy(service->breakpoints);
    umi_protocol_client_destroy(service->client);
    umi_protocol_transport_destroy(service->transport);
    free(service);
}

/*
 * Initialise studio debugger service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debugger_service_initialize(
    UmiStudioDebuggerService *service,
    const char *adapter_id,
    int64_t *out_request_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioDebuggerNativeBusy(service)) return UMI_STATUS_BUSY;
    service->nativeMode = 0;
    {
        UmiStatus status = umi_debug_controller_initialize(
            service->controller, adapter_id);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (status == UMI_STATUS_OK && out_request_id != NULL) {
            UmiDebugControllerSnapshot snapshot;
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_debug_controller_snapshot(service->controller,
                                               &snapshot) == UMI_STATUS_OK) {
                *out_request_id = snapshot.last_request_id;
            }
        }
        return status;
    }
}

/*
 * Provide the studio debugger service launch operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_launch(
    UmiStudioDebuggerService *service,
    const char *program,
    const char *working_directory,
    int64_t *out_request_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    {
        UmiStatus status = umi_debug_controller_launch(
            service->controller, program, working_directory);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (status == UMI_STATUS_OK && out_request_id != NULL) {
            UmiDebugControllerSnapshot snapshot;
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_debug_controller_snapshot(service->controller,
                                               &snapshot) == UMI_STATUS_OK) {
                *out_request_id = snapshot.last_request_id;
            }
        }
        return status;
    }
}

/*
 * Provide the studio debugger service start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_start(UmiStudioDebuggerService *service,
    const char *adapter_id, const char *program, const char *working_directory)
{
    UmiDebugControllerSnapshot snapshot;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || adapter_id == NULL || program == NULL ||
        working_directory == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioDebuggerNativeBusy(service)) return UMI_STATUS_BUSY;
    service->nativeMode = 0;
    status = umi_debug_controller_snapshot(service->controller, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK &&
        (snapshot.state == UMI_DEBUG_CONTROLLER_IDLE ||
         snapshot.state == UMI_DEBUG_CONTROLLER_TERMINATED)) {
        status = umi_debug_controller_initialize(service->controller,
                                                 adapter_id);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_debug_controller_launch(service->controller, program,
                                             working_directory);
    }
    return status;
}

static UmiStatus NativeThreadAction(UmiStudioDebuggerService *service,
    int threadId, UmiStatus (*action)(UmiDebugRuntimePlatform *, uint64_t, uint32_t));

/*
 * Provide the studio debugger service continue operation used by this module and its
 * client applications.
 */
// UmiStatus umi_studio_debugger_service_continue(UmiStudioDebuggerService *service,int thread_id){return service!=NULL?umi_debug_controller_continue(service->controller,thread_id):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_continue(UmiStudioDebuggerService *service,int thread_id)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->nativeMode) return NativeThreadAction(service, thread_id,
        umi_debug_runtime_platform_continue);
    return umi_debug_controller_continue(service->controller, thread_id);
}
/*
 * Provide the studio debugger service pause operation used by this module and its client
 * applications.
 */
// UmiStatus umi_studio_debugger_service_pause(UmiStudioDebuggerService *service,int thread_id){return service!=NULL?umi_debug_controller_pause(service->controller,thread_id):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_pause(UmiStudioDebuggerService *service,int thread_id)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->nativeMode) return NativeThreadAction(service, thread_id,
        umi_debug_runtime_platform_pause);
    return umi_debug_controller_pause(service->controller, thread_id);
}
/*
 * Provide the studio debugger service next operation used by this module and its client
 * applications.
 */
// UmiStatus umi_studio_debugger_service_next(UmiStudioDebuggerService *service,int thread_id){return service!=NULL?umi_debug_controller_next(service->controller,thread_id):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_next(UmiStudioDebuggerService *service,int thread_id)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->nativeMode) return NativeThreadAction(service, thread_id,
        umi_debug_runtime_platform_step_over);
    return umi_debug_controller_next(service->controller, thread_id);
}
/*
 * Provide the studio debugger service step in operation used by this module and its client
 * applications.
 */
// UmiStatus umi_studio_debugger_service_step_in(UmiStudioDebuggerService *service,int thread_id){return service!=NULL?umi_debug_controller_step_in(service->controller,thread_id):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_step_in(UmiStudioDebuggerService *service,int thread_id)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->nativeMode) return NativeThreadAction(service, thread_id,
        umi_debug_runtime_platform_step_into);
    return umi_debug_controller_step_in(service->controller, thread_id);
}
/*
 * Provide the studio debugger service step out operation used by this module and its
 * client applications.
 */
// UmiStatus umi_studio_debugger_service_step_out(UmiStudioDebuggerService *service,int thread_id){return service!=NULL?umi_debug_controller_step_out(service->controller,thread_id):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_step_out(UmiStudioDebuggerService *service,int thread_id)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->nativeMode) return NativeThreadAction(service, thread_id,
        umi_debug_runtime_platform_step_out);
    return umi_debug_controller_step_out(service->controller, thread_id);
}
/*
 * Provide the studio debugger service stop operation used by this module and its client
 * applications.
 */
// UmiStatus umi_studio_debugger_service_stop(UmiStudioDebuggerService *service,int restart){return service!=NULL?umi_debug_controller_terminate(service->controller,restart):UMI_STATUS_INVALID_ARGUMENT;}
UmiStatus umi_studio_debugger_service_stop(UmiStudioDebuggerService *service,int restart)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!service->nativeMode) return umi_debug_controller_terminate(service->controller,restart);
    if (restart) return UMI_STATUS_NOT_IMPLEMENTED; /* Explicit Stop then Debug rebuilds safely. */
    service->launchPending = 0;
    UmiStatus status = umi_debug_runtime_platform_stop(service->native, 1, 1500U);
    if (status == UMI_STATUS_NOT_FOUND) status = UMI_STATUS_OK;
    service->inspectedStop = 0;
    service->nativeStatus = status;
    return status;
}

/*
 * Provide the studio debugger service add breakpoint operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_add_breakpoint(
    UmiStudioDebuggerService *service,
    const char *source_path,
    int line,
    int column)
{
    UmiDapBreakpoint breakpoint;
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || source_path == NULL || line <= 0) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    length = strlen(source_path);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length + 1U > sizeof(breakpoint.source_path)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    (void)memset(&breakpoint, 0, sizeof(breakpoint));
    (void)memcpy(breakpoint.source_path, source_path, length + 1U);
    breakpoint.line = line;
    breakpoint.column = column;
    breakpoint.enabled = 1;
    {
        UmiStatus status = umi_dap_breakpoint_add(service->breakpoints,
                                                  &breakpoint);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            UmiDebugBreakpointSnapshot model_breakpoint = {0};
            (void)snprintf(model_breakpoint.id, sizeof(model_breakpoint.id),
                           "%s:%d:%d", source_path, line, column);
            (void)snprintf(model_breakpoint.uri,
                           sizeof(model_breakpoint.uri), "%s", source_path);
            model_breakpoint.line = (uint32_t)line;
            model_breakpoint.column = column > 0 ? (uint32_t)column : 0U;
            model_breakpoint.enabled = 1;
            status = umi_debug_breakpoint_registry_upsert(
                umi_debug_service_breakpoint(service->model),
                &model_breakpoint);
        }
        if (status == UMI_STATUS_OK && UmiStudioDebuggerNativeBusy(service) && !service->launchPending)
            status = umi_debug_runtime_platform_sync_breakpoints(service->native, source_path, 1500U);
        return status;
    }
}

/*
 * Provide the studio debugger service set breakpoint enabled operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_debugger_service_set_breakpoint_enabled(
    UmiStudioDebuggerService *service, const char *breakpoint_id, int enabled)
{
    UmiDebugBreakpointSnapshot model_breakpoint;
    UmiDapBreakpoint dap_breakpoint;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || breakpoint_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_debug_breakpoint_registry_find(
        umi_debug_service_breakpoint(service->model), breakpoint_id,
        &model_breakpoint);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_debug_workspace_set_breakpoint_enabled(
        service->workspace, breakpoint_id, enabled);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    /* Keep the DAP request registry aligned with the Framework model. */
    (void)memset(&dap_breakpoint, 0, sizeof(dap_breakpoint));
    (void)snprintf(dap_breakpoint.source_path,
                   sizeof(dap_breakpoint.source_path), "%s",
                   model_breakpoint.uri);
    dap_breakpoint.line = (int)model_breakpoint.line;
    dap_breakpoint.column = (int)model_breakpoint.column;
    dap_breakpoint.enabled = enabled != 0;
    dap_breakpoint.verified = model_breakpoint.verified;
    (void)umi_dap_breakpoint_remove(service->breakpoints,
                                    dap_breakpoint.source_path,
                                    dap_breakpoint.line);
    /* Retain the desired breakpoint even if the adapter rejects it; the returned
     * error reports that it is not yet installed. Framework owns DAP syncing.
     * Previous local-only completion: */
    // return umi_dap_breakpoint_add(service->breakpoints, &dap_breakpoint);
    status = umi_dap_breakpoint_add(service->breakpoints, &dap_breakpoint);
    if (status == UMI_STATUS_OK && UmiStudioDebuggerNativeBusy(service) && !service->launchPending)
        status = umi_debug_runtime_platform_sync_breakpoints(service->native, model_breakpoint.uri, 1500U);
    return status;
}

/*
 * Provide the studio debugger service remove breakpoint operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_debugger_service_remove_breakpoint(
    UmiStudioDebuggerService *service, const char *breakpoint_id)
{
    UmiDebugBreakpointSnapshot breakpoint;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || breakpoint_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_debug_breakpoint_registry_find(
        umi_debug_service_breakpoint(service->model), breakpoint_id,
        &breakpoint);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    (void)umi_dap_breakpoint_remove(service->breakpoints, breakpoint.uri,
                                    (int)breakpoint.line);
    /* Removal must also send the remaining source breakpoints to the adapter.
     * Previous local-only operation is retained here. */
    // return umi_debug_workspace_remove_breakpoint(service->workspace, breakpoint_id);
    status = umi_debug_workspace_remove_breakpoint(service->workspace, breakpoint_id);
    if (status == UMI_STATUS_OK && UmiStudioDebuggerNativeBusy(service) && !service->launchPending)
        status = umi_debug_runtime_platform_sync_breakpoints(service->native, breakpoint.uri, 1500U);
    return status;
}

/*
 * Provide the studio debugger service add watch operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_add_watch(
    UmiStudioDebuggerService *service, const char *expression,
    char *out_watch_id, size_t out_watch_id_capacity)
{
    /* Keep the same watch registry. An explicit Add Watch while paused requests
     * its value through Framework; we never evaluate expressions on every tick.
     * Former registry-only operation retained for migration reference. */
    // return service != NULL
    //     ? umi_debug_workspace_add_watch(service->workspace, expression,
    //                                    out_watch_id, out_watch_id_capacity)
    //     : UMI_STATUS_INVALID_ARGUMENT;
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus status = umi_debug_workspace_add_watch(service->workspace, expression,
        out_watch_id, out_watch_id_capacity);
    if (status == UMI_STATUS_OK && service->nativeMode) {
        UmiDebugRuntimePlatformSnapshot snapshot;
        status = umi_debug_runtime_platform_snapshot(service->native, &snapshot);
        if (status == UMI_STATUS_OK && snapshot.active && snapshot.paused)
            status = umi_debug_runtime_platform_evaluate_watch(service->native,
                out_watch_id, expression, snapshot.active_frame_id, 1500U);
    }
    return status;
}

/*
 * Provide the studio debugger service remove watch operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_remove_watch(
    UmiStudioDebuggerService *service, const char *watch_id)
{
    return service != NULL
        ? umi_debug_workspace_remove_watch(service->workspace, watch_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio debugger service select thread operation used by this module and its
 * client applications.
 */
/* Native inspection belongs to a stopped process. Check before mutating the
 * shared selection, so commands cannot report success against an inactive or
 * running session while retained model rows still exist. */
static UmiStatus StudioDebugInspectionReady(UmiStudioDebuggerService *service)
{
    if(service==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    if(!service->nativeMode)return UMI_STATUS_OK;
    UmiDebugRuntimePlatformSnapshot state;
    UmiStatus status=umi_debug_runtime_platform_snapshot(service->native,&state);
    if(status!=UMI_STATUS_OK)return status;
    return state.active&&state.paused?UMI_STATUS_OK:UMI_STATUS_INVALID_STATE;
}

UmiStatus umi_studio_debugger_service_select_thread(
    UmiStudioDebuggerService *service, const char *thread_id)
{
    /* Previous selection-only path is retained below. Native inspection uses
     * the existing Framework platform and the same model registries. */
    // return service != NULL
    //     ? umi_debug_workspace_select_thread(service->workspace, thread_id)
    //     : UMI_STATUS_INVALID_ARGUMENT;
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus inspectionStatus=StudioDebugInspectionReady(service);
    if(inspectionStatus!=UMI_STATUS_OK)return inspectionStatus;
    UmiStatus status = umi_debug_workspace_select_thread(service->workspace, thread_id);
    if (status != UMI_STATUS_OK || !service->nativeMode) return status;
    UmiDebugRuntimePlatformSnapshot native;
    status = umi_debug_runtime_platform_snapshot(service->native, &native);
    if (status != UMI_STATUS_OK || !native.active || !native.paused) return status;
    UmiDebugThreadSnapshot thread;
    status = umi_debug_thread_registry_find(umi_debug_service_thread(service->model), thread_id, &thread);
    if (status == UMI_STATUS_OK)
        status = umi_debug_runtime_platform_refresh_stack(service->native, thread.native_id, 1500U);
    service->nativeStatus = status;
    return status;
}

/*
 * Provide the studio debugger service select frame operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_select_frame(
    UmiStudioDebuggerService *service, const char *frame_id)
{
    /* Previous selection-only path is retained below. Native inspection uses
     * the existing Framework platform and the same model registries. */
    // return service != NULL
    //     ? umi_debug_workspace_select_frame(service->workspace, frame_id)
    //     : UMI_STATUS_INVALID_ARGUMENT;
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus inspectionStatus=StudioDebugInspectionReady(service);
    if(inspectionStatus!=UMI_STATUS_OK)return inspectionStatus;
    UmiStatus status = umi_debug_workspace_select_frame(service->workspace, frame_id);
    if (status != UMI_STATUS_OK || !service->nativeMode) return status;
    UmiDebugRuntimePlatformSnapshot native;
    status = umi_debug_runtime_platform_snapshot(service->native, &native);
    if (status != UMI_STATUS_OK || !native.active || !native.paused) return status;
    /* Frame IDs are the decimal DAP identities published by Framework. */
    char *end = NULL;
    unsigned long long frame = strtoull(frame_id, &end, 10);
    if (frame_id[0] == '\0' || end == NULL || *end != '\0' || frame > INT32_MAX)
        return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_debug_runtime_platform_refresh_scopes(service->native, (uint64_t)frame, 1500U);
    service->nativeStatus = status;
    return status;
}

/*
 * Provide the studio debugger service select scope operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_select_scope(
    UmiStudioDebuggerService *service, const char *scope_id)
{
    /* Previous selection-only path is retained below. Native inspection uses
     * the existing Framework platform and the same model registries. */
    // return service != NULL
    //     ? umi_debug_workspace_select_scope(service->workspace, scope_id)
    //     : UMI_STATUS_INVALID_ARGUMENT;
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus inspectionStatus=StudioDebugInspectionReady(service);
    if(inspectionStatus!=UMI_STATUS_OK)return inspectionStatus;
    UmiStatus status = umi_debug_workspace_select_scope(service->workspace, scope_id);
    if (status != UMI_STATUS_OK || !service->nativeMode) return status;
    UmiDebugRuntimePlatformSnapshot native;
    status = umi_debug_runtime_platform_snapshot(service->native, &native);
    if (status != UMI_STATUS_OK || !native.active || !native.paused) return status;
    UmiDebugScopeSnapshot scope;
    status = umi_debug_scope_registry_find(umi_debug_service_scope(service->model), scope_id, &scope);
    if (status == UMI_STATUS_OK && scope.variables_reference != 0U)
        status = umi_debug_runtime_platform_refresh_variables(service->native, scope.id,
            scope.variables_reference, 1500U);
    service->nativeStatus = status;
    return status;
}

/*
 * Provide the studio debugger service clear console operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_clear_console(
    UmiStudioDebuggerService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    umi_debug_workspace_clear_console(service->workspace);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio debugger service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_snapshot(
    const UmiStudioDebuggerService *service,
    UmiStudioDebuggerSnapshot *out_snapshot)
{
    UmiProtocolTransportStats stats;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    stats = umi_protocol_transport_stats(service->transport);
    out_snapshot->client_state = umi_protocol_client_state(service->client);
    out_snapshot->initialized = service->dap.initialized;
    out_snapshot->breakpoint_count = umi_debug_breakpoint_registry_count(
        umi_debug_service_breakpoint(service->model));
    out_snapshot->queued_messages = stats.queued;
    out_snapshot->sent_messages = stats.sent;
    out_snapshot->received_messages = stats.received;
    {
        UmiDebugServiceSnapshot model;
        UmiDebugControllerSnapshot controller;
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (umi_debug_service_snapshot(service->model, &model) == UMI_STATUS_OK) {
            out_snapshot->session_count = model.session_count;
            out_snapshot->thread_count = model.thread_count;
            out_snapshot->stack_frame_count = model.stack_frame_count;
            out_snapshot->variable_count = model.variable_count;
            out_snapshot->watch_count = model.watch_count;
            out_snapshot->event_count = model.event_count;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_debug_controller_snapshot(service->controller,
                                          &controller) == UMI_STATUS_OK) {
            (void)snprintf(out_snapshot->controller_state,
                           sizeof(out_snapshot->controller_state), "%s",
                           umi_debug_controller_state_text(controller.state));
        }
        (void)umi_debug_workspace_snapshot(service->workspace,
                                           &out_snapshot->workspace);
    }
    if (service->nativeMode) {
        UmiDebugRuntimePlatformSnapshot native;
        UmiStatus status = umi_debug_runtime_platform_snapshot(service->native, &native);
        if (status != UMI_STATUS_OK) return status;
        out_snapshot->initialized = native.initialized;
        out_snapshot->sent_messages = (size_t)native.adapter.messages_sent;
        out_snapshot->received_messages = (size_t)native.adapter.messages_received;
        out_snapshot->queued_messages = native.adapter.queued_events;
        const char *state = service->launchPending ? "building for debug" :
            service->nativeStatus != UMI_STATUS_OK ? "native operation failed" :
            !native.active ? "idle" : native.paused ? "paused" :
            native.adapter.state == UMI_DEBUG_RUNTIME_ADAPTER_STOPPED ? "exited" : "running";
        (void)snprintf(out_snapshot->controller_state, sizeof(out_snapshot->controller_state), "%s", state);
        out_snapshot->workspace.can_start = !native.active && !service->launchPending;
        out_snapshot->workspace.can_continue = native.active && native.paused;
        out_snapshot->workspace.can_pause = native.active && !native.paused;
        out_snapshot->workspace.can_step = native.active && native.paused;
        out_snapshot->workspace.can_stop = native.active || service->launchPending;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio debugger service workspace operation used by this module and its
 * client applications.
 */
UmiDebugWorkspace *umi_studio_debugger_service_workspace(
    UmiStudioDebuggerService *service)
{
    return service != NULL ? service->workspace : NULL;
}

/*
 * Provide the studio debugger service model operation used by this module and its client
 * applications.
 */
UmiDebugService *umi_studio_debugger_service_model(
    UmiStudioDebuggerService *service)
{
    return service != NULL ? service->model : NULL;
}

/*
 * Provide the studio debugger service controller operation used by this module and its
 * client applications.
 */
UmiDebugController *umi_studio_debugger_service_controller(
    UmiStudioDebuggerService *service)
{
    return service != NULL ? service->controller : NULL;
}

/*
 * Provide the studio debugger service transport operation used by this module and its
 * client applications.
 */
UmiProtocolTransport *umi_studio_debugger_service_transport(
    UmiStudioDebuggerService *service)
{
    return service != NULL ? service->transport : NULL;
}


/* Native hosting below is application composition only: Framework owns the
 * process, DAP framing, handshake, request correlation and inspection models. */
UmiDebugRuntimePlatform *UmiStudioDebuggerNativePlatform(UmiStudioDebuggerService *service)
{ return service != NULL ? service->native : NULL; }

int UmiStudioDebuggerNativeBusy(const UmiStudioDebuggerService *service)
{
    if (service == NULL || !service->nativeMode) return 0;
    UmiDebugRuntimePlatformSnapshot snapshot;
    return service->launchPending || (umi_debug_runtime_platform_snapshot(service->native, &snapshot) == UMI_STATUS_OK && snapshot.active);
}

/* Present the service's accepted choice instead of resetting the controls to
 * platform defaults whenever the debugger panel is reopened. */
UmiStatus UmiStudioDebuggerNativeChoice(const UmiStudioDebuggerService *service,
    char *kind, size_t kindCapacity, char *executable, size_t executableCapacity)
{
    if (service == NULL || kind == NULL || executable == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    size_t kindSize = strlen(service->nativeKind), executableSize = strlen(service->nativeExecutable);
    if (kindSize >= kindCapacity || executableSize >= executableCapacity) return UMI_STATUS_CAPACITY_EXCEEDED;
    memcpy(kind, service->nativeKind, kindSize + 1U);
    memcpy(executable, service->nativeExecutable, executableSize + 1U);
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioDebuggerConfigureNative(UmiStudioDebuggerService *service,
    const char *kind, const char *executable)
{
    if (service == NULL || kind == NULL || executable == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioDebuggerNativeBusy(service)) return UMI_STATUS_BUSY;
    if (strcmp(kind, "gdb") != 0 && strcmp(kind, "lldb") != 0) return UMI_STATUS_NOT_IMPLEMENTED;
    if (strlen(executable) >= sizeof(service->nativeExecutable)) return UMI_STATUS_CAPACITY_EXCEEDED;
    (void)snprintf(service->nativeKind, sizeof(service->nativeKind), "%s", kind);
    (void)snprintf(service->nativeExecutable, sizeof(service->nativeExecutable), "%s", executable);
    service->nativeMode = 1; service->nativeStatus = UMI_STATUS_OK;
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioDebuggerQueueNative(UmiStudioDebuggerService *service, UmiStudioBuildService *build, const UmiBuildProfile *profile)
{
    if (service == NULL || build == NULL || profile == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioDebuggerNativeBusy(service) || UmiStudioBuildBusy(build)) return UMI_STATUS_BUSY;
    UmiStatus status = umi_build_profile_validate(profile, NULL, 0U);
    if (status != UMI_STATUS_OK) return status;
    if (profile->run_program[0] == '\0') return UMI_STATUS_INVALID_STATE;
    UmiBuildProjectSessionSnapshot progress;
    status = UmiStudioBuildProgress(build, &progress);
    if (status != UMI_STATUS_OK) return status;
    if (progress.operation_id == UINT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    service->launchOperation = progress.operation_id + 1U;
    service->launchProfile = *profile;
    service->launchPending = 1; service->nativeMode = 1;
    service->nativeStatus = UMI_STATUS_OK; service->inspectedStop = 0;
    return UMI_STATUS_OK;
}

static UmiStatus NativeThreadAction(UmiStudioDebuggerService *service, int threadId,
    UmiStatus (*action)(UmiDebugRuntimePlatform *, uint64_t, uint32_t))
{
    if (threadId < 0 || action == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiDebugRuntimePlatformSnapshot snapshot;
    UmiStatus status = umi_debug_runtime_platform_snapshot(service->native, &snapshot);
    if (status != UMI_STATUS_OK) return status;
    if (!snapshot.active || service->launchPending) return UMI_STATUS_INVALID_STATE;
    uint64_t target = threadId == 0 ? snapshot.active_thread_id : (uint64_t)threadId;
    if (target == 0U) return UMI_STATUS_NOT_FOUND;
    status = action(service->native, target, 1500U);
    if (status == UMI_STATUS_OK) service->inspectedStop = 0;
    service->nativeStatus = status;
    return status;
}

UmiStatus UmiStudioDebuggerPollNative(UmiStudioDebuggerService *service, UmiStudioBuildService *build)
{
    if (service == NULL || build == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!service->nativeMode) return UMI_STATUS_OK;
    UmiStatus status = UMI_STATUS_OK;
    if (service->launchPending) {
        if (UmiStudioBuildBusy(build)) return UMI_STATUS_OK;
        service->launchPending = 0;
        UmiBuildProjectSessionSnapshot progress;
        status = UmiStudioBuildProgress(build, &progress);
        if (status != UMI_STATUS_OK || progress.operation_id != service->launchOperation ||
            progress.requested_phase != UMI_BUILD_PHASE_BUILD || progress.status != UMI_STATUS_OK ||
            progress.completed_phase_count < 2U) {
            service->nativeStatus = status != UMI_STATUS_OK ? status : UMI_STATUS_INVALID_STATE;
            return service->nativeStatus;
        }
        UmiBuildResult *result = NULL;
        status = umi_build_result_create(&result);
        if (status != UMI_STATUS_OK) return status;
        status = umi_build_history_latest(umi_studio_build_service_history(build), result);
        if (status == UMI_STATUS_OK && (result->phase != UMI_BUILD_PHASE_BUILD ||
            result->status != UMI_STATUS_OK || result->exit_code != 0 ||
            strcmp(result->profile_id, service->launchProfile.profile_id) != 0))
            status = UMI_STATUS_INVALID_STATE;
        umi_build_result_destroy(result);
        const UmiBuildProfile *current = umi_studio_build_service_profile(build);
        if (status == UMI_STATUS_OK && (current == NULL ||
            !umi_build_profile_equal(current, &service->launchProfile)))
            status = UMI_STATUS_INVALID_STATE;
        /* Shared launch descriptions keep the debugger's program, working
         * folder and argument boundaries identical to the reviewed settings.
         * The previous application-side assembly is retained for engineering
         * review, including its earlier compatibility references. */
#if 0
        char program[UMI_BUILD_PATH_CAPACITY];
        if (status == UMI_STATUS_OK) {
            if (umi_fs_is_absolute(service->launchProfile.run_program))
                (void)snprintf(program, sizeof(program), "%s", service->launchProfile.run_program);
            else status = umi_fs_join(program, sizeof(program), service->launchProfile.source_directory,
                service->launchProfile.run_program);
        }
        if (status == UMI_STATUS_OK)
        {
            /* Resolve arguments through Framework, exactly as Run does. This
             * preserves a legacy literal argument and supports reviewed lists
             * without Studio carrying its own parser. The old call is retained
             * for review because it parsed the literal field as command text. */
#if 0
            status = UmiDebugRuntimePlatformLaunchNative(service->native, service->nativeKind,
                service->nativeExecutable, program, service->launchProfile.source_directory,
                service->launchProfile.run_argument, 3000U);
#endif
            UmiArguments arguments;
            status = UmiBuildProfileArguments(&service->launchProfile, &arguments);
            /* Framework resolves the same saved working folder used by Run.
             * The previous project-root-only call remains for review; program
             * lookup is still relative to the project, not the data folder. */
#if 0
            if (status == UMI_STATUS_OK)
                status = UmiDebugRuntimePlatformLaunchArguments(service->native, service->nativeKind,
                    service->nativeExecutable, program, service->launchProfile.source_directory,
                    arguments.values, arguments.count, 3000U);
#endif
            char workingDirectory[UMI_BUILD_PATH_CAPACITY];
            if (status == UMI_STATUS_OK)
                status = UmiBuildProfileLaunchDirectory(&service->launchProfile,
                    service->launchProfile.source_directory, workingDirectory, sizeof(workingDirectory));
            if (status == UMI_STATUS_OK)
                status = UmiDebugRuntimePlatformLaunchArguments(service->native, service->nativeKind,
                    service->nativeExecutable, program, workingDirectory,
                    arguments.values, arguments.count, 3000U);
        }
#endif
        if (status == UMI_STATUS_OK) {
            UmiBuildLaunchPlan launch;
            status = UmiBuildLaunchPlanCreate(&service->launchProfile,
                service->launchProfile.source_directory, &launch);
            if (status == UMI_STATUS_OK) {
                const char *arguments[UMI_ARGUMENTS_CAPACITY];
                for (size_t i = 0U; i < launch.argument_count; ++i) arguments[i] = launch.arguments[i];
                status = UmiDebugRuntimePlatformLaunchArguments(service->native, service->nativeKind,
                    service->nativeExecutable, launch.debug_program, launch.working_directory,
                    arguments, launch.argument_count, 3000U);
            }
        }
        service->nativeStatus = status;
        if (status != UMI_STATUS_OK) return status;
    }
    UmiDebugRuntimePlatformSnapshot snapshot;
    status = umi_debug_runtime_platform_snapshot(service->native, &snapshot);
    if (status != UMI_STATUS_OK || !snapshot.active) return status;
    for (unsigned index = 0U; index < 16U; ++index) {
        int handled = 0;
        status = umi_debug_runtime_platform_pump_event(service->native, 0U, &handled);
        if (status == UMI_STATUS_NOT_FOUND) break;
        if (status != UMI_STATUS_OK) { service->nativeStatus = status; return status; }
        if (!handled) break;
    }
    status = umi_debug_runtime_platform_snapshot(service->native, &snapshot);
    if (status != UMI_STATUS_OK) return status;
    if (!snapshot.paused) service->inspectedStop = 0;
    if (snapshot.paused && !service->inspectedStop) {
        service->inspectedStop = 1; /* An error is visible, not retried every UI tick. */
        status = UmiDebugRuntimePlatformInspectStopped(service->native, 2000U);
        service->nativeStatus = status;
        (void)umi_debug_workspace_refresh(service->workspace);
    }
    return status;
}

// MIGRATION REFERENCE — previous implementation excerpts
// Native execution now delegates to Framework debug_runtime/platform.c and build/project_session.c. Studio retains command composition and presentation; the explicit legacy memory-protocol API remains available for its existing callers.
// These comments explain superseded statements; do not enable both execution paths.
// Previous source near line 102:
//     umi_debug_service_destroy(service->model);


/* Framework validates captured identity and owns properties/adapter sync.
 * Studio maintains its existing legacy registry and reports local versus
 * remote outcomes separately; it never starts an adapter from an edit. */
UmiStatus UmiStudioDebuggerEditBreakpoint(UmiStudioDebuggerService *service,
    const UmiDebugBreakpointEdit *edit, const UmiDebugBreakpointSettings *settings,
    int remove, UmiStudioBreakpointEditResult *out)
{
    if (out != NULL) memset(out, 0, sizeof *out);
    if (service == NULL || out == NULL || (remove != 0 && remove != 1)) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    UmiDebugBreakpointChange change;
    UmiStatus status = remove ? UmiDebugBreakpointEditRemove(service->workspace, edit, &change) :
        UmiDebugBreakpointEditApply(service->workspace, edit, settings, &change);
    if (status != UMI_STATUS_OK) return status;
    out->desiredApplied = 1; out->desiredChanged = change.changed;
    /* The legacy protocol store has no condition or log-message fields. Its
     * line/enabled compatibility view remains aligned; native requests read
     * the complete canonical Framework record, never this reduced view. */
    (void)umi_dap_breakpoint_remove(service->breakpoints, change.before.uri, (int)change.before.line);
    if (!remove) {
        UmiDapBreakpoint legacy = {0};
        (void)snprintf(legacy.source_path, sizeof legacy.source_path, "%s", change.after.uri);
        legacy.line = (int)change.after.line; legacy.column = (int)change.after.column;
        legacy.enabled = change.after.enabled; legacy.verified = change.after.verified;
        status = umi_dap_breakpoint_add(service->breakpoints, &legacy);
        if (status != UMI_STATUS_OK) return status;
    }
    if (service->nativeMode && UmiStudioDebuggerNativeBusy(service)) {
        status = umi_debug_runtime_platform_sync_breakpoints(service->native, change.before.uri, 1500U);
        out->adapterSynchronized = status == UMI_STATUS_OK;
    }
    return status;
}


/* Product composition keeps mutation and explicit execution separate. Shared
 * owner/generation, frame and protocol checks belong in Framework. */
UmiStatus UmiStudioDebuggerEditWatch(UmiStudioDebuggerService *service,
    const UmiDebugWatchEdit *edit, const UmiDebugWatchSettings *settings, int remove,
    UmiDebugWatchChange *out)
{
    if (out != NULL) memset(out, 0, sizeof *out);
    if (service == NULL || out == NULL || (remove != 0 && remove != 1)) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    return remove ? UmiDebugWatchEditRemove(service->workspace, edit, out) :
        UmiDebugWatchEditApply(service->workspace, edit, settings, out);
}
UmiStatus UmiStudioDebuggerEvaluateWatch(UmiStudioDebuggerService *service, const UmiDebugWatchEdit *edit)
{
    if (service == NULL || edit == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    if (!service->nativeMode) return UMI_STATUS_INVALID_STATE;
    UmiStatus status = UmiDebugRuntimeEvaluateWatchEdit(service->native, service->workspace, edit, 1500U);
    service->nativeStatus = status;
    return status;
}


/* Studio composes one explicit request. Framework owns protocol decoding,
 * stopped-frame validation and result lifetime independently from GTK. */
UmiStatus UmiStudioDebuggerInspectVariable(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, UmiDebugVariablePage **out)
{
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out = NULL;
    if (service == NULL || target == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    if (!service->nativeMode) return UMI_STATUS_INVALID_STATE;
    UmiStatus status = UmiDebugRuntimeInspectVariable(service->native, service->workspace, target, 1500U, out);
    service->nativeStatus = status;
    return status;
}

/* Studio supplies product state checks; Framework owns container identity,
 * capability negotiation, transport and capture invalidation. Keep this wrapper
 * thin so another native debugger host can use exactly the same rules. */
UmiStatus UmiStudioDebuggerCheckVariableAssignment(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target)
{
    if (service == NULL || target == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    if (!service->nativeMode) return UMI_STATUS_INVALID_STATE;
    return UmiDebugRuntimeCheckVariableAssignment(service->native, service->workspace, target);
}
UmiStatus UmiStudioDebuggerAssignVariable(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, const char *value, UmiDebugVariableAssignment *out)
{
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    memset(out, 0, sizeof *out);
    UmiStatus status = UmiStudioDebuggerCheckVariableAssignment(service, target);
    if (status != UMI_STATUS_OK) return status;
    status = UmiDebugRuntimeAssignVariable(service->native, service->workspace, target, value, 1500U, out);
    if (out->attempted) service->nativeStatus = status;
    return status;
}


/* Keep the compatibility projection coherent without giving it authority over
 * native desired settings. All allocations and capacity checks precede the
 * Framework publication. Replacing this private legacy owner cannot invalidate
 * a public borrowed handle because Studio never exposes this registry. */
UmiStatus UmiStudioDebuggerApplySetup(UmiStudioDebuggerService *service, const UmiDebugSetupReview *review)
{
    if (service == NULL || review == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiDebugRuntimePlatformSnapshot native;
    UmiStatus status = umi_debug_runtime_platform_snapshot(service->native, &native);
    if (status != UMI_STATUS_OK)
        return status;
    if (service->launchPending || native.active)
        return UMI_STATUS_BUSY;
    status = UmiDebugSetupReviewValidate(service->workspace, review);
    UmiDebugSetupSummary summary;
    const UmiDebugSetup *setup = UmiDebugSetupReviewAfter(review);
    if (status == UMI_STATUS_OK)
        status = UmiDebugSetupInspect(setup, &summary);
    UmiDapBreakpointRegistry *prepared = NULL;
    if (status == UMI_STATUS_OK)
        status = umi_dap_breakpoint_registry_create(&prepared);
    for (size_t i = 0U; status == UMI_STATUS_OK && i < summary.breakpoints; ++i)
    {
        UmiDebugSetupBreakpoint item;
        status = UmiDebugSetupBreakpointAt(setup, i, &item);
        if (status != UMI_STATUS_OK)
            break;
        UmiDapBreakpoint legacy = {0};
        size_t length = strlen(item.source);
        if (length >= sizeof legacy.source_path)
        {
            status = UMI_STATUS_CAPACITY_EXCEEDED;
            break;
        }
        memcpy(legacy.source_path, item.source, length + 1U);
        legacy.line = (int)item.line;
        legacy.column = (int)item.column;
        legacy.enabled = item.enabled;
        status = umi_dap_breakpoint_add(prepared, &legacy);
    }
    if (status == UMI_STATUS_OK)
        status = UmiDebugSetupReviewApply(service->workspace, review);
    if (status == UMI_STATUS_OK)
    {
        UmiDapBreakpointRegistry *previous = service->breakpoints;
        service->breakpoints = prepared;
        prepared = previous;
    }
    umi_dap_breakpoint_registry_destroy(prepared);
    return status;
}

/* Studio contributes product lifecycle gates. Framework keeps byte bounds,
 * response parsing and stopped-frame ownership reusable by other frontends. */
UmiStatus UmiStudioDebuggerCheckMemory(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target)
{
    if (service == NULL || target == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->launchPending) return UMI_STATUS_BUSY;
    if (!service->nativeMode) return UMI_STATUS_INVALID_STATE;
    return UmiDebugRuntimeCheckMemory(service->native, service->workspace, target);
}
UmiStatus UmiStudioDebuggerInspectMemory(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, int64_t offset, uint32_t count,
    UmiDebugMemoryCapture **out)
{
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out = NULL;
    UmiStatus status = UmiStudioDebuggerCheckMemory(service, target);
    if (status != UMI_STATUS_OK) return status;
    status = UmiDebugRuntimeInspectMemory(service->native, service->workspace, target, offset, count, 1500U, out);
    service->nativeStatus = status;
    return status;
}
