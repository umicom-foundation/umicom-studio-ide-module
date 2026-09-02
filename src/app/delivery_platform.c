/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/delivery_platform.c
 *
 * PURPOSE:
 *   Own Studio's shared Framework delivery, packaging, release and update composition.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes Framework delivery services here; packaging mechanics remain reusable inside Framework.
 */

#include "umicom/studio/delivery_platform.h"
#include <stdlib.h>

struct UmiStudioDeliveryPlatform {
    UmiReleaseRegistry releases;
    UmiGenerationStore generations;
    UmiDeliveryPipeline pipeline;
    UmiChannelRegistry channels;
    UmiDeliveryPolicy policy;
};

/*
 * Initialise studio delivery platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_delivery_platform_create(UmiStudioDeliveryPlatform **out_platform)
{
    UmiStudioDeliveryPlatform *platform;
    UmiReleaseGate gate;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_platform == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_platform = NULL;
    platform = (UmiStudioDeliveryPlatform *)calloc(1U, sizeof(*platform));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    umi_release_registry_init(&platform->releases);
    umi_generation_store_init(&platform->generations);
    umi_channel_registry_init(&platform->channels);
    platform->policy = umi_delivery_policy_default();
    (void)umi_channel_registry_add(&platform->channels,
                                   "development",
                                   umi_release_channel_policy(UMI_RELEASE_DEVELOPMENT));
    (void)umi_channel_registry_add(&platform->channels,
                                   "beta",
                                   umi_release_channel_policy(UMI_RELEASE_BETA));
    (void)umi_channel_registry_add(&platform->channels,
                                   "stable",
                                   umi_release_channel_policy(UMI_RELEASE_STABLE));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_delivery_pipeline_init(&platform->pipeline, "studio.release") != UMI_STATUS_OK) {
        free(platform);
        return UMI_STATUS_INTERNAL_ERROR;
    }
    (void)umi_release_gate_init(&gate, "build", 1);
    (void)umi_delivery_pipeline_add_gate(&platform->pipeline, &gate);
    (void)umi_release_gate_init(&gate, "tests", 1);
    (void)umi_delivery_pipeline_add_gate(&platform->pipeline, &gate);
    (void)umi_release_gate_init(&gate, "package", 1);
    (void)umi_delivery_pipeline_add_gate(&platform->pipeline, &gate);
    (void)umi_release_gate_init(&gate, "verification", 1);
    (void)umi_delivery_pipeline_add_gate(&platform->pipeline, &gate);

    *out_platform = platform;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio delivery platform so the same storage can be
 * reused safely.
 */
void umi_studio_delivery_platform_destroy(UmiStudioDeliveryPlatform *platform)
{
    free(platform);
}

/*
 * Provide the studio delivery releases operation used by this module and its client
 * applications.
 */
UmiReleaseRegistry *umi_studio_delivery_releases(UmiStudioDeliveryPlatform *platform)
{
    return platform != NULL ? &platform->releases : NULL;
}
/*
 * Provide the studio delivery generations operation used by this module and its client
 * applications.
 */
UmiGenerationStore *umi_studio_delivery_generations(UmiStudioDeliveryPlatform *platform)
{
    return platform != NULL ? &platform->generations : NULL;
}
/*
 * Provide the studio delivery pipeline operation used by this module and its client
 * applications.
 */
UmiDeliveryPipeline *umi_studio_delivery_pipeline(UmiStudioDeliveryPlatform *platform)
{
    return platform != NULL ? &platform->pipeline : NULL;
}
/*
 * Provide the studio delivery channels operation used by this module and its client
 * applications.
 */
UmiChannelRegistry *umi_studio_delivery_channels(UmiStudioDeliveryPlatform *platform)
{
    return platform != NULL ? &platform->channels : NULL;
}
/*
 * Provide the studio delivery policy operation used by this module and its client
 * applications.
 */
const UmiDeliveryPolicy *umi_studio_delivery_policy(const UmiStudioDeliveryPlatform *platform)
{
    return platform != NULL ? &platform->policy : NULL;
}
