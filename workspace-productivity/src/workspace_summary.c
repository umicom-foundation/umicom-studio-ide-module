/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/workspace_summary.c
 *
 * PURPOSE:
 *   Summarise Studio open-editor and recent-workspace state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#include "umicom/studio/workspace_summary.h"

#include <string.h>

/*
 * Provide the studio workspace summary build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_summary_build(
    const UmiStudioDocumentSet *documents,
    const UmiStudioRecentWorkspaceList *recent,
    UmiStudioWorkspaceSummary *summary)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL || recent == NULL || summary == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(summary, 0, sizeof(*summary));
    summary->open_documents = documents->count;
    summary->dirty_documents = umi_studio_document_set_dirty_count(documents);
    summary->recent_workspaces = recent->count;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < documents->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (documents->documents[index].pinned) {
            ++summary->pinned_documents;
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (documents->documents[index].preview) {
            ++summary->preview_documents;
        }
    }

    return UMI_STATUS_OK;
}
