/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public security commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_COMMANDS_H
#define UMICOM_STUDIO_SECURITY_COMMANDS_H
#include "umicom/studio/security_roles.h"
typedef enum UmiStudioSecurityCommand { UMI_STUDIO_SECURITY_COMMAND_SEED = 1, UMI_STUDIO_SECURITY_COMMAND_AUTHORISE_BUILD, UMI_STUDIO_SECURITY_COMMAND_AUTHORISE_PLUGIN } UmiStudioSecurityCommand;
UmiStatus umi_studio_security_seed(UmiStudioSecurityCentre *centre);
UmiStatus umi_studio_security_execute(UmiStudioSecurityCentre *centre,UmiStudioSecurityCommand command,UmiSecurityGovernanceDecision *out_decision);
#endif
