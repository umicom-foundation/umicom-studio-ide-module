/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/delivery_platform.h
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

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_PLATFORM_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_PLATFORM_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio delivery platform data shared with callers of this public contract.
 */
typedef struct UmiStudioDeliveryPlatform UmiStudioDeliveryPlatform;

/**
 * Initialise studio delivery platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_delivery_platform_create(UmiStudioDeliveryPlatform **out_platform);
/**
 * Release or reset state held by studio delivery platform so the same storage can be
 * reused safely.
 */
void umi_studio_delivery_platform_destroy(UmiStudioDeliveryPlatform *platform);

/**
 * Provide the studio delivery releases operation used by this module and its client
 * applications.
 */
UmiReleaseRegistry *umi_studio_delivery_releases(UmiStudioDeliveryPlatform *platform);
/**
 * Provide the studio delivery generations operation used by this module and its client
 * applications.
 */
UmiGenerationStore *umi_studio_delivery_generations(UmiStudioDeliveryPlatform *platform);
/**
 * Provide the studio delivery pipeline operation used by this module and its client
 * applications.
 */
UmiDeliveryPipeline *umi_studio_delivery_pipeline(UmiStudioDeliveryPlatform *platform);
/**
 * Provide the studio delivery channels operation used by this module and its client
 * applications.
 */
UmiChannelRegistry *umi_studio_delivery_channels(UmiStudioDeliveryPlatform *platform);
/**
 * Provide the studio delivery policy operation used by this module and its client
 * applications.
 */
const UmiDeliveryPolicy *umi_studio_delivery_policy(const UmiStudioDeliveryPlatform *platform);

#ifdef __cplusplus
}
#endif

#endif
