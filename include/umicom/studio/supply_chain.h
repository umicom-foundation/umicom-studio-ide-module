/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/supply_chain.h
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
#ifndef UMICOM_STUDIO_SUPPLY_CHAIN_H
#define UMICOM_STUDIO_SUPPLY_CHAIN_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio supply chain data shared with callers of this public contract.
 */
typedef struct UmiStudioSupplyChain {
    UmiReleaseEvidence evidence;
    char checksum_algorithm[32];
    char sbom_format[32];
    char signer[UMI_DELIVERY_ID_CAPACITY];
} UmiStudioSupplyChain;

/**
 * Initialise studio supply chain from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_supply_chain_init(UmiStudioSupplyChain *supply_chain);
/**
 * Provide the studio supply chain mark local complete operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_supply_chain_mark_local_complete(
    UmiStudioSupplyChain *supply_chain);
/**
 * Provide the studio supply chain approve signature operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_supply_chain_approve_signature(
    UmiStudioSupplyChain *supply_chain,
    const char *signer);
/**
 * Provide the studio supply chain ready operation used by this module and its client
 * applications.
 */
int umi_studio_supply_chain_ready(
    const UmiStudioSupplyChain *supply_chain,
    UmiReleaseChannel channel);

#ifdef __cplusplus
}
#endif
#endif
