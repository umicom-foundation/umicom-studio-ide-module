/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/terminal.h
 *
 * PURPOSE:
 *   Expose prepared terminal sessions and captured transcripts to Studio frontends.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_TERMINAL_H
#define UMICOM_STUDIO_TERMINAL_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio terminal service data shared with callers of this public contract.
 */
typedef struct UmiStudioTerminalService UmiStudioTerminalService;

/**
 * Represent the studio terminal snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioTerminalSnapshot {
    size_t session_count;
    UmiTerminalSessionSnapshot primary;
} UmiStudioTerminalSnapshot;

/**
 * Initialise studio terminal service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_terminal_service_create(
    const char *working_directory,
    UmiClock *clock,
    UmiStudioTerminalService **out_service
);
/**
 * Release or reset state held by studio terminal service so the same storage can be reused
 * safely.
 */
void umi_studio_terminal_service_destroy(UmiStudioTerminalService *service);
/**
 * Perform studio terminal service through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_terminal_service_execute(
    UmiStudioTerminalService *service,
    const char *command,
    uint32_t timeout_ms,
    UmiCancellationToken *cancellation,
    int *out_exit_code
);
/**
 * Provide the studio terminal service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_terminal_service_snapshot(
    const UmiStudioTerminalService *service,
    UmiStudioTerminalSnapshot *out_snapshot
);
/**
 * Provide the studio terminal service manager operation used by this module and its client
 * applications.
 */
UmiTerminalManager *umi_studio_terminal_service_manager(
    UmiStudioTerminalService *service
);
/**
 * Provide the studio terminal service primary operation used by this module and its client
 * applications.
 */
UmiTerminalSession *umi_studio_terminal_service_primary(
    UmiStudioTerminalService *service
);

#ifdef __cplusplus
}
#endif

#endif
