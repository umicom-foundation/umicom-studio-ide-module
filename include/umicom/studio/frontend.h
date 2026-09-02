/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/frontend.h
 *
 * PURPOSE:
 *   Expose Studio multi-frontend planning and browser-artifact generation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This is a thin Studio-facing API. Reusable HTTP, browser, chart and frontend logic remains in Umicom Framework.
 */

#ifndef UMICOM_STUDIO_FRONTEND_H
#define UMICOM_STUDIO_FRONTEND_H
#include "umicom/studio/web_platform.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Release or reset state held by studio frontend so the same storage can be reused safely.
 */
UmiStatus umi_studio_frontend_reset(UmiStudioWebPlatform *platform,UmiFrontendKind kind);
/**
 * Provide the studio frontend generate html operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_generate_html(UmiStudioWebPlatform *platform,char *out_text,size_t capacity);
/**
 * Provide the studio frontend generate manifest operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_generate_manifest(UmiStudioWebPlatform *platform,char *out_text,size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
