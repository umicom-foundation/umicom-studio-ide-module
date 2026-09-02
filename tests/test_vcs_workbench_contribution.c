/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_vcs_workbench_contribution.c
 *
 * PURPOSE:
 *   Verify Studio contributes only unique placements and creates the public
 *   Framework VCS workbench runtime.
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

#include "umicom/studio/vcs_workbench_contribution.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiVcsWorkbenchRuntime *runtime = NULL;
    UmiVcsWorkbenchRuntimeSnapshot snapshot;
    size_t index;
    size_t comparison;

    assert(umi_studio_vcs_workbench_command_count() ==
           umi_vcs_workbench_command_count());
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_studio_vcs_workbench_command_count(); ++index) {
        const UmiStudioVcsWorkbenchCommandContribution *command =
            umi_studio_vcs_workbench_command_at(index);
        assert(command != NULL);
        assert(umi_vcs_workbench_command_find(
                   command->framework_command_id) != NULL);
    }
    assert(umi_studio_vcs_workbench_view_count() == 6U);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_studio_vcs_workbench_view_count(); ++index) {
        const UmiStudioVcsWorkbenchViewContribution *view =
            umi_studio_vcs_workbench_view_at(index);
        assert(view != NULL);
        assert(view->framework_contract[0] != '\0');
        assert(view->default_region[0] != '\0');
        /* Visit each bounded item once so every record receives the same rule. */
        for (comparison = index + 1U;
             comparison < umi_studio_vcs_workbench_view_count(); ++comparison) {
            assert(strcmp(
                       view->view_id,
                       umi_studio_vcs_workbench_view_at(comparison)->view_id) !=
                   0);
        }
    }
    assert(umi_studio_vcs_workbench_view_find(
               "studio.vcs-workbench.partial-staging") != NULL);
    assert(umi_studio_vcs_workbench_create(&runtime) == UMI_STATUS_OK);
    assert(umi_vcs_workbench_runtime_snapshot(runtime, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.phase == UMI_VCS_WORKBENCH_IDLE);
    umi_vcs_workbench_runtime_destroy(runtime);
    return 0;
}
