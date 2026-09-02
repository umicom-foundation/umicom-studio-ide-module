/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_integration_designer_centre.c
 *
 * PURPOSE:
 *   Verify the test integration designer centre behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE Tests | Integration Designer centre | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include <string.h>
#include "integration_designer_fixture.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioIntegrationDesignerCentre *centre = umi_test_integration_designer_centre();
    UmiStudioIntegrationDesignerSnapshot snapshot;
    assert(umi_studio_integration_designer_activate(centre,"workflow") == UMI_STATUS_OK);
    assert(umi_studio_integration_designer_snapshot(centre,&snapshot) == UMI_STATUS_OK);
    assert(strcmp(snapshot.active_view,"workflow") == 0 && snapshot.fabric.revision == 1U);
    assert(umi_studio_integration_designer_fabric(centre) != NULL);
    umi_studio_integration_designer_destroy(centre);
    return 0;
}
