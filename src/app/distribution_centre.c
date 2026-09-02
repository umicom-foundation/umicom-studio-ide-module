/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/distribution_centre.c
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
#include "umicom/studio/distribution_centre.h"
#include <string.h>

/*
 * Provide the studio distribution centre refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_distribution_centre_refresh(
    UmiStudioDistributionCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    centre->release.evidence = centre->supply_chain.evidence;
    return UMI_STATUS_OK;
}

/*
 * Initialise studio distribution centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_distribution_centre_init(
    UmiStudioDistributionCentre *centre,
    const char *release_id,
    const char *version)
{
    const UmiUpdateChannel *channel;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || release_id == NULL || version == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(centre, 0, sizeof(*centre));
    status = umi_studio_package_project_init(&centre->packages, version);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_installer_centre_init(&centre->installers, version);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_release_channels_init(&centre->channels);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    umi_studio_supply_chain_init(&centre->supply_chain);
    channel = umi_studio_release_channels_current(&centre->channels);
    status = umi_product_release_init(
        &centre->release, release_id, UMI_RELEASE_STABLE,
        &centre->packages.windows, channel);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_product_release_add_installer(
        &centre->release, &centre->installers.windows);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_product_release_add_installer(
        &centre->release, &centre->installers.linux);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_studio_distribution_centre_refresh(centre);
}

/*
 * Provide the studio distribution centre blockers operation used by this module and its
 * client applications.
 */
size_t umi_studio_distribution_centre_blockers(
    const UmiStudioDistributionCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return 1U;
    return umi_product_release_blocker_count(&centre->release);
}

/*
 * Provide the studio distribution centre ready operation used by this module and its
 * client applications.
 */
int umi_studio_distribution_centre_ready(
    const UmiStudioDistributionCentre *centre)
{
    return centre != NULL && umi_product_release_ready(&centre->release);
}
