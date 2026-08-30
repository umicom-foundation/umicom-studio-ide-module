/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_settings.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio settings contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_SETTINGS_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_SETTINGS_H
#include "umicom/studio/web_platform.h"
typedef struct UmiStudioFrontendSettings { char source_root[UMI_FRONTEND_DEV_PATH_CAPACITY]; char host[128]; uint16_t port; int hot_reload; int allow_remote; UmiFrontendQualityBudget quality_budget; } UmiStudioFrontendSettings;
UmiStudioFrontendSettings umi_studio_frontend_settings_default(void);
UmiStatus umi_studio_frontend_settings_apply(UmiStudioWebPlatform *platform,const UmiStudioFrontendSettings *settings);
#endif
