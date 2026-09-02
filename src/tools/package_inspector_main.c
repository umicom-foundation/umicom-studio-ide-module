/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/package_inspector_main.c
 *
 * PURPOSE:
 *   Provide a native command for inspecting Studio package manifests.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include "umicom/studio/package_project.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioPackageProject project;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_studio_package_project_init(&project, "0.23.0") !=
        UMI_STATUS_OK) return 1;
    printf("Umicom Studio Package Inspector\n");
    printf("Windows files: %zu\n", project.windows.count);
    printf("Linux files: %zu\n", project.linux.count);
    return umi_studio_package_project_ready(&project) ? 0 : 1;
}
