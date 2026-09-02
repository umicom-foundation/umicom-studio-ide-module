/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_api.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer api contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_API_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_API_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * Provide the studio integration designer add contract operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_contract(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationApiContract *contract);
/**
 * Provide the studio integration designer find contract operation used by this module and
 * its client applications.
 */
const UmiIntegrationApiContract *umi_studio_integration_designer_find_contract(const UmiStudioIntegrationDesignerCentre *centre,const char *contract_id);
/**
 * Provide the studio integration designer plan request operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_plan_request(const UmiStudioIntegrationDesignerCentre *centre,const char *contract_id,const char *operation_id,const char *payload,const char *correlation_id,UmiIntegrationRequestPlan *out_plan);
#endif
