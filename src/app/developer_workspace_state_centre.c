/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/developer_workspace_state_centre.c
 *
 * PURPOSE:
 *   Implement persistent Studio developer context and workflow preferences.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_workspace_state_centre.h"

#include <stdlib.h>
#include <string.h>

struct UmiStudioDeveloperWorkspaceStateCentre {
    UmiDeveloperRuntime *runtime;
    UmiDeveloperWorkspaceStateSnapshot state;
    uint64_t revision;
};

/* Provide the copy text operation used by this module and its client applications. */
static UmiStatus copy_text(char *destination, size_t capacity, const char *source)
{
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U) return UMI_STATUS_INVALID_ARGUMENT;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source == NULL) source = "";
    length = strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length >= capacity) return UMI_STATUS_CAPACITY_EXCEEDED;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length > 0U) memcpy(destination, source, length);
    destination[length] = '\0';
    return UMI_STATUS_OK;
}

/* Provide the copy display text operation used by this module and its client applications. */
static void copy_display_text(char *destination, size_t capacity, const char *source)
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

/*
 * Initialise studio developer workspace state centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_developer_workspace_state_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperWorkspaceStateCentre **out_centre)
{
    UmiStudioDeveloperWorkspaceStateCentre *centre;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    centre = (UmiStudioDeveloperWorkspaceStateCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->runtime = runtime;
    centre->revision = 1U;
    umi_developer_workspace_state_init(&centre->state);
    {
        UmiStatus status =
            umi_studio_developer_workspace_state_centre_capture_context(centre);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            free(centre);
            return status;
        }
    }
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio developer workspace state centre so the same
 * storage can be reused safely.
 */
void umi_studio_developer_workspace_state_centre_destroy(
    UmiStudioDeveloperWorkspaceStateCentre *centre)
{
    free(centre);
}

/*
 * Provide the studio developer workspace state centre snapshot operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_snapshot(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    UmiStudioDeveloperWorkspaceStateCentreSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = 1U;
    copy_display_text(out_snapshot->area_id, sizeof(out_snapshot->area_id),
                      "studio.developer-workspace-state");
    copy_display_text(out_snapshot->title, sizeof(out_snapshot->title),
                      "Developer Workspace State");
    copy_display_text(out_snapshot->summary, sizeof(out_snapshot->summary),
                      "Persistent developer context, last task, launch profile and workflow preferences.");
    out_snapshot->state = centre->state;
    out_snapshot->revision = centre->revision + centre->state.revision;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio developer workspace state centre capture context operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_capture_context(
    UmiStudioDeveloperWorkspaceStateCentre *centre)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_developer_context_snapshot(
        umi_developer_runtime_context(centre->runtime), &centre->state.context);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->state.revision += 1U;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio developer workspace state centre set preferences operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_set_preferences(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *last_task_id,
    const char *last_launch_profile_id,
    int configure_before_build,
    int build_before_run,
    int test_after_build)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = copy_text(centre->state.last_task_id,
                       sizeof(centre->state.last_task_id), last_task_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = copy_text(centre->state.last_launch_profile_id,
                       sizeof(centre->state.last_launch_profile_id),
                       last_launch_profile_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    centre->state.configure_before_build = configure_before_build != 0;
    centre->state.build_before_run = build_before_run != 0;
    centre->state.test_after_build = test_after_build != 0;
    centre->state.revision += 1U;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Write studio developer workspace state centre in its stable representation and report
 * capacity or input failures to the caller.
 */
UmiStatus umi_studio_developer_workspace_state_centre_save(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *path)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || path == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_developer_workspace_state_centre_capture_context(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_developer_workspace_state_save(path, &centre->state);
}

/*
 * Read studio developer workspace state centre into validated module state and return a
 * status when input cannot be used.
 */
UmiStatus umi_studio_developer_workspace_state_centre_load(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *path)
{
    UmiDeveloperWorkspaceStateSnapshot loaded;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || path == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_developer_workspace_state_load(path, &loaded);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_developer_runtime_set_context(centre->runtime, &loaded.context);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    centre->state = loaded;
    centre->state.revision += 1U;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio developer workspace state centre runtime operation used by this
 * module and its client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_workspace_state_centre_runtime(
    UmiStudioDeveloperWorkspaceStateCentre *centre)
{
    return centre != NULL ? centre->runtime : NULL;
}
