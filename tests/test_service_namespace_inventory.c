/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_service_namespace_inventory.c
 *
 * PURPOSE:
 *   Verify namespace-level inventory reporting for Studio commands and services
 *   while keeping the registry open to other Framework-owned contributions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
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
    UmiStudioPlatformContractSnapshot snapshot;

    (void)umi_fs_remove_tree(".umicom");
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_capture_bootstrap(
        bootstrap, &snapshot) == UMI_STATUS_OK);
    size_t expectedNamespace = 0U;
    for (size_t index = 0U; index < umi_studio_platform_contract_core_command_count(); ++index)
        if (strncmp(umi_studio_platform_contract_core_command_id(index), "studio.", 7U) == 0)
            ++expectedNamespace;
    for (size_t index = 0U; index < umi_studio_platform_contract_workbench_command_count(); ++index)
        if (strncmp(umi_studio_platform_contract_workbench_command_id(index), "studio.", 7U) == 0)
            ++expectedNamespace;
    assert(snapshot.missing_core_command_count == 0U);
    assert(snapshot.missing_workbench_command_count == 0U);
    assert(snapshot.missing_contributed_command_count == 0U);
    assert(snapshot.studio_namespace_command_count >= expectedNamespace);
    assert(snapshot.studio_namespace_service_count >=
           snapshot.required_service_count);
    assert(snapshot.runtime.command_count >= snapshot.expected_minimum_command_count);

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
    return 0;
}
