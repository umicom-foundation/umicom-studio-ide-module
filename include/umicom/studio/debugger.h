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
#include "umicom/studio/build.h"
#include "umicom/debug_runtime/platform.h"
#include "umicom/debug_runtime/native_attach.h"
#include "umicom/debug/breakpoint_edit.h"
#include "umicom/debug_runtime/watch_evaluation.h"
#include "umicom/debug_runtime/variable_inspection.h"
#include "umicom/debug_runtime/memory_inspection.h"
#include "umicom/debug_runtime/variable_assignment.h"

#include "umicom/debug/setup.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio debugger service data shared with callers of this public contract.
 */
typedef struct UmiStudioDebuggerService UmiStudioDebuggerService;

/** Attach the selected native adapter to an explicit external process ID.
 * The host supplies current workspace trust and an idle build service; this
 * call does not save drafts or submit a build. Program is an optional absolute
 * symbols file. The current project root and tool folder are copied into the
 * shared attach plan; launch arguments and launch environment are not applied
 * to an existing process. Operations remain on the debugger owner thread.
 * A failed or uncertain attachment is reported without automatic retry. */
UmiStatus UmiStudioDebuggerAttachNative(UmiStudioDebuggerService *service,
    UmiStudioBuildService *build, uint64_t process_id, const char *program, int trusted);
/** Observe whether this captured row can be assigned in the current native
 * stop. No adapter request is sent; queued launches and legacy sessions refuse. */
UmiStatus UmiStudioDebuggerCheckVariableAssignment(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target);
/** Apply an explicit runtime value through Framework. No automatic retry or
 * inspection follows. After attempted=1, use Inspect scope to obtain fresh
 * captures; a missing reply does not prove that the program stayed unchanged. */
UmiStatus UmiStudioDebuggerAssignVariable(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, const char *value, UmiDebugVariableAssignment *out);


/** Desired state and adapter confirmation are distinct. A failed sync may
 * follow a successful local edit; never automatically undo or retry it. */
typedef struct UmiStudioBreakpointEditResult {
    int desiredApplied;
    int desiredChanged;
    int adapterSynchronized;
} UmiStudioBreakpointEditResult;
/** Apply or remove a captured row using Framework validation and synchronization.
 * remove must be 0/1; settings is required only when remove is zero.
 * A queued launch returns BUSY before editing. Idle/memory sessions retain
 * desired properties for later synchronization. No process is launched here. */
UmiStatus UmiStudioDebuggerEditBreakpoint(UmiStudioDebuggerService *service,
    const UmiDebugBreakpointEdit *edit, const UmiDebugBreakpointSettings *settings,
    int remove, UmiStudioBreakpointEditResult *out);

/** Edit/remove a retained watch without executing its expression. A queued
 * launch is BUSY. Evaluation is a separate explicit action in a stopped native
 * session and uses the current inspected frame. Neither API launches a process. */
UmiStatus UmiStudioDebuggerEditWatch(UmiStudioDebuggerService *service,
    const UmiDebugWatchEdit *edit, const UmiDebugWatchSettings *settings, int remove,
    UmiDebugWatchChange *out);
UmiStatus UmiStudioDebuggerEvaluateWatch(UmiStudioDebuggerService *service,
    const UmiDebugWatchEdit *edit);

/** Capture children through Framework's current-stop and owned-reference checks.
 * Idle/memory sessions do not launch an adapter. A queued launch returns BUSY.
 * On failure *out is NULL; previous caller-owned pages remain unchanged. */
UmiStatus UmiStudioDebuggerInspectVariable(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, UmiDebugVariablePage **out);

/** Read one current paused variable's memory through the shared Framework.
 * No adapter is launched for idle/simulated sessions. A queued launch is BUSY.
 * The caller owns the independent capture; *out is NULL on failure. */
UmiStatus UmiStudioDebuggerInspectMemory(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target, int64_t offset, uint32_t count,
    UmiDebugMemoryCapture **out);
UmiStatus UmiStudioDebuggerCheckMemory(UmiStudioDebuggerService *service,
    const UmiDebugVariableTarget *target);

/* Copy the currently selected adapter for a newly opened settings surface.
 * The two output buffers must not overlap. Output changes only on success. This does not read disk or start an adapter. */
UmiStatus UmiStudioDebuggerNativeChoice(const UmiStudioDebuggerService *service,
    char *kind, size_t kindCapacity, char *executable, size_t executableCapacity);

/** Select an installed native adapter. kind is "lldb" or "gdb"; executable
 * is empty for PATH discovery or a complete executable path. The values are
 * copied; a busy session is not reconfigured. This is a per-window setting. */
UmiStatus UmiStudioDebuggerConfigureNative(UmiStudioDebuggerService *service,
    const char *kind, const char *executable);
/** Capture the reviewed launch profile and the next build operation identity.
 * Submit the existing Build worker immediately after OK; cancel this request if
 * submission fails. PollNative requires that exact operation to finish before launching.
 * The host must retain trust and the same workspace, and poll on its owner
 * thread. No native process starts while a compiler job is still running. */
UmiStatus UmiStudioDebuggerQueueNative(UmiStudioDebuggerService *service,
    UmiStudioBuildService *build, const UmiBuildProfile *profile);
/** Poll bounded DAP input, and finish a queued build-before-debug request.
 * Build failures never launch the previous executable. A stopped session uses
 * Framework's existing inspection registries. Native request waits are bounded
 * but synchronous: the current host may pause briefly while an adapter replies. */
UmiStatus UmiStudioDebuggerPollNative(UmiStudioDebuggerService *service,
    UmiStudioBuildService *build);
/** Pending compiler-to-debug transition or a connected native process. */
int UmiStudioDebuggerNativeBusy(const UmiStudioDebuggerService *service);
/** Borrow the real Framework runtime for explicit inspection and tests. */
UmiDebugRuntimePlatform *UmiStudioDebuggerNativePlatform(UmiStudioDebuggerService *service);


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
/* restart != 0 requests the active adapter's advertised restart operation.
 * It reuses current launch settings and performs no build. Pending builds return
 * BUSY; unsupported adapters retain explicit Stop then Debug as the alternative.
 * Uncertain replies retire old inspection data and require an explicit Stop. */
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


/* Framework owns the saved settings and stale-review rules. Studio also checks
 * its native process/build launch and prepares the legacy protocol projection
 * before any settings change. No adapter request or watch evaluation is sent. */
UmiStatus UmiStudioDebuggerApplySetup(UmiStudioDebuggerService *service, const UmiDebugSetupReview *review);

#ifdef __cplusplus
}
#endif

#endif
