/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/document_set.c
 *
 * PURPOSE:
 *   Implement open-editor lifecycle, preview replacement, activation, pinning and dirty-close protection.
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

#include "umicom/studio/document_set.h"

#include <string.h>

/*
 * Initialise studio document set from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_document_set_init(UmiStudioDocumentSet *set)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set != NULL) {
        (void)memset(set, 0, sizeof(*set));
    }
}

/*
 * Find studio document set while leaving the underlying catalogue or model owned by this
 * module.
 */
UmiStudioWorkspaceDocument *umi_studio_document_set_find(
    UmiStudioDocumentSet *set,
    const char *path)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL || path == NULL) {
        return NULL;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(set->documents[index].path, path) == 0) {
            return &set->documents[index];
        }
    }

    return NULL;
}

/*
 * Provide the studio document set find const operation used by this module and its client
 * applications.
 */
const UmiStudioWorkspaceDocument *umi_studio_document_set_find_const(
    const UmiStudioDocumentSet *set,
    const char *path)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL || path == NULL) {
        return NULL;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(set->documents[index].path, path) == 0) {
            return &set->documents[index];
        }
    }

    return NULL;
}

/* Provide the activate document operation used by this module and its client applications. */
static void activate_document(UmiStudioDocumentSet *set,
                              UmiStudioWorkspaceDocument *document)
{
    size_t index;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        set->documents[index].active = false;
    }

    document->active = true;
    document->last_used_tick = ++set->usage_tick;
}

/*
 * Provide the replaceable preview operation used by this module and its client
 * applications.
 */
static UmiStudioWorkspaceDocument *replaceable_preview(UmiStudioDocumentSet *set)
{
    size_t index;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        UmiStudioWorkspaceDocument *document = &set->documents[index];
        /* Apply this branch only when its contract condition is satisfied. */
        if (document->preview && !document->pinned && !document->dirty) {
            return document;
        }
    }

    return NULL;
}

/*
 * Provide the studio document set open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_open(
    UmiStudioDocumentSet *set,
    const char *path,
    const char *language,
    bool preview)
{
    UmiStudioWorkspaceDocument *document;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL || path == NULL || language == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    document = umi_studio_document_set_find(set, path);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document != NULL) {
        activate_document(set, document);
        return UMI_STATUS_OK;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (preview) {
        document = replaceable_preview(set);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (document != NULL) {
            umi_studio_workspace_document_init(document);
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        document = NULL;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (set->count >= UMI_STUDIO_WORKSPACE_MAX_DOCUMENTS) {
            return UMI_STATUS_CAPACITY_EXCEEDED;
        }
        document = &set->documents[set->count++];
        umi_studio_workspace_document_init(document);
    }

    status = umi_studio_workspace_document_set_path(document, path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    status = umi_studio_workspace_document_set_language(document, language);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    document->preview = preview;
    activate_document(set, document);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio document set activate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_activate(
    UmiStudioDocumentSet *set,
    const char *path)
{
    UmiStudioWorkspaceDocument *document;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL || path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    document = umi_studio_document_set_find(set, path);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        return UMI_STATUS_NOT_FOUND;
    }

    activate_document(set, document);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio document set active operation used by this module and its client
 * applications.
 */
UmiStudioWorkspaceDocument *umi_studio_document_set_active(
    UmiStudioDocumentSet *set)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL) {
        return NULL;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (set->documents[index].active) {
            return &set->documents[index];
        }
    }

    return NULL;
}

/*
 * Provide the studio document set mark dirty operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_mark_dirty(
    UmiStudioDocumentSet *set,
    const char *path,
    bool dirty)
{
    UmiStudioWorkspaceDocument *document = umi_studio_document_set_find(set, path);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        return set == NULL || path == NULL
            ? UMI_STATUS_INVALID_ARGUMENT
            : UMI_STATUS_NOT_FOUND;
    }

    document->dirty = dirty;
    /* Apply this branch only when its contract condition is satisfied. */
    if (dirty) {
        document->preview = false;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio document set pin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_pin(
    UmiStudioDocumentSet *set,
    const char *path,
    bool pinned)
{
    UmiStudioWorkspaceDocument *document = umi_studio_document_set_find(set, path);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        return set == NULL || path == NULL
            ? UMI_STATUS_INVALID_ARGUMENT
            : UMI_STATUS_NOT_FOUND;
    }

    document->pinned = pinned;
    /* Apply this branch only when its contract condition is satisfied. */
    if (pinned) {
        document->preview = false;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio document set close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_close(
    UmiStudioDocumentSet *set,
    const char *path,
    bool force)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL || path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(set->documents[index].path, path) == 0) {
            bool was_active = set->documents[index].active;

            /* Keep the operation inside its valid bounds before reading, writing or adding data. */
            if (set->documents[index].dirty && !force) {
                return UMI_STATUS_BUSY;
            }

            /* Keep the operation inside its valid bounds before reading, writing or adding data. */
            if (index + 1U < set->count) {
                (void)memmove(&set->documents[index],
                              &set->documents[index + 1U],
                              (set->count - index - 1U) *
                                  sizeof(set->documents[0]));
            }
            --set->count;

            /* Keep the operation inside its valid bounds before reading, writing or adding data. */
            if (was_active && set->count > 0U) {
                set->documents[set->count - 1U].active = true;
                set->documents[set->count - 1U].last_used_tick =
                    ++set->usage_tick;
            }

            return UMI_STATUS_OK;
        }
    }

    return UMI_STATUS_NOT_FOUND;
}

/*
 * Return the number of records represented by studio document set dirty without changing
 * their state.
 */
size_t umi_studio_document_set_dirty_count(const UmiStudioDocumentSet *set)
{
    size_t count = 0U;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (set == NULL) {
        return 0U;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (set->documents[index].dirty) {
            ++count;
        }
    }

    return count;
}
