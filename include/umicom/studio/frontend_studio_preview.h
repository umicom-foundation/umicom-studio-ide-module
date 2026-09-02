/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_preview.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio preview contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_PREVIEW_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_PREVIEW_H
#include "umicom/studio/web_platform.h"
/**
 * Provide the studio frontend preview start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_start(UmiStudioWebPlatform *platform);
/**
 * Provide the studio frontend preview stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_stop(UmiStudioWebPlatform *platform);
/**
 * Provide the studio frontend preview navigate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_preview_navigate(UmiStudioWebPlatform *platform,const char *url);
/**
 * Provide the studio frontend preview select device operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_preview_select_device(UmiStudioWebPlatform *platform,const char *device_id);
/**
 * Provide the studio frontend preview reload operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_reload(UmiStudioWebPlatform *platform);
#endif
