/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_settings.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder settings contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Typed settings produce one reusable frontend-independent builder configuration. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_SETTINGS_H
#define UMICOM_STUDIO_VISUAL_BUILDER_SETTINGS_H
#include "umicom/studio/settings.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioVisualBuilderConfig {UmiDesignerSurfaceOptions surface;UmiDesignerPreviewProfile preview_profile;int show_generated_source;int auto_refresh_preview;} UmiStudioVisualBuilderConfig;
UmiStudioVisualBuilderConfig umi_studio_visual_builder_config_default(void);
UmiStatus umi_studio_visual_builder_settings_apply(const UmiSettings *settings,UmiStudioVisualBuilderConfig *config);
#ifdef __cplusplus
}
#endif
#endif
