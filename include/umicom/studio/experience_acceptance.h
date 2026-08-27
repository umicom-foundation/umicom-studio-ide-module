/*-----------------------------------------------------------------------------
 * Umicom Studio
 * File: include/umicom/studio/experience_acceptance.h
 *
 * PURPOSE:
 *   Project Framework-owned executable journey evidence into this thin product.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#ifndef STUDIO_EXPERIENCE_ACCEPTANCE_H
#define STUDIO_EXPERIENCE_ACCEPTANCE_H
#include "umicom/application/journey/journey_platform.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioExperienceAcceptance {
    UmiApplicationJourneyCatalogue catalogue;
    UmiApplicationJourneyAudit audit;
    UmiApplicationJourneyReport report;
    int product_ready;
} UmiStudioExperienceAcceptance;
UmiStatus umi_studio_experience_acceptance_build(
    const UmiApplicationJourneyEvidenceRegistry *evidence,
    UmiStudioExperienceAcceptance *out_acceptance);
#ifdef __cplusplus
}
#endif
#endif

