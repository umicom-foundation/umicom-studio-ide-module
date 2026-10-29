/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_command_inventory.c
 *
 * PURPOSE:
 *   Verify every required Studio command identity rather than assuming Studio
 *   owns every entry in the extensible Framework command registry.
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

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiCommandRegistry *registry;
    size_t index;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    registry = umi_studio_bootstrap_command_registry(bootstrap);
    assert(registry != NULL);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_platform_contract_core_command_count();
         ++index) {
        const char *id = umi_studio_platform_contract_core_command_id(index);
        assert(id != NULL);
        assert(umi_runtime_inventory_has_command(registry, id));
    }
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_platform_contract_workbench_command_count();
         ++index) {
        const char *id = umi_studio_platform_contract_workbench_command_id(index);
        assert(id != NULL);
        assert(umi_runtime_inventory_has_command(registry, id));
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_platform_contract_contributed_command_count();
         ++index) {
        const char *id = umi_studio_platform_contract_contributed_command_id(index);
        assert(id != NULL);
        assert(umi_runtime_inventory_has_command(registry, id));
    }

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
