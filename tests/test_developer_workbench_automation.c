/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_developer_workbench_automation.c
 *
 * PURPOSE:
 *   Verify every new automation centre borrows one shared developer runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_workbench.h"

#include <assert.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioDeveloperWorkbench *workbench = NULL;
    UmiDeveloperRuntime *runtime;
    assert(umi_studio_developer_workbench_create(&workbench) == UMI_STATUS_OK);
    runtime = umi_studio_developer_workbench_runtime(workbench);
    assert(runtime != NULL);
    assert(umi_studio_developer_task_centre_runtime(
        umi_studio_developer_workbench_task_centre(workbench)) == runtime);
    assert(umi_studio_developer_run_centre_runtime(
        umi_studio_developer_workbench_run_centre(workbench)) == runtime);
    assert(umi_studio_developer_workspace_state_centre_runtime(
        umi_studio_developer_workbench_workspace_state(workbench)) == runtime);
    assert(umi_studio_developer_pipeline_centre_runtime(
        umi_studio_developer_workbench_pipeline(workbench)) == runtime);
    umi_studio_developer_workbench_destroy(workbench);
    return 0;
}
