/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/designer_state.h
 *
 * PURPOSE:
 *   Format the active designer document, palette, selection and history into a native Studio state report.
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

#ifndef UMICOM_STUDIO_DESIGNER_STATE_H
#define UMICOM_STUDIO_DESIGNER_STATE_H
#include "umicom/studio/designer.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio designer state report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_state_report(const UmiStudioDesigner *designer,char *out_text,size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
