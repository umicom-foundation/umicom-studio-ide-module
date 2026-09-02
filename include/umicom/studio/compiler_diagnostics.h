/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_diagnostics.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler diagnostics contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_DIAGNOSTICS_H
#define UMICOM_STUDIO_COMPILER_DIAGNOSTICS_H
#include "umicom/compiler/compiler.h"
/**
 * Represent the studio compiler diagnostic filter data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioCompilerDiagnosticFilter { UmiCompilerDiagnosticSeverity minimum_severity; char file_contains[128U]; char code_contains[64U]; } UmiStudioCompilerDiagnosticFilter;
/**
 * Represent the studio compiler diagnostic summary data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioCompilerDiagnosticSummary { size_t visible; size_t notes; size_t warnings; size_t errors; char headline[UMI_COMPILER_TEXT_CAPACITY]; } UmiStudioCompilerDiagnosticSummary;
/**
 * Provide the studio compiler diagnostic matches operation used by this module and its
 * client applications.
 */
bool umi_studio_compiler_diagnostic_matches(const UmiCompilerDiagnostic *diagnostic,const UmiStudioCompilerDiagnosticFilter *filter);
/**
 * Provide the studio compiler diagnostics summarise operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_diagnostics_summarise(const UmiCompilerDiagnosticSet *set,const UmiStudioCompilerDiagnosticFilter *filter,UmiStudioCompilerDiagnosticSummary *out_summary);
#endif
