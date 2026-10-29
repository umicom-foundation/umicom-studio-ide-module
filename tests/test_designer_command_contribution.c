/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_designer_command_contribution.c
 *
 * PURPOSE:
 *   Verify the two reusable Framework Designer commands that Studio publishes
 *   alongside its own core and workbench command sets.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Setup and contract assertions remain active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include "umicom/studio/platform_contract.h"

#include <assert.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiCommandRegistry *registry;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_platform_contract_contributed_command_count() == 2U);
    assert(strcmp(umi_studio_platform_contract_contributed_command_id(0U),
                  "designer.undo") == 0);
    assert(strcmp(umi_studio_platform_contract_contributed_command_id(1U),
                  "designer.redo") == 0);

    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    registry = umi_studio_bootstrap_command_registry(bootstrap);
    assert(umi_runtime_inventory_has_command(registry, "designer.undo"));
    assert(umi_runtime_inventory_has_command(registry, "designer.redo"));

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
