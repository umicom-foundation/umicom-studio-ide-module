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
#include "umicom/terminal_ui/execution.h"
#include "umicom/build/project_profile.h"

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
        /* Opening and watching form one user-visible operation. If the watcher
         * cannot start, close the services again so later commands do not see
         * a workspace that the open command reported as failed. */
        if (status != UMI_STATUS_OK) {
            (void)umi_studio_services_close_workspace(services);
        }
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
/* Revoking workspace trust now stops its pending terminal command. The earlier trust-only update is retained for review. The previous implementation is retained for engineering review. */
#if 0
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
#endif
UmiStatus umi_studio_workspace_set_trusted(UmiStudioServices *services,
                                           int trusted)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Revocation requests cancellation immediately. It does not claim to undo
     * effects already performed by the command; final status arrives through Poll. */
    if (!trusted && UmiTerminalControllerJobPending(
            umi_studio_services_terminal_controller(services)))
        (void)UmiTerminalControllerStopJob(umi_studio_services_terminal_controller(services));
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

/* File enumeration belongs to Framework's bounded worker, not GTK's owner
 * thread. Keep the established synchronous API above for non-interactive users. */
UmiStatus UmiStudioWorkspaceOpenForEditing(UmiStudioServices *services,
    const char *root, int trusted, int startWatcher, UmiStatus *outIndexStatus)
{
    if (services == NULL || root == NULL || root[0] == '\0' || outIndexStatus == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    *outIndexStatus = UMI_STATUS_UNAVAILABLE;
    UmiFileIndex *index = umi_studio_services_file_index(services);
    UmiFileIndexRefreshSnapshot refresh;
    UmiStatus status = UmiFileIndexRefreshRead(index, &refresh);
    if (status != UMI_STATUS_OK) return status;
    if (refresh.active) {
        /* Cancellation is nonblocking. Do not replace the old project while
         * its retained index task still owns the worker slot. */
        (void)UmiFileIndexRefreshCancel(index);
        return UMI_STATUS_BUSY;
    }
    /* Workers require an absolute root so changing the process directory later
     * cannot redirect enumeration. Use the same canonical profile resolver. */
    UmiBuildProfile profile;
    status = UmiBuildProfileForWorkspace(root, &profile);
    if (status != UMI_STATUS_OK) return status;
    status = UmiStudioServicesPrepareWorkspace(services, profile.source_directory, trusted);
    if (status != UMI_STATUS_OK) return status;
    if (startWatcher) {
        status = umi_studio_watcher_start(services);
        if (status != UMI_STATUS_OK) {
            (void)umi_studio_services_close_workspace(services);
            return status;
        }
    }
    UmiFileIndexStats files = umi_file_index_stats(index);
    *outIndexStatus = UmiFileIndexRefreshStart(index, files.revision);
    return UMI_STATUS_OK;
}

/* Explicit workspace refresh uses Framework's existing scan worker just like
 * Explorer. Only immediate child-project discovery happens on the owner thread;
 * recursive file enumeration must not stop keyboard input in the editor. */
UmiStatus UmiStudioWorkspaceRefreshForEditing(UmiStudioServices *services)
{
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiWorkspaceGraph *graph = umi_studio_services_workspace(services);
    UmiFileIndex *index = umi_studio_services_file_index(services);
    UmiWorkspaceGraphSnapshot workspace;
    UmiStatus status = umi_workspace_graph_snapshot(graph, &workspace);
    if (status != UMI_STATUS_OK) return status;
    if (!workspace.open) return UMI_STATUS_INVALID_STATE;
    UmiFileIndexStats files = umi_file_index_stats(index);
    if (!umi_path_equal(files.root, workspace.root)) return UMI_STATUS_INVALID_STATE;
    UmiFileIndexRefreshSnapshot refresh;
    status = UmiFileIndexRefreshRead(index, &refresh);
    if (status != UMI_STATUS_OK) return status;
    if (refresh.active) return UMI_STATUS_BUSY;
    status = umi_workspace_graph_discover(graph);
    if (status != UMI_STATUS_OK) return status;
    return UmiFileIndexRefreshStart(index, files.revision);
}
