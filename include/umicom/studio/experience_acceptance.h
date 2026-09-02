/*-----------------------------------------------------------------------------
 * Umicom Studio
 * File: include/umicom/studio/experience_acceptance.h
 *
 * PURPOSE:
 *   Project Framework-owned executable journey evidence into this thin product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef STUDIO_EXPERIENCE_ACCEPTANCE_H
#define STUDIO_EXPERIENCE_ACCEPTANCE_H
#include "umicom/application/journey/journey_platform.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio experience acceptance data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioExperienceAcceptance {
    UmiApplicationJourneyCatalogue catalogue;
    UmiApplicationJourneyAudit audit;
    UmiApplicationJourneyReport report;
    int product_ready;
} UmiStudioExperienceAcceptance;
/**
 * Provide the studio experience acceptance build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_experience_acceptance_build(
    const UmiApplicationJourneyEvidenceRegistry *evidence,
    UmiStudioExperienceAcceptance *out_acceptance);
#ifdef __cplusplus
}
#endif
#endif

