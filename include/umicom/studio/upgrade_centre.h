/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/upgrade_centre.h
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
#ifndef UMICOM_STUDIO_UPGRADE_CENTRE_H
#define UMICOM_STUDIO_UPGRADE_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio upgrade centre data shared with callers of this public contract.
 */
typedef struct UmiStudioUpgradeCentre {
    UmiUpgradePlan plan;
    char status[UMI_DELIVERY_TEXT_CAPACITY];
} UmiStudioUpgradeCentre;

/**
 * Provide the studio upgrade centre prepare operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_upgrade_centre_prepare(
    UmiStudioUpgradeCentre *centre,
    const char *current_version,
    const char *target_version,
    uint64_t current_generation,
    uint64_t target_generation,
    int compatible);
/**
 * Provide the studio upgrade centre approve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_upgrade_centre_approve(
    UmiStudioUpgradeCentre *centre,
    int backup_available);
/**
 * Provide the studio upgrade centre ready operation used by this module and its client
 * applications.
 */
int umi_studio_upgrade_centre_ready(
    const UmiStudioUpgradeCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
