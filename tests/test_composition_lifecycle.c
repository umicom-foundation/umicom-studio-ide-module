/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_composition_lifecycle.c
 *
 * PURPOSE:
 *   Verify the Studio composition can complete two independent create/start/
 *   contract-validation/stop/destroy cycles without leaking registry state.
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
 * Exercise run cycle and return a clear result when the behaviour no longer matches its
 * contract.
 */
static void run_cycle(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioPlatformContractSnapshot snapshot;

    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_capture_bootstrap(
        bootstrap, &snapshot) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_validate(&snapshot) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    run_cycle();
    run_cycle();
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
