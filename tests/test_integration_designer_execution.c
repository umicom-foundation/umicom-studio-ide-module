/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_integration_designer_execution.c
 *
 * PURPOSE:
 *   Verify the test integration designer execution behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE Tests | Deterministic execution | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include <string.h>
#include "integration_designer_fixture.h"
int main(void)
{
    UmiStudioIntegrationDesignerCentre *centre = umi_test_seeded_integration_designer();
    UmiIntegrationSimulation simulation;
    assert(umi_studio_integration_designer_simulate(centre,"order-sync",&simulation) == UMI_STATUS_OK);
    assert(simulation.status == UMI_INTEGRATION_DESIGNER_RUN_SUCCEEDED && simulation.trace_count == 3U);
    assert(strcmp(simulation.trace[1].detail,"Request planned without network I/O") == 0);
    assert(centre->fabric.runs.count == 1U);
    umi_studio_integration_designer_destroy(centre);
    return 0;
}
