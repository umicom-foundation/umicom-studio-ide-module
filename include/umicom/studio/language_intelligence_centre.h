/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/language_intelligence_centre.h
 *
 * PURPOSE:
 *   Compose the Language Intelligence Centre over reusable Umicom Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns product composition only. The reusable data model and service
 * remain in Umicom Framework so future applications can consume the same API.
 */
#ifndef UMICOM_STUDIO_LANGUAGE_INTELLIGENCE_CENTRE_H
#define UMICOM_STUDIO_LANGUAGE_INTELLIGENCE_CENTRE_H
#include <stdint.h>
#include "umicom/language/service.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio language intelligence centre data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioLanguageIntelligenceCentre UmiStudioLanguageIntelligenceCentre;
/**
 * Represent the studio language intelligence centre snapshot data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioLanguageIntelligenceCentreSnapshot {
    uint32_t struct_size; uint32_t api_version;
    char area_id[128]; char title[256]; char summary[512];
    UmiLanguageServiceSnapshot service; uint64_t revision; int available;
} UmiStudioLanguageIntelligenceCentreSnapshot;
/**
 * Initialise studio language intelligence centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_language_intelligence_centre_create(UmiStudioLanguageIntelligenceCentre **out_centre);
/**
 * Release or reset state held by studio language intelligence centre so the same storage
 * can be reused safely.
 */
void umi_studio_language_intelligence_centre_destroy(UmiStudioLanguageIntelligenceCentre *centre);
/**
 * Provide the studio language intelligence centre snapshot operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_language_intelligence_centre_snapshot(UmiStudioLanguageIntelligenceCentre *centre,UmiStudioLanguageIntelligenceCentreSnapshot *out_snapshot);
/**
 * Provide the studio language intelligence centre service operation used by this module
 * and its client applications.
 */
UmiLanguageService *umi_studio_language_intelligence_centre_service(UmiStudioLanguageIntelligenceCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
