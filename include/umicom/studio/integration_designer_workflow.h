/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_workflow.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer workflow contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_WORKFLOW_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_WORKFLOW_H
#include "umicom/studio/integration_designer_centre.h"
UmiStatus umi_studio_integration_designer_add_workflow(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationWorkflow *workflow);
UmiStatus umi_studio_integration_designer_validate_workflow(const UmiStudioIntegrationDesignerCentre *centre,const char *workflow_id,UmiIntegrationDesignerValidation *validation);
#endif
