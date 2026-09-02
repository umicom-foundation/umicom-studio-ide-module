/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/web.h
 *
 * PURPOSE:
 *   Expose Studio HTTP processing and default REST endpoint registration.
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

#ifndef UMICOM_STUDIO_WEB_H
#define UMICOM_STUDIO_WEB_H
#include "umicom/studio/web_platform.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio web process operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_web_process(UmiStudioWebPlatform *platform,const char *request,char *out_response,size_t capacity);
/**
 * Provide the studio web register defaults operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_web_register_defaults(UmiStudioWebPlatform *platform);
#ifdef __cplusplus
}
#endif
#endif
