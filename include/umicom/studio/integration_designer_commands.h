/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_COMMANDS_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_COMMANDS_H
#include "umicom/studio/integration_designer_execution.h"
/**
 * List the named studio integration designer command values accepted by this public
 * contract.
 */
typedef enum UmiStudioIntegrationDesignerCommand {
    UMI_STUDIO_INTEGRATION_COMMAND_SEED_SAMPLE = 1,
    UMI_STUDIO_INTEGRATION_COMMAND_VALIDATE,
    UMI_STUDIO_INTEGRATION_COMMAND_SIMULATE
} UmiStudioIntegrationDesignerCommand;
/**
 * Provide the studio integration designer seed sample operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_seed_sample(UmiStudioIntegrationDesignerCentre *centre);
/**
 * Perform studio integration designer through the module contract so client applications
 * do not duplicate its policy.
 */
UmiStatus umi_studio_integration_designer_execute(UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerCommand command,UmiIntegrationSimulation *out_simulation);
#endif
