/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/recent_workspaces.c
 *
 * PURPOSE:
 *   Implement recent-workspace MRU ordering and pinning.
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

#include "umicom/studio/recent_workspaces.h"

#include <stdio.h>
#include <string.h>

/* Provide the copy text operation used by this module and its client applications. */
static UmiStatus copy_text(char *destination, size_t capacity, const char *source)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(destination, capacity, "%s", source);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}

/*
 * Initialise studio recent workspaces from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_recent_workspaces_init(UmiStudioRecentWorkspaceList *list)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (list != NULL) {
        (void)memset(list, 0, sizeof(*list));
    }
}

/* Provide the find index operation used by this module and its client applications. */
static size_t find_index(const UmiStudioRecentWorkspaceList *list,
                         const char *path)
{
    size_t index;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < list->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(list->items[index].path, path) == 0) {
            return index;
        }
    }

    return list->count;
}

/* Provide the move to front operation used by this module and its client applications. */
static void move_to_front(UmiStudioRecentWorkspaceList *list, size_t index)
{
    UmiStudioRecentWorkspace item;

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (index == 0U || index >= list->count) {
        return;
    }

    item = list->items[index];
    (void)memmove(&list->items[1],
                  &list->items[0],
                  index * sizeof(list->items[0]));
    list->items[0] = item;
}

/*
 * Provide the studio recent workspaces touch operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_recent_workspaces_touch(
    UmiStudioRecentWorkspaceList *list,
    const char *path,
    const char *name)
{
    size_t index;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (list == NULL || path == NULL || name == NULL ||
        path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    index = find_index(list, path);

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (index == list->count) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (list->count < UMI_STUDIO_RECENT_WORKSPACE_MAX) {
            ++list->count;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            index = list->count - 1U;
        }

        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (index >= list->count) {
            index = list->count - 1U;
        }

        (void)memset(&list->items[index], 0, sizeof(list->items[index]));
    }

    status = copy_text(list->items[index].path,
                       sizeof(list->items[index].path),
                       path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    status = copy_text(list->items[index].name,
                       sizeof(list->items[index].name),
                       name);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    list->items[index].last_opened_tick = ++list->tick;
    move_to_front(list, index);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio recent workspaces pin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_recent_workspaces_pin(
    UmiStudioRecentWorkspaceList *list,
    const char *path,
    bool pinned)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (list == NULL || path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    index = find_index(list, path);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (index >= list->count) {
        return UMI_STATUS_NOT_FOUND;
    }

    list->items[index].pinned = pinned;
    return UMI_STATUS_OK;
}

/*
 * Find studio recent workspaces while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioRecentWorkspace *umi_studio_recent_workspaces_at(
    const UmiStudioRecentWorkspaceList *list,
    size_t index)
{
    return list != NULL && index < list->count
        ? &list->items[index]
        : NULL;
}
