/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_workflow.c
 *
 * PURPOSE:
 *   Implement the integration designer workflow behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer workflow canvas | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_workflow.h"

/*
 * Provide the studio integration designer add workflow operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_workflow(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationWorkflow *workflow)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_integration_designer_fabric_add_workflow(&centre->fabric,workflow);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

/*
 * Provide the studio integration designer validate workflow operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_integration_designer_validate_workflow(const UmiStudioIntegrationDesignerCentre *centre,const char *workflow_id,UmiIntegrationDesignerValidation *validation)
{
    const UmiIntegrationWorkflow *workflow;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || validation == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    workflow = umi_integration_designer_fabric_find_workflow(&centre->fabric,workflow_id);
    return workflow == NULL ? UMI_STATUS_NOT_FOUND : umi_integration_workflow_validate(workflow,validation);
}
