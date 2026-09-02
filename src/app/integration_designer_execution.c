/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_execution.c
 *
 * PURPOSE:
 *   Implement the integration designer execution behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer execution | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_execution.h"
#include <inttypes.h>
#include <stdio.h>

/*
 * Provide the studio integration designer set policy operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_set_policy(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationExecutionPolicy *policy)
{
    UmiIntegrationDesignerValidation validation = {0};
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || policy == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_integration_execution_policy_validate(policy,&validation) != UMI_STATUS_OK) return UMI_STATUS_INVALID_STATE;
    centre->policy = *policy;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio integration designer simulate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_simulate(UmiStudioIntegrationDesignerCentre *centre,const char *workflow_id,UmiIntegrationSimulation *out_simulation)
{
    const UmiIntegrationWorkflow *workflow;
    char run_id[UMI_INTEGRATION_DESIGNER_ID_CAPACITY];
    int length;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || workflow_id == NULL || out_simulation == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    workflow = umi_integration_designer_fabric_find_workflow(&centre->fabric,workflow_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workflow == NULL) return UMI_STATUS_NOT_FOUND;
    centre->next_run_sequence += 1U;
    length = snprintf(run_id,sizeof(run_id),"studio-run-%" PRIu64,centre->next_run_sequence);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length < 0 || (size_t)length >= sizeof(run_id)) return UMI_STATUS_CAPACITY_EXCEEDED;
    status = umi_integration_simulate(workflow,&centre->policy,run_id,out_simulation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_integration_designer_fabric_record_run(&centre->fabric,out_simulation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}
