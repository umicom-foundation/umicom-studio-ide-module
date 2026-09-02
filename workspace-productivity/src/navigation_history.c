/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/navigation_history.c
 *
 * PURPOSE:
 *   Implement editor navigation history with standard back/forward truncation semantics.
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

#include "umicom/studio/navigation_history.h"

#include <stdio.h>
#include <string.h>

/*
 * Initialise studio navigation history from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_navigation_history_init(UmiStudioNavigationHistory *history)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (history != NULL) {
        (void)memset(history, 0, sizeof(*history));
    }
}

/*
 * Provide the studio navigation history push operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_navigation_history_push(
    UmiStudioNavigationHistory *history,
    const char *path,
    size_t line,
    size_t column)
{
    UmiStudioNavigationLocation *location;
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (history == NULL || path == NULL || path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (history->count > 0U && history->cursor + 1U < history->count) {
        history->count = history->cursor + 1U;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (history->count == UMI_STUDIO_NAVIGATION_MAX) {
        (void)memmove(&history->items[0],
                      &history->items[1],
                      (history->count - 1U) * sizeof(history->items[0]));
        --history->count;
    }

    location = &history->items[history->count];
    (void)memset(location, 0, sizeof(*location));
    written = snprintf(location->path, sizeof(location->path), "%s", path);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(location->path)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    location->line = line == 0U ? 1U : line;
    location->column = column == 0U ? 1U : column;

    history->cursor = history->count;
    ++history->count;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio navigation history current operation used by this module and its
 * client applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_current(
    const UmiStudioNavigationHistory *history)
{
    return history != NULL && history->count > 0U &&
           history->cursor < history->count
        ? &history->items[history->cursor]
        : NULL;
}

/*
 * Provide the studio navigation history back operation used by this module and its client
 * applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_back(
    UmiStudioNavigationHistory *history)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (history == NULL || history->count == 0U || history->cursor == 0U) {
        return NULL;
    }

    --history->cursor;
    return &history->items[history->cursor];
}

/*
 * Provide the studio navigation history forward operation used by this module and its
 * client applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_forward(
    UmiStudioNavigationHistory *history)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (history == NULL || history->count == 0U ||
        history->cursor + 1U >= history->count) {
        return NULL;
    }

    ++history->cursor;
    return &history->items[history->cursor];
}
