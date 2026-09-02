/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/package_project.c
 *
 * PURPOSE:
 *   Build Windows and Linux package manifests for the Studio product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/package_project.h"
#include <string.h>

/*
 * Provide the studio package add common operation used by this module and its client
 * applications.
 */
static UmiStatus umi_studio_package_add_common(UmiPackageManifest *manifest,
                                                const char *source,
                                                const char *destination)
{
    UmiStatus status = umi_package_manifest_add(
        manifest, source, destination, UMI_PACKAGE_FILE_EXECUTABLE, 1);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_package_manifest_add(
        manifest, "LICENSE", "share/licenses/umicom-studio/LICENSE",
        UMI_PACKAGE_FILE_LICENCE, 1);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_package_manifest_add(
        manifest, "config", "share/umicom/config",
        UMI_PACKAGE_FILE_CONFIGURATION, 0);
}

/*
 * Initialise studio package project from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_package_project_init(UmiStudioPackageProject *project,
                                              const char *version)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (project == NULL || version == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(project, 0, sizeof(*project));
    status = umi_package_manifest_init(
        &project->windows, "umicom-studio", version,
        "bin/umicom-studio-console.exe", UMI_PACKAGE_WINDOWS_SETUP);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_package_add_common(
        &project->windows, "build/bin/umicom-studio-console.exe",
        "bin/umicom-studio-console.exe");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_package_manifest_init(
        &project->linux, "umicom-studio", version,
        "bin/umicom-studio-console", UMI_PACKAGE_TAR_GZ);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_studio_package_add_common(
        &project->linux, "build/bin/umicom-studio-console",
        "bin/umicom-studio-console");
}

/*
 * Provide the studio package project manifest operation used by this module and its client
 * applications.
 */
const UmiPackageManifest *umi_studio_package_project_manifest(
    const UmiStudioPackageProject *project,
    UmiInstallerPlatform platform)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (project == NULL) return NULL;
    /* Apply this branch only when its contract condition is satisfied. */
    if (platform == UMI_INSTALLER_WINDOWS) return &project->windows;
    /* Apply this branch only when its contract condition is satisfied. */
    if (platform == UMI_INSTALLER_LINUX) return &project->linux;
    return NULL;
}

/*
 * Provide the studio package project ready operation used by this module and its client
 * applications.
 */
int umi_studio_package_project_ready(const UmiStudioPackageProject *project)
{
    return project != NULL &&
           umi_package_manifest_validate(&project->windows) == UMI_STATUS_OK &&
           umi_package_manifest_validate(&project->linux) == UMI_STATUS_OK;
}
