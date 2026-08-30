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
typedef struct UmiStudioCompilerCommand { const char *command_id; const char *title; const char *category; const char *default_key; bool requires_project; bool requires_provider; bool destructive; } UmiStudioCompilerCommand;
size_t umi_studio_compiler_commands_count(void);
const UmiStudioCompilerCommand *umi_studio_compiler_commands_at(size_t index);
const UmiStudioCompilerCommand *umi_studio_compiler_commands_find(const char *command_id);
#endif
