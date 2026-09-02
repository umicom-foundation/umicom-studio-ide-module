/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/knowledge_settings.h
 * PURPOSE: Apply validated Knowledge Centre settings to Studio AI composition.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Typed settings are validated before the service allocates its index. */
#ifndef UMICOM_STUDIO_KNOWLEDGE_SETTINGS_H
#define UMICOM_STUDIO_KNOWLEDGE_SETTINGS_H

#include "umicom/studio/ai_platform.h"
#include "umicom/studio/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Perform studio knowledge settings through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_knowledge_settings_apply(
    const UmiSettings *settings,
    UmiStudioAiPlatformConfig *config);

#ifdef __cplusplus
}
#endif
#endif
