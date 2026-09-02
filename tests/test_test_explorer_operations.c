/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_test_explorer_operations.c
 *
 * PURPOSE:
 *   Verify bound Test Explorer operation planning, start, stop and snapshots.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/test_explorer_centre.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioTestService *tests = NULL;
    UmiStudioTestExplorerCentre *centre = NULL;
    UmiTestPlatformItemSnapshot item;
    UmiTestPlatformOperationPlan plan;
    UmiStudioTestExplorerCentreSnapshot snapshot;
    assert(umi_studio_test_service_create(&tests) == UMI_STATUS_OK);
    (void)memset(&item, 0, sizeof(item));
    item.struct_size = (uint32_t)sizeof(item);
    item.api_version = UMI_TEST_PLATFORM_ITEM_API_VERSION;
    (void)strcpy(item.id, "studio.alpha");
    (void)strcpy(item.name, "Studio Alpha");
    (void)strcpy(item.kind, "test");
    (void)strcpy(item.framework, "ctest");
    (void)strcpy(item.suite_id, "studio");
    item.enabled = 1;
    item.discovered = 1;
    assert(umi_test_platform_item_registry_upsert(
               umi_test_platform_service_item(
                   umi_studio_test_service_platform(tests)),
               &item) == UMI_STATUS_OK);
    assert(umi_studio_test_explorer_centre_create_bound(tests, &centre) ==
           UMI_STATUS_OK);
    assert(umi_studio_test_explorer_centre_plan_all(centre, 2U, 0, &plan) ==
           UMI_STATUS_OK);
    assert(plan.selection.count == 1U);
    assert(umi_studio_test_explorer_centre_begin(centre, &plan) ==
           UMI_STATUS_OK);
    assert(umi_studio_test_explorer_centre_stop(centre) == UMI_STATUS_OK);
    assert(umi_studio_test_explorer_centre_snapshot(centre, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.operation_running == 1);
    assert(snapshot.service.stop_requested == 1);
    assert(snapshot.selected_count == 1U);
    umi_studio_test_explorer_centre_finish(centre);
    umi_studio_test_explorer_centre_destroy(centre);
    umi_studio_test_service_destroy(tests);
    return 0;
}
