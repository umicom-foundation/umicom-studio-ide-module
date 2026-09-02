/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/upgrade_centre.c
 *
 * PURPOSE:
 *   Prepare, approve and explain Studio upgrades and rollback points.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/upgrade_centre.h"
#include <stdio.h>
#include <string.h>

/*
 * Provide the studio upgrade centre prepare operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_upgrade_centre_prepare(
    UmiStudioUpgradeCentre *centre,
    const char *current_version,
    const char *target_version,
    uint64_t current_generation,
    uint64_t target_generation,
    int compatible)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(centre, 0, sizeof(*centre));
    status = umi_upgrade_plan_init(
        &centre->plan, current_version, target_version, current_generation,
        target_generation, compatible);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    (void)snprintf(centre->status, sizeof(centre->status),
                   "Upgrade %s to %s is waiting for a backup.",
                   current_version, target_version);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio upgrade centre approve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_upgrade_centre_approve(
    UmiStudioUpgradeCentre *centre,
    int backup_available)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_upgrade_plan_authorise(&centre->plan, backup_available);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(centre->status, sizeof(centre->status),
                       "Upgrade approved; rollback generation is %llu.",
                       (unsigned long long)
                           umi_upgrade_plan_rollback_generation(&centre->plan));
    }
    return status;
}

/*
 * Provide the studio upgrade centre ready operation used by this module and its client
 * applications.
 */
int umi_studio_upgrade_centre_ready(
    const UmiStudioUpgradeCentre *centre)
{
    return centre != NULL &&
           umi_upgrade_plan_validate(&centre->plan) == UMI_STATUS_OK;
}
