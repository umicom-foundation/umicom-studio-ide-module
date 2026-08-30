/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_templates.c
 *
 * PURPOSE:
 *   Verify the test compiler templates behavior for
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
#include "umicom/studio/compiler_templates.h"
#include <assert.h>
int main(void) { const UmiStudioCompilerTemplate *poly = umi_studio_compiler_templates_find("polyglot-console"); assert(umi_studio_compiler_templates_count() >= 7U); assert(poly != NULL); assert(poly->polyglot); assert(poly->requires_umicc); return 0; }
