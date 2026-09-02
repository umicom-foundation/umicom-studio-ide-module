/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/processes.c
 *
 * PURPOSE:
 *   Enforce Studio workspace trust and delegate asynchronous child-process
 *   lifecycle to the shared Framework process supervisor.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/processes.h"

/*
 * Provide the studio process submit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_submit(UmiStudioServices *services,
                                    const char *label,
                                    const UmiProcessRequest *request,
                                    UmiProcessJobId *out_job_id)
{
    UmiWorkspaceGraphSnapshot workspace;
    UmiProcessRequest effective;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || request == NULL || out_job_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_workspace_graph_snapshot(
        umi_studio_services_workspace(services), &workspace);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK || !workspace.open) {
        return status != UMI_STATUS_OK ? status : UMI_STATUS_INVALID_STATE;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (!workspace.trusted) return UMI_STATUS_PERMISSION_DENIED;
    effective = *request;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (effective.working_directory == NULL ||
        effective.working_directory[0] == '\0') {
        effective.working_directory = workspace.root;
    }
    return umi_process_supervisor_submit(
        umi_studio_services_process_supervisor(services),
        label,
        &effective,
        out_job_id);
}

/*
 * Provide the studio process cancel operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_cancel(UmiStudioServices *services,
                                    UmiProcessJobId job_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_process_supervisor_cancel(
        umi_studio_services_process_supervisor(services), job_id);
}

/*
 * Provide the studio process wait operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_wait(UmiStudioServices *services,
                                  UmiProcessJobId job_id,
                                  uint32_t timeout_ms)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_process_supervisor_wait(
        umi_studio_services_process_supervisor(services),
        job_id,
        timeout_ms);
}

/*
 * Provide the studio process snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_process_snapshot(
    UmiStudioServices *services,
    UmiProcessJobId job_id,
    UmiProcessJobSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_process_supervisor_snapshot(
        umi_studio_services_process_supervisor(services),
        job_id,
        out_snapshot);
}

/*
 * Provide the studio process stats operation used by this module and its client
 * applications.
 */
UmiProcessSupervisorStats umi_studio_process_stats(
    UmiStudioServices *services)
{
    return services != NULL
        ? umi_process_supervisor_stats(
              umi_studio_services_process_supervisor(services))
        : (UmiProcessSupervisorStats){0};
}
