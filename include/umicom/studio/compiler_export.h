/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_export.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler export contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_EXPORT_H
#define UMICOM_STUDIO_COMPILER_EXPORT_H
#include "umicom/compiler/compiler.h"
UmiStatus umi_studio_compiler_export_summary(const UmiCompilerCentreSnapshot *snapshot,char *out_text,size_t capacity);
UmiStatus umi_studio_compiler_export_database(const UmiCompilerCentre *centre,char *out_json,size_t capacity);
#endif
