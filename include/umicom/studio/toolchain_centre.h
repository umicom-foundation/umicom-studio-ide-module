/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/toolchain_centre.h
 *
 * PURPOSE:
 *   Compose Framework-owned toolchain discovery, capabilities, SDK metadata,
 *   child environments, compilation databases and per-project compiler choices
 *   into one Studio-facing centre without duplicating reusable mechanisms.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TOOLCHAIN_CENTRE_H
#define UMICOM_STUDIO_TOOLCHAIN_CENTRE_H

#include <stdint.h>

#include "umicom/toolchain/toolchain.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_TOOLCHAIN_CENTRE_API_VERSION 1U

/**
 * Represent the studio toolchain centre data shared with callers of this public contract.
 */
typedef struct UmiStudioToolchainCentre UmiStudioToolchainCentre;

/**
 * Represent the studio toolchain centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioToolchainCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    uint64_t revision;
    UmiToolchainCatalogueSnapshot catalogue;
    UmiToolchainCapabilitySnapshot selected_capability;
    UmiSdkCatalogueSnapshot selected_sdks;
    UmiCompilationDatabaseSnapshot compilation_database;
    size_t environment_variable_count;
    int has_selected_profile;
    int environment_ready;
    int has_compilation_database;
    int available;
} UmiStudioToolchainCentreSnapshot;

/**
 * Initialise studio toolchain centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_toolchain_centre_create(
    UmiStudioToolchainCentre **out_centre);
/**
 * Release or reset state held by studio toolchain centre so the same storage can be reused
 * safely.
 */
void umi_studio_toolchain_centre_destroy(UmiStudioToolchainCentre *centre);
/**
 * Provide the studio toolchain centre discover operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_discover(
    UmiStudioToolchainCentre *centre,
    const UmiToolchainCatalogueDiscoveryRequest *request,
    UmiToolchainCatalogueDiscoverySnapshot *out_discovery);
/**
 * Provide the studio toolchain centre select profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_select_profile(
    UmiStudioToolchainCentre *centre,
    const char *profile_id);
/**
 * Provide the studio toolchain centre bind project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_bind_project(
    UmiStudioToolchainCentre *centre,
    const UmiToolchainProjectBindingSnapshot *binding);
/**
 * Provide the studio toolchain centre select project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_select_project(
    UmiStudioToolchainCentre *centre,
    const char *project_id,
    int require_c23);
/**
 * Provide the studio toolchain centre load compilation database operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_toolchain_centre_load_compilation_database(
    UmiStudioToolchainCentre *centre,
    const char *path);
/**
 * Provide the studio toolchain centre discover compilation database operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_toolchain_centre_discover_compilation_database(
    UmiStudioToolchainCentre *centre,
    const char *project_root,
    const char *const *candidate_build_directories,
    size_t candidate_build_directory_count,
    char *out_path,
    size_t capacity);
/**
 * Provide the studio toolchain centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_snapshot(
    UmiStudioToolchainCentre *centre,
    UmiStudioToolchainCentreSnapshot *out_snapshot);
/**
 * Provide the studio toolchain centre selected profile operation used by this module and
 * its client applications.
 */
const UmiToolchainProfile *umi_studio_toolchain_centre_selected_profile(
    const UmiStudioToolchainCentre *centre);
/**
 * Provide the studio toolchain centre environment operation used by this module and its
 * client applications.
 */
const UmiEnvironmentPlan *umi_studio_toolchain_centre_environment(
    const UmiStudioToolchainCentre *centre);
/**
 * Provide the studio toolchain centre catalogue operation used by this module and its
 * client applications.
 */
UmiToolchainCatalogue *umi_studio_toolchain_centre_catalogue(
    UmiStudioToolchainCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
