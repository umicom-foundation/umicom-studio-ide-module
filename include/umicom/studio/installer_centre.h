/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/installer_centre.h
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
#ifndef UMICOM_STUDIO_INSTALLER_CENTRE_H
#define UMICOM_STUDIO_INSTALLER_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio installer centre data shared with callers of this public contract.
 */
typedef struct UmiStudioInstallerCentre {
    UmiInstallerPlan windows;
    UmiInstallerPlan linux;
} UmiStudioInstallerCentre;

/**
 * Initialise studio installer centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_installer_centre_init(UmiStudioInstallerCentre *centre,
                                               const char *version);
/**
 * Provide the studio installer centre plan operation used by this module and its client
 * applications.
 */
UmiInstallerPlan *umi_studio_installer_centre_plan(
    UmiStudioInstallerCentre *centre,
    UmiInstallerPlatform platform);
/**
 * Provide the studio installer centre ready operation used by this module and its client
 * applications.
 */
int umi_studio_installer_centre_ready(
    const UmiStudioInstallerCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
