/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/installer_centre.c
 *
 * PURPOSE:
 *   Configure Studio Windows and Linux installer-generation plans.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/installer_centre.h"
#include <string.h>

/*
 * Initialise studio installer centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_installer_centre_init(UmiStudioInstallerCentre *centre,
                                               const char *version)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || version == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(centre, 0, sizeof(*centre));
    status = umi_installer_plan_init(
        &centre->windows, "Umicom Studio", "Umicom Foundation", version,
        "Umicom Studio", "bin/umicom-studio-ide.exe",
        UMI_INSTALLER_WINDOWS, UMI_INSTALL_SCOPE_USER);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    umi_installer_plan_set_shortcuts(&centre->windows, 1, 0);
    status = umi_installer_plan_init(
        &centre->linux, "Umicom Studio", "Umicom Foundation", version,
        "/opt/umicom-studio", "bin/umicom-studio-ide",
        UMI_INSTALLER_LINUX, UMI_INSTALL_SCOPE_USER);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    umi_installer_plan_set_shortcuts(&centre->linux, 0, 0);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio installer centre plan operation used by this module and its client
 * applications.
 */
UmiInstallerPlan *umi_studio_installer_centre_plan(
    UmiStudioInstallerCentre *centre,
    UmiInstallerPlatform platform)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return NULL;
    /* Apply this branch only when its contract condition is satisfied. */
    if (platform == UMI_INSTALLER_WINDOWS) return &centre->windows;
    /* Apply this branch only when its contract condition is satisfied. */
    if (platform == UMI_INSTALLER_LINUX) return &centre->linux;
    return NULL;
}

/*
 * Provide the studio installer centre ready operation used by this module and its client
 * applications.
 */
int umi_studio_installer_centre_ready(
    const UmiStudioInstallerCentre *centre)
{
    return centre != NULL &&
           umi_installer_plan_validate(&centre->windows) == UMI_STATUS_OK &&
           umi_installer_plan_validate(&centre->linux) == UMI_STATUS_OK;
}
