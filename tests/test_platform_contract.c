/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_platform_contract.c
 *
 * PURPOSE:
 *   Verify the complete Studio-owned command/service composition contract after
 *   the Framework composition root has started.
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
    UmiStudioPlatformContractSnapshot snapshot;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_capture_bootstrap(
        bootstrap, &snapshot) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_validate(&snapshot) == UMI_STATUS_OK);
    assert(snapshot.valid);
    assert(snapshot.core_commands_complete);
    assert(snapshot.workbench_commands_complete);
    assert(snapshot.contributed_commands_complete);
    assert(snapshot.required_services_complete);
    assert(snapshot.runtime.command_count >= snapshot.expected_minimum_command_count);
    assert(snapshot.runtime.service_count >= snapshot.required_service_count);

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
