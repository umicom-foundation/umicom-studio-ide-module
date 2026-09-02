/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/debugger.h
 *
 * PURPOSE:
 *   Provide a toolkit-neutral Studio debugger service over Debug Adapter Protocol contracts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_DEBUGGER_H
#define UMICOM_STUDIO_DEBUGGER_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio debugger service data shared with callers of this public contract.
 */
typedef struct UmiStudioDebuggerService UmiStudioDebuggerService;

/**
 * Represent the studio debugger snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioDebuggerSnapshot {
    UmiProtocolClientState client_state;
    int initialized;
    size_t breakpoint_count;
    size_t queued_messages;
    size_t sent_messages;
    size_t received_messages;
    size_t session_count;
    size_t thread_count;
    size_t stack_frame_count;
    size_t variable_count;
    size_t watch_count;
    size_t event_count;
    char controller_state[64];
    UmiDebugWorkspaceSnapshot workspace;
} UmiStudioDebuggerSnapshot;

/**
 * Initialise studio debugger service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debugger_service_create(
    UmiStudioDebuggerService **out_service
);
/**
 * Release or reset state held by studio debugger service so the same storage can be reused
 * safely.
 */
void umi_studio_debugger_service_destroy(UmiStudioDebuggerService *service);
/**
 * Initialise studio debugger service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debugger_service_initialize(
    UmiStudioDebuggerService *service,
    const char *adapter_id,
    int64_t *out_request_id
);
/**
 * Provide the studio debugger service launch operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_launch(
    UmiStudioDebuggerService *service,
    const char *program,
    const char *working_directory,
    int64_t *out_request_id
);
/**
 * Provide the studio debugger service start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_start(
    UmiStudioDebuggerService *service, const char *adapter_id,
    const char *program, const char *working_directory
);
/**
 * Provide the studio debugger service continue operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_continue(
    UmiStudioDebuggerService *service, int thread_id
);
/**
 * Provide the studio debugger service pause operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_pause(
    UmiStudioDebuggerService *service, int thread_id
);
/**
 * Provide the studio debugger service next operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_next(
    UmiStudioDebuggerService *service, int thread_id
);
/**
 * Provide the studio debugger service step in operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_step_in(
    UmiStudioDebuggerService *service, int thread_id
);
/**
 * Provide the studio debugger service step out operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_step_out(
    UmiStudioDebuggerService *service, int thread_id
);
/**
 * Provide the studio debugger service stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debugger_service_stop(
    UmiStudioDebuggerService *service, int restart
);
/**
 * Provide the studio debugger service add breakpoint operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_add_breakpoint(
    UmiStudioDebuggerService *service,
    const char *source_path,
    int line,
    int column
);
/**
 * Provide the studio debugger service set breakpoint enabled operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_debugger_service_set_breakpoint_enabled(
    UmiStudioDebuggerService *service,
    const char *breakpoint_id,
    int enabled
);
/**
 * Provide the studio debugger service remove breakpoint operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_debugger_service_remove_breakpoint(
    UmiStudioDebuggerService *service,
    const char *breakpoint_id
);
/**
 * Provide the studio debugger service add watch operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_add_watch(
    UmiStudioDebuggerService *service,
    const char *expression,
    char *out_watch_id,
    size_t out_watch_id_capacity
);
/**
 * Provide the studio debugger service remove watch operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_remove_watch(
    UmiStudioDebuggerService *service,
    const char *watch_id
);
/**
 * Provide the studio debugger service select thread operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_select_thread(
    UmiStudioDebuggerService *service,
    const char *thread_id
);
/**
 * Provide the studio debugger service select frame operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_select_frame(
    UmiStudioDebuggerService *service,
    const char *frame_id
);
/**
 * Provide the studio debugger service select scope operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_select_scope(
    UmiStudioDebuggerService *service,
    const char *scope_id
);
/**
 * Provide the studio debugger service clear console operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_clear_console(
    UmiStudioDebuggerService *service
);
/**
 * Provide the studio debugger service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_debugger_service_snapshot(
    const UmiStudioDebuggerService *service,
    UmiStudioDebuggerSnapshot *out_snapshot
);
/**
 * Provide the studio debugger service transport operation used by this module and its
 * client applications.
 */
UmiProtocolTransport *umi_studio_debugger_service_transport(
    UmiStudioDebuggerService *service
);
/**
 * Provide the studio debugger service model operation used by this module and its client
 * applications.
 */
UmiDebugService *umi_studio_debugger_service_model(
    UmiStudioDebuggerService *service
);
/**
 * Provide the studio debugger service controller operation used by this module and its
 * client applications.
 */
UmiDebugController *umi_studio_debugger_service_controller(
    UmiStudioDebuggerService *service
);
/**
 * Provide the studio debugger service workspace operation used by this module and its
 * client applications.
 */
UmiDebugWorkspace *umi_studio_debugger_service_workspace(
    UmiStudioDebuggerService *service
);

#ifdef __cplusplus
}
#endif

#endif
