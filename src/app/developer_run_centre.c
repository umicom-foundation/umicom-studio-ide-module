/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/developer_run_centre.c
 *
 * PURPOSE:
 *   Implement Studio launch-profile browsing and run/debug submission.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_run_centre.h"

#include <stdlib.h>
#include <string.h>

struct UmiStudioDeveloperRunCentre {
    UmiDeveloperRuntime *runtime;
    uint64_t revision;
};

/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char *destination, size_t capacity, const char *source)
{
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U) return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source == NULL) source = "";
    length = strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length >= capacity) length = capacity - 1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length > 0U) memcpy(destination, source, length);
    destination[length] = '\0';
}

/* Provide the launch registry operation used by this module and its client applications. */
static UmiProjectLaunchProfileRegistry *launch_registry(UmiStudioDeveloperRunCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->runtime == NULL) return NULL;
    return umi_project_workspace_launch_profile(
        umi_developer_runtime_projects(centre->runtime));
}

/*
 * Initialise studio developer run centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_run_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperRunCentre **out_centre)
{
    UmiStudioDeveloperRunCentre *centre;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    centre = (UmiStudioDeveloperRunCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->runtime = runtime;
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio developer run centre so the same storage can be
 * reused safely.
 */
void umi_studio_developer_run_centre_destroy(UmiStudioDeveloperRunCentre *centre)
{
    free(centre);
}

/*
 * Provide the studio developer run centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_run_centre_snapshot(
    UmiStudioDeveloperRunCentre *centre,
    UmiStudioDeveloperRunCentreSnapshot *out_snapshot)
{
    UmiProjectLaunchProfileRegistry *registry;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    registry = launch_registry(centre);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (registry == NULL) return UMI_STATUS_INVALID_STATE;
    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = 1U;
    copy_text(out_snapshot->area_id, sizeof(out_snapshot->area_id), "studio.developer-run");
    copy_text(out_snapshot->title, sizeof(out_snapshot->title), "Run and Debug");
    copy_text(out_snapshot->summary, sizeof(out_snapshot->summary),
              "Run and debug launch profiles executed through the common developer pipeline.");
    out_snapshot->launch_profile_count = umi_project_launch_profile_registry_count(registry);
    out_snapshot->launch_profile_revision = umi_project_launch_profile_registry_revision(registry);
    out_snapshot->revision = centre->revision + out_snapshot->launch_profile_revision;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}

/*
 * Find studio developer run centre while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_developer_run_centre_at(
    UmiStudioDeveloperRunCentre *centre,
    size_t index,
    UmiProjectLaunchProfileSnapshot *out_profile)
{
    UmiProjectLaunchProfileRegistry *registry = launch_registry(centre);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (registry == NULL) return UMI_STATUS_INVALID_STATE;
    return umi_project_launch_profile_registry_at(registry, index, out_profile);
}

/*
 * Provide the studio developer run centre submit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_run_centre_submit(
    UmiStudioDeveloperRunCentre *centre,
    const UmiDeveloperLaunchPlanRequest *request,
    UmiDeveloperLaunchPlanSnapshot *out_plan)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || request == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_developer_launch_plan_submit(centre->runtime, request, out_plan);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

/*
 * Provide the studio developer run centre runtime operation used by this module and its
 * client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_run_centre_runtime(
    UmiStudioDeveloperRunCentre *centre)
{
    return centre != NULL ? centre->runtime : NULL;
}
