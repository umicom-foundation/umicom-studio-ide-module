/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/workspace.c
 *
 * PURPOSE:
 *   Coordinate workspace opening, project discovery, index refresh, trust and
 *   watcher lifecycle through the shared Framework service container.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace.h"

#include "umicom/studio/watcher.h"

/*
 * Provide the studio workspace open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_open(UmiStudioServices *services,
                                    const char *root,
                                    int trusted,
                                    int start_watcher)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || root == NULL || root[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_studio_services_open_workspace(services, root, trusted);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK && start_watcher) {
        status = umi_studio_watcher_start(services);
    }
    return status;
}

/*
 * Provide the studio workspace close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_close(UmiStudioServices *services)
{
    return umi_studio_services_close_workspace(services);
}

/*
 * Provide the studio workspace refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_refresh(UmiStudioServices *services)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_workspace_graph_discover(
        umi_studio_services_workspace(services));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_rebuild(
            umi_studio_services_file_index(services));
    }
    return status;
}

/*
 * Provide the studio workspace set trusted operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_set_trusted(UmiStudioServices *services,
                                           int trusted)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_workspace_graph_set_trusted(
        umi_studio_services_workspace(services), trusted);
}

/*
 * Provide the studio workspace snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_snapshot(
    UmiStudioServices *services,
    UmiStudioWorkspaceSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_workspace_graph_snapshot(
        umi_studio_services_workspace(services),
        &out_snapshot->graph);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    out_snapshot->files = umi_file_index_stats(
        umi_studio_services_file_index(services));
    out_snapshot->watcher = umi_watcher_stats(
        umi_studio_services_watcher(services));
    return UMI_STATUS_OK;
}

/*
 * Find studio workspace project while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_workspace_project_at(
    UmiStudioServices *services,
    size_t index,
    UmiWorkspaceProjectSnapshot *out_project)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_project == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_workspace_graph_project_at(
        umi_studio_services_workspace(services), index, out_project);
}
