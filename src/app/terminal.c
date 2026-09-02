/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/terminal.c
 *
 * PURPOSE:
 *   Implement Studio terminal-session ownership and command execution.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/terminal.h"

#include <stdlib.h>
#include <string.h>

struct UmiStudioTerminalService {
    UmiTerminalManager *manager;
    UmiTerminalSession *primary;
};

/*
 * Initialise studio terminal service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_terminal_service_create(
    const char *working_directory,
    UmiClock *clock,
    UmiStudioTerminalService **out_service)
{
    UmiStudioTerminalService *service;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (working_directory == NULL || clock == NULL || out_service == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_service = NULL;
    service = (UmiStudioTerminalService *)calloc(1U, sizeof(*service));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_terminal_manager_create(clock, &service->manager);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_terminal_manager_open(service->manager,
                                           "studio.primary",
                                           "Umicom Studio",
                                           working_directory,
                                           &service->primary);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_terminal_service_destroy(service);
        return status;
    }
    *out_service = service;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio terminal service so the same storage can be reused
 * safely.
 */
void umi_studio_terminal_service_destroy(UmiStudioTerminalService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_terminal_manager_destroy(service->manager);
    free(service);
}

/*
 * Perform studio terminal service through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_terminal_service_execute(
    UmiStudioTerminalService *service,
    const char *command,
    uint32_t timeout_ms,
    UmiCancellationToken *cancellation,
    int *out_exit_code)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || command == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_terminal_session_execute(service->primary,
                                        command,
                                        timeout_ms,
                                        cancellation,
                                        out_exit_code);
}

/*
 * Provide the studio terminal service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_terminal_service_snapshot(
    const UmiStudioTerminalService *service,
    UmiStudioTerminalSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->session_count = umi_terminal_manager_count(service->manager);
    return umi_terminal_session_snapshot(service->primary,
                                         &out_snapshot->primary);
}

/*
 * Provide the studio terminal service manager operation used by this module and its client
 * applications.
 */
UmiTerminalManager *umi_studio_terminal_service_manager(
    UmiStudioTerminalService *service)
{
    return service != NULL ? service->manager : NULL;
}

/*
 * Provide the studio terminal service primary operation used by this module and its client
 * applications.
 */
UmiTerminalSession *umi_studio_terminal_service_primary(
    UmiStudioTerminalService *service)
{
    return service != NULL ? service->primary : NULL;
}
