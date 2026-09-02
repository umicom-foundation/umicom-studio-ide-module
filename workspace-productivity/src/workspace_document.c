/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/workspace_document.c
 *
 * PURPOSE:
 *   Implement path/name/language/cursor state for one open editor document.
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

#include "umicom/studio/workspace_document.h"

#include <stdio.h>
#include <string.h>

/*
 * Provide the basename from path operation used by this module and its client
 * applications.
 */
static const char *basename_from_path(const char *path)
{
    const char *slash;
    const char *backslash;
    const char *last;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path == NULL) {
        return "";
    }

    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    last = slash;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (backslash != NULL && (last == NULL || backslash > last)) {
        last = backslash;
    }

    return last != NULL ? last + 1 : path;
}

/*
 * Initialise studio workspace document from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_workspace_document_init(UmiStudioWorkspaceDocument *document)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document != NULL) {
        (void)memset(document, 0, sizeof(*document));
        document->line = 1U;
        document->column = 1U;
    }
}

/*
 * Provide the studio workspace document set path operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_workspace_document_set_path(
    UmiStudioWorkspaceDocument *document,
    const char *path)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL || path == NULL || path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(document->path, sizeof(document->path), "%s", path);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(document->path)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    written = snprintf(document->display_name,
                       sizeof(document->display_name),
                       "%s",
                       basename_from_path(path));
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(document->display_name)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    return UMI_STATUS_OK;
}

/*
 * Provide the studio workspace document set language operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_workspace_document_set_language(
    UmiStudioWorkspaceDocument *document,
    const char *language)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL || language == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(document->language,
                       sizeof(document->language),
                       "%s",
                       language);
    return written >= 0 && (size_t)written < sizeof(document->language)
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}

/*
 * Provide the studio workspace document set cursor operation used by this module and its
 * client applications.
 */
void umi_studio_workspace_document_set_cursor(
    UmiStudioWorkspaceDocument *document,
    size_t line,
    size_t column)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document == NULL) {
        return;
    }

    document->line = line == 0U ? 1U : line;
    document->column = column == 0U ? 1U : column;
}
