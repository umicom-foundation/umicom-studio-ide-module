/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_workspace_state_centre.h
 *
 * PURPOSE:
 *   Manage persistent Studio developer context and workflow preferences using
 *   the reusable Framework developer workspace-state contract.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEVELOPER_WORKSPACE_STATE_CENTRE_H
#define UMICOM_STUDIO_DEVELOPER_WORKSPACE_STATE_CENTRE_H

#include <stdint.h>
#include "umicom/developer/developer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio developer workspace state centre data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioDeveloperWorkspaceStateCentre UmiStudioDeveloperWorkspaceStateCentre;

/**
 * Represent the studio developer workspace state centre snapshot data shared with callers
 * of this public contract.
 */
typedef struct UmiStudioDeveloperWorkspaceStateCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char title[256];
    char summary[512];
    UmiDeveloperWorkspaceStateSnapshot state;
    uint64_t revision;
    int available;
} UmiStudioDeveloperWorkspaceStateCentreSnapshot;

/**
 * Initialise studio developer workspace state centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_developer_workspace_state_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperWorkspaceStateCentre **out_centre);
/**
 * Release or reset state held by studio developer workspace state centre so the same
 * storage can be reused safely.
 */
void umi_studio_developer_workspace_state_centre_destroy(
    UmiStudioDeveloperWorkspaceStateCentre *centre);
/**
 * Provide the studio developer workspace state centre snapshot operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_snapshot(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    UmiStudioDeveloperWorkspaceStateCentreSnapshot *out_snapshot);
/**
 * Provide the studio developer workspace state centre capture context operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_capture_context(
    UmiStudioDeveloperWorkspaceStateCentre *centre);
/**
 * Provide the studio developer workspace state centre set preferences operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_workspace_state_centre_set_preferences(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *last_task_id,
    const char *last_launch_profile_id,
    int configure_before_build,
    int build_before_run,
    int test_after_build);
/**
 * Write studio developer workspace state centre in its stable representation and report
 * capacity or input failures to the caller.
 */
UmiStatus umi_studio_developer_workspace_state_centre_save(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *path);
/**
 * Read studio developer workspace state centre into validated module state and return a
 * status when input cannot be used.
 */
UmiStatus umi_studio_developer_workspace_state_centre_load(
    UmiStudioDeveloperWorkspaceStateCentre *centre,
    const char *path);
/**
 * Provide the studio developer workspace state centre runtime operation used by this
 * module and its client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_workspace_state_centre_runtime(
    UmiStudioDeveloperWorkspaceStateCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
