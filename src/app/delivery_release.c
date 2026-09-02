/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/delivery_release.c
 *
 * PURPOSE:
 *   Prepare Studio release candidates and register verified local release records.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * These helpers translate Studio product identity into Framework release records without implementing generic release mechanics twice.
 */

#include "umicom/studio/delivery_release.h"
#include "umicom/studio/version.h"

/*
 * Provide the studio release prepare operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_release_prepare(UmiStudioDeliveryPlatform *platform,
                                      const char *release_id,
                                      const char *source_revision,
                                      UmiReleaseChannel channel,
                                      UmiReleaseCandidate *out_candidate)
{
    (void)platform;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (release_id == NULL || source_revision == NULL || out_candidate == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    umi_release_candidate_init(out_candidate);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_delivery_manifest_init(&out_candidate->manifest,
                                   "org.umicom.studio",
                                   release_id,
                                   umi_studio_version_string(),
                                   channel) != UMI_STATUS_OK) {
        return UMI_STATUS_INVALID_STATE;
    }
    return umi_delivery_manifest_set_source_revision(&out_candidate->manifest,
                                                     source_revision);
}

/* Add studio release only after its inputs and available capacity have been checked. */
UmiStatus umi_studio_release_register(UmiStudioDeliveryPlatform *platform,
                                      const UmiReleaseCandidate *candidate,
                                      uint64_t generation)
{
    UmiRelease release;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || candidate == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (!umi_release_candidate_ready(candidate)) return UMI_STATUS_INVALID_STATE;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_release_init(&release, &candidate->manifest, generation) != UMI_STATUS_OK) {
        return UMI_STATUS_INVALID_STATE;
    }
    release.verification = UMI_EVIDENCE_PASS;
    return umi_release_registry_add(umi_studio_delivery_releases(platform), &release);
}
