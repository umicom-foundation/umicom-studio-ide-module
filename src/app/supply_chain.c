/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/supply_chain.c
 *
 * PURPOSE:
 *   Track Studio checksums, signatures, licences, SBOM and provenance gates.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/supply_chain.h"
#include <stdio.h>
#include <string.h>

/*
 * Initialise studio supply chain from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_supply_chain_init(UmiStudioSupplyChain *supply_chain)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (supply_chain == NULL) return;
    (void)memset(supply_chain, 0, sizeof(*supply_chain));
    umi_release_evidence_init(&supply_chain->evidence);
    (void)snprintf(supply_chain->checksum_algorithm,
                   sizeof(supply_chain->checksum_algorithm), "%s", "SHA-256");
    (void)snprintf(supply_chain->sbom_format,
                   sizeof(supply_chain->sbom_format), "%s", "CycloneDX JSON");
}

/*
 * Provide the studio supply chain mark local complete operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_supply_chain_mark_local_complete(
    UmiStudioSupplyChain *supply_chain)
{
    static const char *checks[] = {
        "tests", "checksums", "licences", "sbom", "provenance"
    };
    size_t index;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (supply_chain == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(checks) / sizeof(checks[0]); ++index) {
        status = umi_release_evidence_set(
            &supply_chain->evidence, checks[index], UMI_EVIDENCE_PASS);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return umi_release_evidence_set(
        &supply_chain->evidence, "signatures", UMI_EVIDENCE_SKIP);
}

/*
 * Provide the studio supply chain approve signature operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_supply_chain_approve_signature(
    UmiStudioSupplyChain *supply_chain,
    const char *signer)
{
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (supply_chain == NULL || signer == NULL || signer[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(supply_chain->signer, sizeof(supply_chain->signer),
                       "%s", signer);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(supply_chain->signer)) {
        supply_chain->signer[0] = '\0';
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    return umi_release_evidence_set(
        &supply_chain->evidence, "signatures", UMI_EVIDENCE_PASS);
}

/*
 * Provide the studio supply chain ready operation used by this module and its client
 * applications.
 */
int umi_studio_supply_chain_ready(
    const UmiStudioSupplyChain *supply_chain,
    UmiReleaseChannel channel)
{
    return supply_chain != NULL &&
           umi_release_evidence_ready(&supply_chain->evidence, channel);
}
