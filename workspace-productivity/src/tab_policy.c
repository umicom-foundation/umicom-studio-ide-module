/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/tab_policy.c
 *
 * PURPOSE:
 *   Implement editor-tab dirty-close policy and compact tab labels.
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

#include "umicom/studio/tab_policy.h"

#include <stdio.h>

/*
 * Provide the studio tab close decision operation used by this module and its client
 * applications.
 */
UmiStudioTabCloseDecision umi_studio_tab_close_decision(
    const UmiStudioWorkspaceDocument *document)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        return UMI_STUDIO_TAB_CLOSE_NOT_FOUND;
    }

    return document->dirty
        ? UMI_STUDIO_TAB_CLOSE_CONFIRM_DIRTY
        : UMI_STUDIO_TAB_CLOSE_ALLOWED;
}

/* Provide the studio tab label operation used by this module and its client applications. */
UmiStatus umi_studio_tab_label(
    const UmiStudioWorkspaceDocument *document,
    char *output,
    size_t output_capacity)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL || output == NULL || output_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(
        output,
        output_capacity,
        "%s%s%s",
        document->pinned ? "[P] " : "",
        document->display_name,
        document->dirty ? " *" : ""
    );

    return written >= 0 && (size_t)written < output_capacity
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}
