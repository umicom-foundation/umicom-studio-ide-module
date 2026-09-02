/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_COMMANDS_H
#define UMICOM_STUDIO_COMPILER_COMMANDS_H
#include <stddef.h>
#include <stdbool.h>
/**
 * Represent the studio compiler command data shared with callers of this public contract.
 */
typedef struct UmiStudioCompilerCommand { const char *command_id; const char *title; const char *category; const char *default_key; bool requires_project; bool requires_provider; bool destructive; } UmiStudioCompilerCommand;
/**
 * Return the number of records represented by studio compiler commands without changing
 * their state.
 */
size_t umi_studio_compiler_commands_count(void);
/**
 * Find studio compiler commands while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioCompilerCommand *umi_studio_compiler_commands_at(size_t index);
/**
 * Find studio compiler commands while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioCompilerCommand *umi_studio_compiler_commands_find(const char *command_id);
#endif
