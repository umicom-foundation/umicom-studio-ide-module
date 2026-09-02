/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_COMMANDS_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_COMMANDS_H
#include "umicom/studio/database_explorer_queries.h"
#define UMI_STUDIO_DATABASE_COMMAND_COUNT 10U
/**
 * Represent the studio database command data shared with callers of this public contract.
 */
typedef struct UmiStudioDatabaseCommand { const char *id; const char *title; int mutates_state; } UmiStudioDatabaseCommand;
/**
 * Return the number of records represented by studio database commands without changing
 * their state.
 */
size_t umi_studio_database_commands_count(void);
/**
 * Find studio database command while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioDatabaseCommand *umi_studio_database_command_at(size_t index);
/**
 * Perform studio database command through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_database_command_execute(UmiStudioDatabaseExplorerCentre *centre,const char *command_id,const char *argument,char *message,size_t message_capacity);
#endif
