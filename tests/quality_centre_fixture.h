/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: tests/quality_centre_fixture.h
 *
 * PURPOSE:
 *   Provide small caller-owned Quality Centre fixtures shared by focused
 *   Studio tests without copying setup and cleanup assumptions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TEST_QUALITY_CENTRE_FIXTURE_H
#define UMICOM_STUDIO_TEST_QUALITY_CENTRE_FIXTURE_H

#include <assert.h>

#include "umicom/studio/quality_commands.h"

/* Create a centre over a stable non-NULL service token owned by the test process. */
static inline UmiStudioQualityCentre *umi_test_quality_centre(void)
{
    static int services_token;
    UmiStudioQualityCentre *centre = NULL;
    assert(umi_studio_quality_centre_create(
        (UmiStudioServices *)(void *)&services_token,
        &centre) == UMI_STATUS_OK);
    return centre;
}

/* Create a centre and populate its existing architecture-quality fixture data. */
static inline UmiStudioQualityCentre *umi_test_seeded_quality_centre(void)
{
    UmiStudioQualityCentre *centre = umi_test_quality_centre();
    assert(umi_studio_quality_seed_evidence(centre) == UMI_STATUS_OK);
    return centre;
}

#endif
