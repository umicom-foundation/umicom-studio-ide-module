/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_api.c
 *
 * PURPOSE:
 *   Implement the integration designer api behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer API explorer | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_api.h"
#include <string.h>

/*
 * Provide the studio integration designer add contract operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_contract(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationApiContract *contract)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (contract == NULL || umi_integration_connection_catalogue_find(&centre->fabric.connections,contract->profile_id) == NULL) return UMI_STATUS_NOT_FOUND;
    status = umi_integration_designer_fabric_add_contract(&centre->fabric,contract);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

/*
 * Provide the studio integration designer find contract operation used by this module and
 * its client applications.
 */
const UmiIntegrationApiContract *umi_studio_integration_designer_find_contract(const UmiStudioIntegrationDesignerCentre *centre,const char *contract_id)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || contract_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < centre->fabric.contract_count; ++index) /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (strcmp(centre->fabric.contracts[index].id,contract_id) == 0) return &centre->fabric.contracts[index];
    return NULL;
}

/*
 * Provide the studio integration designer plan request operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_plan_request(const UmiStudioIntegrationDesignerCentre *centre,const char *contract_id,const char *operation_id,const char *payload,const char *correlation_id,UmiIntegrationRequestPlan *out_plan)
{
    const UmiIntegrationApiContract *contract = umi_studio_integration_designer_find_contract(centre,contract_id);
    const UmiIntegrationApiOperation *operation;
    const UmiIntegrationConnectionProfile *profile;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (contract == NULL) return UMI_STATUS_NOT_FOUND;
    operation = umi_integration_api_contract_find_operation(contract,operation_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operation == NULL) return UMI_STATUS_NOT_FOUND;
    profile = umi_integration_connection_catalogue_find(&centre->fabric.connections,contract->profile_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (profile == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_integration_request_plan_build(profile,operation,payload,correlation_id,out_plan);
}
