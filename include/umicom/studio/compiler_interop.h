/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_interop.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler interop contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_INTEROP_H
#define UMICOM_STUDIO_COMPILER_INTEROP_H
#include "umicom/compiler/compiler.h"
typedef struct UmiStudioCompilerInteropSummary { size_t units; size_t boundaries; size_t required_boundaries; size_t stable_boundaries; size_t incompatible_boundaries; bool buildable; char headline[UMI_COMPILER_TEXT_CAPACITY]; } UmiStudioCompilerInteropSummary;
UmiStatus umi_studio_compiler_interop_summarise(const UmiInteropGraph *graph,UmiStudioCompilerInteropSummary *out_summary);
#endif
