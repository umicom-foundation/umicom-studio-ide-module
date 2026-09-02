/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_views_commands.c
 *
 * PURPOSE:
 *   Verify the test compiler views commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio Batch 58 test | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_commands.h"
#include <string.h>
#include "umicom/studio/compiler_views.h"
#include <assert.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) { const UmiStudioCompilerCommand *profile; assert(umi_studio_compiler_views_count() >= 8U); assert(umi_studio_compiler_views_find("compiler.interop") != NULL); assert(umi_studio_compiler_commands_count() >= 10U); assert(umi_studio_compiler_commands_find("compiler.build") != NULL); profile = umi_studio_compiler_commands_find("compiler.profile.select"); assert(profile != NULL); assert(strcmp(profile->default_key, "Ctrl+Alt+Shift+P") == 0); return 0; }
