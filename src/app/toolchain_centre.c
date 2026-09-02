/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/toolchain_centre.c
 * PURPOSE: Implement Studio composition for Framework toolchain services.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/toolchain_centre.h"

#include <stdlib.h>
#include <string.h>

struct UmiStudioToolchainCentre {
    UmiToolchainCatalogue *catalogue;
    UmiCompilationDatabase *compilation_database;
    UmiToolchainProfile selected_profile;
    UmiEnvironmentPlan environment;
    uint64_t revision;
    int has_selected_profile;
    int environment_ready;
    int has_compilation_database;
};

/*
 * Provide the select profile internal operation used by this module and its client
 * applications.
 */
static UmiStatus select_profile_internal(UmiStudioToolchainCentre *centre,
                                         const UmiToolchainProfile *profile)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || profile == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    centre->selected_profile = *profile;
    status = umi_environment_plan_from_toolchain(
        &centre->selected_profile, &centre->environment);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    centre->has_selected_profile = 1;
    centre->environment_ready = 1;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Initialise studio toolchain centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_toolchain_centre_create(
    UmiStudioToolchainCentre **out_centre)
{
    UmiStudioToolchainCentre *centre;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    centre = (UmiStudioToolchainCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_toolchain_catalogue_create(&centre->catalogue);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_compilation_database_create(&centre->compilation_database);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_toolchain_catalogue_destroy(centre->catalogue);
        free(centre);
        return status;
    }
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio toolchain centre so the same storage can be reused
 * safely.
 */
void umi_studio_toolchain_centre_destroy(UmiStudioToolchainCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return;
    umi_compilation_database_destroy(centre->compilation_database);
    umi_toolchain_catalogue_destroy(centre->catalogue);
    free(centre);
}

/*
 * Provide the studio toolchain centre discover operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_discover(
    UmiStudioToolchainCentre *centre,
    const UmiToolchainCatalogueDiscoveryRequest *request,
    UmiToolchainCatalogueDiscoverySnapshot *out_discovery)
{
    UmiToolchainCatalogueDiscoverySnapshot local;
    UmiToolchainProfile profile;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_toolchain_catalogue_discover(
        centre->catalogue, request, &local);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_toolchain_catalogue_resolve_selected_profile(
        centre->catalogue, request != NULL ? request->require_c23 : 0,
        &profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = select_profile_internal(centre, &profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_discovery != NULL) *out_discovery = local;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio toolchain centre select profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_select_profile(
    UmiStudioToolchainCentre *centre, const char *profile_id)
{
    UmiToolchainProfile profile;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_toolchain_catalogue_select_profile(
        centre->catalogue, profile_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_toolchain_catalogue_find_profile(
        centre->catalogue, profile_id, &profile);
    return status == UMI_STATUS_OK
        ? select_profile_internal(centre, &profile) : status;
}

/*
 * Provide the studio toolchain centre bind project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_bind_project(
    UmiStudioToolchainCentre *centre,
    const UmiToolchainProjectBindingSnapshot *binding)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_toolchain_catalogue_bind_project(centre->catalogue, binding);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

/*
 * Provide the studio toolchain centre select project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_select_project(
    UmiStudioToolchainCentre *centre, const char *project_id, int require_c23)
{
    UmiToolchainProfile profile;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_toolchain_catalogue_resolve_project_profile(
        centre->catalogue, project_id, require_c23, &profile);
    return status == UMI_STATUS_OK
        ? select_profile_internal(centre, &profile) : status;
}

/*
 * Provide the studio toolchain centre load compilation database operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_toolchain_centre_load_compilation_database(
    UmiStudioToolchainCentre *centre, const char *path)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_compilation_database_load(centre->compilation_database, path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_compilation_database = 1;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio toolchain centre discover compilation database operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_toolchain_centre_discover_compilation_database(
    UmiStudioToolchainCentre *centre,
    const char *project_root,
    const char *const *candidate_build_directories,
    size_t candidate_build_directory_count,
    char *out_path,
    size_t capacity)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_compilation_database_discover(
        project_root, candidate_build_directories,
        candidate_build_directory_count, out_path, capacity);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_studio_toolchain_centre_load_compilation_database(
        centre, out_path);
}

/*
 * Provide the studio toolchain centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_toolchain_centre_snapshot(
    UmiStudioToolchainCentre *centre,
    UmiStudioToolchainCentreSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = UMI_STUDIO_TOOLCHAIN_CENTRE_API_VERSION;
    out_snapshot->revision = centre->revision;
    status = umi_toolchain_catalogue_snapshot(
        centre->catalogue, &out_snapshot->catalogue);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->has_selected_profile) {
        status = umi_toolchain_capability_snapshot(
            &centre->selected_profile, &out_snapshot->selected_capability);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_toolchain_sdk_discover(
            &centre->selected_profile, &out_snapshot->selected_sdks);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        out_snapshot->has_selected_profile = 1;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->has_compilation_database) {
        status = umi_compilation_database_snapshot(
            centre->compilation_database, &out_snapshot->compilation_database);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        out_snapshot->has_compilation_database = 1;
    }
    out_snapshot->environment_variable_count = centre->environment.count;
    out_snapshot->environment_ready = centre->environment_ready;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio toolchain centre selected profile operation used by this module and
 * its client applications.
 */
const UmiToolchainProfile *umi_studio_toolchain_centre_selected_profile(
    const UmiStudioToolchainCentre *centre)
{
    return centre != NULL && centre->has_selected_profile
        ? &centre->selected_profile : NULL;
}

/*
 * Provide the studio toolchain centre environment operation used by this module and its
 * client applications.
 */
const UmiEnvironmentPlan *umi_studio_toolchain_centre_environment(
    const UmiStudioToolchainCentre *centre)
{
    return centre != NULL && centre->environment_ready
        ? &centre->environment : NULL;
}

/*
 * Provide the studio toolchain centre catalogue operation used by this module and its
 * client applications.
 */
UmiToolchainCatalogue *umi_studio_toolchain_centre_catalogue(
    UmiStudioToolchainCentre *centre)
{
    return centre != NULL ? centre->catalogue : NULL;
}
