/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/test_readiness_contribution.h
 * PURPOSE: Place Framework test-readiness commands and panel inside Studio.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TEST_READINESS_CONTRIBUTION_H
#define UMICOM_STUDIO_TEST_READINESS_CONTRIBUTION_H

#include <stddef.h>

#include "umicom/test_platform/build_readiness/build_readiness.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio test readiness contribution data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioTestReadinessContribution {
    const char *product_id;
    const char *view_id;
    const char *region;
    const UmiTestPlatformProductValidationProfile *profile;
    size_t command_count;
} UmiStudioTestReadinessContribution;

/**
 * Provide the studio test readiness contribution operation used by this module and its
 * client applications.
 */
const UmiStudioTestReadinessContribution *
umi_studio_test_readiness_contribution(void);

#ifdef __cplusplus
}
#endif
#endif
