/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/watchers/include/watcher_integration.h
 *
 * PURPOSE:
 *   Declare the watcher integration contract shared by Framework services and
 *   thin applications.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/watchers/include/watcher_integration.h
 *
 * PURPOSE:
 *   Glue between the recursive watcher and UI/workspace components.
 *   On any FS event, we refresh the FileTree; workspace state is optional.
 *
 * API:
 *   typedef struct _UmiWatcherIntegration UmiWatcherIntegration;
 *   UmiWatcherIntegration *umi_watch_integ_new(FileTree *tree, WorkspaceState *ws);
 *   gboolean               umi_watch_integ_add(UmiWatcherIntegration *wi, const UmiPathWatch *req);
 *   void                   umi_watch_integ_free(UmiWatcherIntegration *wi);
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_WATCHER_INTEGRATION_H
#define UMICOM_WATCHER_INTEGRATION_H

#include <glib.h>
#include "watcher_recursive.h"

/**
 * Represent the file tree data shared with callers of this public contract.
 */
typedef struct _FileTree       FileTree;
/**
 * Represent the workspace state data shared with callers of this public contract.
 */
typedef struct _WorkspaceState WorkspaceState;

/**
 * Represent the watcher integration data shared with callers of this public contract.
 */
typedef struct _UmiWatcherIntegration UmiWatcherIntegration;

/**
 * Provide the watch integ new operation used by this module and its client applications.
 */
UmiWatcherIntegration *umi_watch_integ_new(FileTree *tree, WorkspaceState *ws);
/**
 * Add watch integ only after its inputs and available capacity have been checked.
 */
gboolean umi_watch_integ_add(UmiWatcherIntegration *wi, const UmiPathWatch *req);
/**
 * Provide the watch integ free operation used by this module and its client applications.
 */
void     umi_watch_integ_free(UmiWatcherIntegration *wi);

#endif /* UMICOM_WATCHER_INTEGRATION_H */
