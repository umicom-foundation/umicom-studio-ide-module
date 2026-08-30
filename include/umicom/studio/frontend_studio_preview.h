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
UmiStatus umi_studio_frontend_preview_start(UmiStudioWebPlatform *platform);
UmiStatus umi_studio_frontend_preview_stop(UmiStudioWebPlatform *platform);
UmiStatus umi_studio_frontend_preview_navigate(UmiStudioWebPlatform *platform,const char *url);
UmiStatus umi_studio_frontend_preview_select_device(UmiStudioWebPlatform *platform,const char *device_id);
UmiStatus umi_studio_frontend_preview_reload(UmiStudioWebPlatform *platform);
#endif
