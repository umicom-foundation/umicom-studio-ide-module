/*-----------------------------------------------------------------------------
 * Umicom Studio
 * File: tests/test_experience_acceptance.c
 *
 * PURPOSE:
 *   Verify the thin product consumes Framework journey readiness unchanged.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include "umicom/studio/experience_acceptance.h"
int main(void)
{
    UmiApplicationJourneyEvidenceRegistry evidence;
    UmiStudioExperienceAcceptance acceptance;
    umi_application_journey_evidence_registry_init(&evidence);
    assert(umi_studio_experience_acceptance_build(
               &evidence, &acceptance) == UMI_STATUS_OK);
    assert(acceptance.audit.valid);
    assert(acceptance.report.journey_count == 8U);
    assert(acceptance.report.step_count == 40U);
    assert(acceptance.report.pending_step_count == 40U);
    assert(!acceptance.product_ready);
    return 0;
}

