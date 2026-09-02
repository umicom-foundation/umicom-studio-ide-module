/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/delivery_distribution.c
 *
 * PURPOSE:
 *   Define Studio's supported distribution targets and runtime bundle defaults.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Distribution metadata tells the release system what Studio intends to ship on each supported platform.
 */

#include "umicom/studio/delivery_distribution.h"

/*
 * Provide the studio distribution matrix operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_distribution_matrix(UmiPlatformMatrix *out_matrix)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_matrix == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_platform_matrix_init(out_matrix);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_platform_matrix_add(out_matrix, "windows", "x86_64") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    return umi_platform_matrix_add(out_matrix, "linux", "x86_64");
}

/*
 * Provide the studio runtime bundle operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_runtime_bundle(UmiRuntimeBundle *out_bundle)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_bundle == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_runtime_bundle_init(out_bundle);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_runtime_bundle_add(out_bundle, "bin/umicom-studio-ide") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_runtime_bundle_add(out_bundle, "bin/umicom") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    return umi_runtime_bundle_add(out_bundle, "share/umicom");
}
