/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/distribution_centre.h
 *
 * PURPOSE:
 *   Compose the complete Studio package, installer and release workflow.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DISTRIBUTION_CENTRE_H
#define UMICOM_STUDIO_DISTRIBUTION_CENTRE_H

#include "umicom/studio/installer_centre.h"
#include "umicom/studio/package_project.h"
#include "umicom/studio/release_channels.h"
#include "umicom/studio/supply_chain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio distribution centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDistributionCentre {
    UmiStudioPackageProject packages;
    UmiStudioInstallerCentre installers;
    UmiStudioReleaseChannels channels;
    UmiStudioSupplyChain supply_chain;
    UmiProductRelease release;
} UmiStudioDistributionCentre;

/**
 * Initialise studio distribution centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_distribution_centre_init(
    UmiStudioDistributionCentre *centre,
    const char *release_id,
    const char *version);
/**
 * Provide the studio distribution centre refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_distribution_centre_refresh(
    UmiStudioDistributionCentre *centre);
/**
 * Provide the studio distribution centre blockers operation used by this module and its
 * client applications.
 */
size_t umi_studio_distribution_centre_blockers(
    const UmiStudioDistributionCentre *centre);
/**
 * Provide the studio distribution centre ready operation used by this module and its
 * client applications.
 */
int umi_studio_distribution_centre_ready(
    const UmiStudioDistributionCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
