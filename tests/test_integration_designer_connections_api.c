/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_integration_designer_connections_api.c
 *
 * PURPOSE:
 *   Verify the test integration designer connections api behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE Tests | Connections and API explorer | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include <string.h>
#include "integration_designer_fixture.h"
#include "umicom/studio/integration_designer_api.h"
#include "umicom/studio/integration_designer_connections.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioIntegrationDesignerCentre *centre = umi_test_seeded_integration_designer();
    UmiIntegrationRequestPlan plan;
    assert(umi_studio_integration_designer_find_connection(centre,"orders-api") != NULL);
    assert(umi_studio_integration_designer_find_contract(centre,"orders") != NULL);
    assert(umi_studio_integration_designer_plan_request(centre,"orders","create-order","{}","corr-studio",&plan) == UMI_STATUS_OK);
    assert(strcmp(plan.target,"https://api.example.test/orders") == 0);
    assert(strcmp(plan.secret_reference,"vault://integration/orders-key") == 0);
    umi_studio_integration_designer_destroy(centre);
    return 0;
}
