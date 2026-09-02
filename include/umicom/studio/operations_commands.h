/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/operations_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public operations commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_OPERATIONS_COMMANDS_H
#define UMICOM_STUDIO_OPERATIONS_COMMANDS_H
#include "umicom/studio/operations_centre.h"
/**
 * List the named studio operations command values accepted by this public contract.
 */
typedef enum UmiStudioOperationsCommand {
    UMI_STUDIO_OPERATIONS_COMMAND_SEED = 1,
    UMI_STUDIO_OPERATIONS_COMMAND_MARK_CRASH_RECOVERED,
    UMI_STUDIO_OPERATIONS_COMMAND_OPEN_PROFILER
} UmiStudioOperationsCommand;
/**
 * Provide the studio operations seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_operations_seed(UmiStudioOperationsCentre *centre);
/**
 * Perform studio operations through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_operations_execute(UmiStudioOperationsCentre *centre,UmiStudioOperationsCommand command,const char *argument);
#endif
