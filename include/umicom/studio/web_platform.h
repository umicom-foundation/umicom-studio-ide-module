/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/web_platform.h
 *
 * PURPOSE:
 *   Expose Studio's shared composition of Framework web, browser, charting and frontend services.
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

#ifndef UMICOM_STUDIO_WEB_PLATFORM_H
#define UMICOM_STUDIO_WEB_PLATFORM_H
#include "umicom/umicom.h"
#include "umicom/frontend/frontend_studio.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio web platform data shared with callers of this public contract.
 */
typedef struct UmiStudioWebPlatform UmiStudioWebPlatform;
/**
 * Represent the studio web platform snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioWebPlatformSnapshot { size_t routes; size_t endpoints; size_t sessions; uint64_t requests; size_t chart_series; size_t frontend_pages; char browser_url[UMI_BROWSER_URL_CAPACITY]; } UmiStudioWebPlatformSnapshot;
/**
 * Initialise studio web platform from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_web_platform_create(UmiStudioWebPlatform **out_platform);
/**
 * Release or reset state held by studio web platform so the same storage can be reused
 * safely.
 */
void umi_studio_web_platform_destroy(UmiStudioWebPlatform *platform);
/**
 * Provide the studio web platform snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_web_platform_snapshot(const UmiStudioWebPlatform *platform,UmiStudioWebPlatformSnapshot *out_snapshot);
/**
 * Provide the studio web platform web operation used by this module and its client
 * applications.
 */
UmiWebService *umi_studio_web_platform_web(UmiStudioWebPlatform *platform);
/**
 * Provide the studio web platform browser operation used by this module and its client
 * applications.
 */
UmiBrowser *umi_studio_web_platform_browser(UmiStudioWebPlatform *platform);
/**
 * Provide the studio web platform chart operation used by this module and its client
 * applications.
 */
UmiChartModel *umi_studio_web_platform_chart(UmiStudioWebPlatform *platform);
/**
 * Provide the studio web platform frontend operation used by this module and its client
 * applications.
 */
UmiFrontendPlan *umi_studio_web_platform_frontend(UmiStudioWebPlatform *platform);
/**
 * Provide the studio web platform frontend studio operation used by this module and its
 * client applications.
 */
UmiFrontendStudio *umi_studio_web_platform_frontend_studio(UmiStudioWebPlatform *platform);
#ifdef __cplusplus
}
#endif
#endif
