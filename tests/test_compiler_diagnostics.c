/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_diagnostics.c
 *
 * PURPOSE:
 *   Verify the test compiler diagnostics behavior for
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
#include "umicom/studio/compiler_diagnostics.h"
#include <assert.h>
#include <string.h>
int main(void) { UmiCompilerDiagnosticSet set = {0}; UmiCompilerDiagnostic diagnostic = {0}; UmiStudioCompilerDiagnosticFilter filter = {0}; UmiStudioCompilerDiagnosticSummary summary; (void)strcpy(diagnostic.file,"main.c"); (void)strcpy(diagnostic.message,"expected expression"); diagnostic.severity = UMI_COMPILER_DIAGNOSTIC_ERROR; assert(umi_compiler_diagnostic_set_add(&set,&diagnostic) == UMI_STATUS_OK); filter.minimum_severity = UMI_COMPILER_DIAGNOSTIC_WARNING; assert(umi_studio_compiler_diagnostics_summarise(&set,&filter,&summary) == UMI_STATUS_OK); assert(summary.errors == 1U); return 0; }
