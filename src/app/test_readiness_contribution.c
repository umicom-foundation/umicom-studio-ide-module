/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/test_readiness_contribution.c
 * PURPOSE: Implement thin placement over Framework readiness contracts.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/test_readiness_contribution.h"

/*
 * Provide the studio test readiness contribution operation used by this module and its
 * client applications.
 */
const UmiStudioTestReadinessContribution *
umi_studio_test_readiness_contribution(void)
{
    static UmiStudioTestReadinessContribution contribution;
    contribution.product_id = "studio";
    contribution.view_id = "studio.testing.build-readiness";
    contribution.region = "bottom-panel";
    contribution.profile =
        umi_test_platform_product_validation_profile_find("studio");
    contribution.command_count =
        umi_test_platform_build_readiness_command_count();
    return contribution.profile != NULL ? &contribution : NULL;
}
