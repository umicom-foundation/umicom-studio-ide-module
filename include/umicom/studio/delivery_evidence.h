/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/delivery_evidence.h
 *
 * PURPOSE:
 *   Populate Studio build, SBOM and licence evidence used by release gates.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Evidence helpers provide deterministic test data now and a stable place to connect real build results later.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_EVIDENCE_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_EVIDENCE_H

#include "umicom/umicom.h"
#include "umicom/studio/delivery_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio delivery build evidence operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_delivery_build_evidence(const char *source_revision,
                                                uint32_t tests_passed,
                                                uint32_t tests_failed,
                                                uint32_t warnings,
                                                UmiBuildEvidence *out_evidence);
/**
 * Provide the studio delivery sbom operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_delivery_sbom(UmiSbom *out_sbom,
                                   UmiLicenseInventory *out_licenses);

#ifdef __cplusplus
}
#endif

#endif
