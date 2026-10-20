/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace.h
 *
 * PURPOSE:
 *   Define Studio workspace use cases over the authoritative Framework
 *   workspace graph, file index and watcher services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_H
#define UMICOM_STUDIO_WORKSPACE_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio workspace snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioWorkspaceSnapshot {
    UmiWorkspaceGraphSnapshot graph;
    UmiFileIndexStats files;
    UmiWatcherStats watcher;
} UmiStudioWorkspaceSnapshot;

/**
 * Provide the studio workspace open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_open(UmiStudioServices *services,
                                    const char *root,
                                    int trusted,
                                    int start_watcher);
/**
 * Provide the studio workspace close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_close(UmiStudioServices *services);

/** Open a project for interactive editing and start its file index in the
 * Framework worker. An older active index is cancelled and BUSY is returned
 * before changing project identity; retry after its completion. On OK the
 * project is open, while outIndexStatus reports scan admission only. A scan
 * failure does not close an otherwise usable project. Read subsequent progress
 * with UmiFileIndexRefreshRead. The caller remains responsible for trust. */
UmiStatus UmiStudioWorkspaceOpenForEditing(UmiStudioServices *services,
    const char *root, int trusted, int startWatcher, UmiStatus *outIndexStatus);

/**
 * Provide the studio workspace refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_refresh(UmiStudioServices *services);

/** Refresh immediate project discovery and queue recursive file indexing.
 * OK reports admission, not scan completion; observe UmiFileIndexRefreshRead.
 * BUSY preserves the active scan. Closing/replacing the workspace invalidates
 * late worker results. Call on the Studio service owner thread. */
UmiStatus UmiStudioWorkspaceRefreshForEditing(UmiStudioServices *services);
/**
 * Provide the studio workspace set trusted operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_set_trusted(UmiStudioServices *services,
                                           int trusted);
/**
 * Provide the studio workspace snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_snapshot(
    UmiStudioServices *services,
    UmiStudioWorkspaceSnapshot *out_snapshot
);
/**
 * Find studio workspace project while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_workspace_project_at(
    UmiStudioServices *services,
    size_t index,
    UmiWorkspaceProjectSnapshot *out_project
);

#ifdef __cplusplus
}
#endif

#endif
