/*-----------------------------------------------------------------------------
 * Umicom Studio
 * File: src/experience_acceptance.c
 *
 * PURPOSE:
 *   Compose Framework journey audit and evidence into thin product readiness.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/experience_acceptance.h"
#include <string.h>
/*
 * Provide the studio experience acceptance build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_experience_acceptance_build(
    const UmiApplicationJourneyEvidenceRegistry *evidence,
    UmiStudioExperienceAcceptance *out_acceptance)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (evidence == NULL || out_acceptance == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_acceptance, 0, sizeof(*out_acceptance));
    status = umi_application_journey_catalogue_build(
        &out_acceptance->catalogue);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_application_journey_audit_catalogue(
            &out_acceptance->catalogue, &out_acceptance->audit);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_application_journey_report_build(
            &out_acceptance->catalogue, evidence, "org.umicom.studio",
            &out_acceptance->report);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        out_acceptance->product_ready =
            out_acceptance->audit.valid &&
            out_acceptance->report.release_ready;
    return status;
}

