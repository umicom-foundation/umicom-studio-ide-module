/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/processes.h
 *
 * PURPOSE:
 *   Submit, cancel, wait for and inspect supervised Studio child processes.
 *   Execution is restricted to trusted workspaces and defaults to the active
 *   workspace root when no working directory is supplied.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PROCESSES_H
#define UMICOM_STUDIO_PROCESSES_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio process submit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_submit(UmiStudioServices *services,
                                    const char *label,
                                    const UmiProcessRequest *request,
                                    UmiProcessJobId *out_job_id);
/**
 * Provide the studio process cancel operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_cancel(UmiStudioServices *services,
                                    UmiProcessJobId job_id);
/**
 * Provide the studio process wait operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_wait(UmiStudioServices *services,
                                  UmiProcessJobId job_id,
                                  uint32_t timeout_ms);
/**
 * Provide the studio process snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_snapshot(
    UmiStudioServices *services,
    UmiProcessJobId job_id,
    UmiProcessJobSnapshot *out_snapshot
);
/**
 * Provide the studio process stats operation used by this module and its client
 * applications.
 */
UmiProcessSupervisorStats umi_studio_process_stats(
    UmiStudioServices *services
);

#ifdef __cplusplus
}
#endif

#endif
