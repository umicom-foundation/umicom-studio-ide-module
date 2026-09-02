/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/designer_preview.h
 *
 * PURPOSE:
 *   Create preview and generated-source reports from the active Studio designer document.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio-facing contract composes reusable Framework capabilities.
 * Generic designer and declarative logic remains owned by Umicom Framework.
 */

#ifndef UMICOM_STUDIO_DESIGNER_PREVIEW_H
#define UMICOM_STUDIO_DESIGNER_PREVIEW_H
#include "umicom/studio/designer.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio designer generate preview operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_designer_generate_preview(UmiStudioDesigner *designer,char *out_text,size_t capacity,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio designer generate c operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_generate_c(UmiStudioDesigner *designer,UmiStudioDeclarative *declarative,char *out_text,size_t capacity,UmiDeclDiagnosticList *diagnostics);
#ifdef __cplusplus
}
#endif
#endif
