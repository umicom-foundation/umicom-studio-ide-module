/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/integration_designer_fixture.h
 *
 * PURPOSE:
 *   Verify the integration designer fixture behaviour and report a clear failure
 *   when its contract changes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Umicom Studio IDE Tests | Integration Designer fixture | Sammy Hegab | Umicom Foundation | MIT */
#ifndef UMICOM_STUDIO_TEST_INTEGRATION_DESIGNER_FIXTURE_H
#define UMICOM_STUDIO_TEST_INTEGRATION_DESIGNER_FIXTURE_H
#include <assert.h>
#include "umicom/studio/integration_designer_commands.h"
/**
 * Exercise test integration designer centre and return a clear result when the behaviour
 * no longer matches its contract.
 */
static inline UmiStudioIntegrationDesignerCentre *umi_test_integration_designer_centre(void)
{
    static int services_token;
    UmiStudioIntegrationDesignerCentre *centre = NULL;
    assert(umi_studio_integration_designer_create((UmiStudioServices *)(void *)&services_token,&centre) == UMI_STATUS_OK);
    return centre;
}
/**
 * Exercise test seeded integration designer and return a clear result when the behaviour
 * no longer matches its contract.
 */
static inline UmiStudioIntegrationDesignerCentre *umi_test_seeded_integration_designer(void)
{
    UmiStudioIntegrationDesignerCentre *centre = umi_test_integration_designer_centre();
    assert(umi_studio_integration_designer_seed_sample(centre) == UMI_STATUS_OK);
    return centre;
}
#endif
