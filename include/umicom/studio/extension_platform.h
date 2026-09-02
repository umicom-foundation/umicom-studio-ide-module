/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/extension_platform.h
 *
 * PURPOSE:
 *   Compose the Framework Extension SDK v2 manager for Studio without moving
 *   reusable policy, catalogue, dependency or package logic into the product.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *----------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_EXTENSION_PLATFORM_H
#define UMICOM_STUDIO_EXTENSION_PLATFORM_H

#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio extension platform data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioExtensionPlatform UmiStudioExtensionPlatform;

/**
 * Represent the studio extension platform snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioExtensionPlatformSnapshot {
    uint64_t revision;
    size_t installed;
    size_t enabled;
    size_t active;
    size_t failed;
    size_t contributions;
    size_t extension_points;
    size_t catalogue_entries;
    size_t pending_updates;
    size_t audit_events;
    size_t permission_grants;
    int native_extensions_allowed;
    UmiPluginIsolationMode default_untrusted_isolation;
} UmiStudioExtensionPlatformSnapshot;

/**
 * Initialise studio extension platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_extension_platform_create(
    UmiPluginHost *host,
    UmiStudioExtensionPlatform **out_platform);
/**
 * Release or reset state held by studio extension platform so the same storage can be
 * reused safely.
 */
void umi_studio_extension_platform_destroy(UmiStudioExtensionPlatform *platform);
/**
 * Provide the studio extension platform manager operation used by this module and its
 * client applications.
 */
UmiPluginManager *umi_studio_extension_platform_manager(
    UmiStudioExtensionPlatform *platform);
/**
 * Provide the studio extension platform snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_extension_platform_snapshot(
    const UmiStudioExtensionPlatform *platform,
    UmiStudioExtensionPlatformSnapshot *out_snapshot);
/**
 * Provide the studio extension platform set enabled operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_extension_platform_set_enabled(
    UmiStudioExtensionPlatform *platform,
    const char *plugin_id,
    int enabled,
    uint64_t timestamp_ms);

#ifdef __cplusplus
}
#endif

#endif
