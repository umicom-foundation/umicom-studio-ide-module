/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/security_centre_fixture.h
 *
 * PURPOSE:
 *   Verify the security centre fixture behaviour and report a clear failure when
 *   its contract changes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Umicom Studio IDE Tests | Security Centre v2 fixture | Sammy Hegab | Umicom Foundation | MIT */
#ifndef UMICOM_STUDIO_TEST_SECURITY_CENTRE_FIXTURE_H
#define UMICOM_STUDIO_TEST_SECURITY_CENTRE_FIXTURE_H
#include <assert.h>
#include "umicom/studio/security_commands.h"

/**
 * Exercise test security centre and return a clear result when the behaviour no longer
 * matches its contract.
 */
static inline UmiStudioSecurityCentre *umi_test_security_centre(void)
{
    static int services_token;
    UmiStudioSecurityCentre *centre = NULL;
    assert(umi_studio_security_centre_create(
        (UmiStudioServices *)(void *)&services_token,&centre) == UMI_STATUS_OK);
    return centre;
}

/**
 * Exercise test seeded security centre and return a clear result when the behaviour no
 * longer matches its contract.
 */
static inline UmiStudioSecurityCentre *umi_test_seeded_security_centre(void)
{
    UmiStudioSecurityCentre *centre = umi_test_security_centre();
    assert(umi_studio_security_seed(centre) == UMI_STATUS_OK);
    return centre;
}
#endif
