/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_quality_commands.c
 *
 * PURPOSE:
 *   Verify stable Studio quality commands seed, evaluate and refresh reusable
 *   Framework quality evidence.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>

#include "quality_centre_fixture.h"

/* Exercise all public Quality Centre commands and their output requirements. */
int main(void)
{
    UmiStudioQualityCentre *centre = umi_test_quality_centre();
    UmiCodeGuardQualityEvaluation evaluation;
    /* Seeding gives the remaining commands deterministic evidence to process. */
    assert(umi_studio_quality_execute(
        centre,
        UMI_STUDIO_QUALITY_COMMAND_SEED_EVIDENCE,
        NULL) == UMI_STATUS_OK);
    assert(umi_studio_quality_execute(
        centre,
        UMI_STUDIO_QUALITY_COMMAND_EVALUATE,
        &evaluation) == UMI_STATUS_OK);
    assert(evaluation.decision == UMI_CODEGUARD_QUALITY_WARN);
    assert(evaluation.pending_duplicate_reviews == 1U);
    assert(umi_studio_quality_execute(
        centre,
        UMI_STUDIO_QUALITY_COMMAND_REFRESH_REMEDIATION,
        NULL) == UMI_STATUS_OK);
    umi_studio_quality_centre_destroy(centre);
    return 0;
}
