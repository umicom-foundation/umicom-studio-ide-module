/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_packages.c
 *
 * PURPOSE:
 *   Implement the security packages behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Package signing trust v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_packages.h"

/*
 * Add studio security signer only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_signer_add(UmiStudioSecurityCentre *centre,
                                         const UmiSecurityTrustedSigner *signer)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_security_signer_store_add(&centre->governance.signers,signer);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->governance.revision += 1U;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio security package verify operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_package_verify(
    const UmiStudioSecurityCentre *centre,
    const UmiSecurityPackageEvidence *evidence,
    UmiSecurityEnvironment environment,
    UmiSecurityGovernanceDecision *out_decision)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_security_package_trust_evaluate(&centre->governance.signers,
                                               evidence,
                                               environment,
                                               out_decision);
}
