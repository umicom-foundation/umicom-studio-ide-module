/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/compatibility_platform.h
 *
 * PURPOSE:
 *   Own Studio access to Framework ABI, SDK and product-conformance services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio is the reference consumer. These adapters call public Framework ABI, SDK and product APIs rather than duplicating those mechanisms inside the IDE.
 */
#ifndef UMICOM_STUDIO_COMPATIBILITY_PLATFORM_H
#define UMICOM_STUDIO_COMPATIBILITY_PLATFORM_H
#include "umicom/umicom.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio compatibility platform data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioCompatibilityPlatform UmiStudioCompatibilityPlatform;
/**
 * Initialise studio compatibility platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_compatibility_platform_create(UmiStudioCompatibilityPlatform **out_platform);
/**
 * Release or reset state held by studio compatibility platform so the same storage can be
 * reused safely.
 */
void umi_studio_compatibility_platform_destroy(UmiStudioCompatibilityPlatform *platform);
/**
 * Return the number of records represented by studio compatibility product without
 * changing their state.
 */
size_t umi_studio_compatibility_product_count(const UmiStudioCompatibilityPlatform *platform);
/**
 * Find studio compatibility product while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiProductProfile *umi_studio_compatibility_product_at(const UmiStudioCompatibilityPlatform *platform,size_t index);
/**
 * Provide the studio compatibility summary operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_compatibility_summary(const UmiStudioCompatibilityPlatform *platform,char *buffer,size_t capacity);
#endif

