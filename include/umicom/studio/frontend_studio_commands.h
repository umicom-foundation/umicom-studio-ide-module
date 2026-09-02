/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_COMMANDS_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_COMMANDS_H
#include "umicom/studio/frontend_studio_centre.h"
#define UMI_STUDIO_FRONTEND_COMMAND_COUNT 9U
/**
 * Represent the studio frontend command data shared with callers of this public contract.
 */
typedef struct UmiStudioFrontendCommand { const char *id; const char *title; int mutates_state; } UmiStudioFrontendCommand;
/**
 * Return the number of records represented by studio frontend commands without changing
 * their state.
 */
size_t umi_studio_frontend_commands_count(void);
/**
 * Find studio frontend command while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioFrontendCommand *umi_studio_frontend_command_at(size_t index);
/**
 * Perform studio frontend command through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_frontend_command_execute(UmiStudioFrontendCentre *centre,const char *command_id,const char *argument,char *message,size_t message_capacity);
#endif
