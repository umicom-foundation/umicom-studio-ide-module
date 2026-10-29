/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_state_inventory_consistency.c
 *
 * PURPOSE:
 *   Verify the operational state report reflects the authoritative runtime
 *   registry counts instead of a duplicated compile-time command total.
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
#include "umicom/studio/state.h"

#include <assert.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioStateReport state;
    UmiStudioPlatformContractSnapshot contract;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_state_capture(bootstrap, &state) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_capture_bootstrap(
        bootstrap, &contract) == UMI_STATUS_OK);
    assert(state.command_count == contract.runtime.command_count);
    assert(state.service_count == contract.runtime.service_count);
    assert(state.command_count >= contract.expected_minimum_command_count);

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
