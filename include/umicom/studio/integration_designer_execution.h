/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_execution.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer execution contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_EXECUTION_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_EXECUTION_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * Provide the studio integration designer set policy operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_set_policy(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationExecutionPolicy *policy);
/**
 * Provide the studio integration designer simulate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_simulate(UmiStudioIntegrationDesignerCentre *centre,const char *workflow_id,UmiIntegrationSimulation *out_simulation);
#endif
