/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/browser.h
 *
 * PURPOSE:
 *   Expose Studio navigation use cases over the Framework browser contract.
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

#ifndef UMICOM_STUDIO_BROWSER_H
#define UMICOM_STUDIO_BROWSER_H
#include "umicom/studio/web_platform.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio browser open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_browser_open(UmiStudioWebPlatform *platform,const char *url);
/**
 * Provide the studio browser current operation used by this module and its client
 * applications.
 */
const char *umi_studio_browser_current(const UmiStudioWebPlatform *platform);
#ifdef __cplusplus
}
#endif
#endif
