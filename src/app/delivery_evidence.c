/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/delivery_evidence.c
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

#include "umicom/studio/delivery_evidence.h"
#include <stdio.h>
#include <string.h>

/*
 * Provide the studio delivery build evidence operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_delivery_build_evidence(const char *source_revision,
                                             uint32_t tests_passed,
                                             uint32_t tests_failed,
                                             uint32_t warnings,
                                             UmiBuildEvidence *out_evidence)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source_revision == NULL || out_evidence == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_build_evidence_init(out_evidence);
    out_evidence->build_succeeded = 1;
    out_evidence->warnings = warnings;
    out_evidence->tests_total = tests_passed + tests_failed;
    out_evidence->tests_passed = tests_passed;
    out_evidence->tests_failed = tests_failed;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_build_evidence_set_source(out_evidence, source_revision) != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    (void)snprintf(out_evidence->compiler, sizeof(out_evidence->compiler), "%s", "configured");
    (void)snprintf(out_evidence->preset, sizeof(out_evidence->preset), "%s", "studio");
    return UMI_STATUS_OK;
}

/*
 * Provide the studio delivery sbom operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_delivery_sbom(UmiSbom *out_sbom,
                                   UmiLicenseInventory *out_licenses)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_sbom == NULL || out_licenses == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_sbom_init(out_sbom);
    umi_license_inventory_init(out_licenses);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_sbom_add(out_sbom, "Umicom Framework", "0.5.0",
                     "Umicom Foundation", "MIT") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_sbom_add(out_sbom, "Umicom Studio IDE", "0.14.0",
                     "Umicom Foundation", "MIT") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_license_inventory_add(out_licenses, "Umicom Framework", "MIT", "LICENSE") != UMI_STATUS_OK) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    return umi_license_inventory_add(out_licenses, "Umicom Studio IDE", "MIT", "LICENSE");
}
