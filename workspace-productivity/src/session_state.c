/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/session_state.c
 *
 * PURPOSE:
 *   Serialize and restore a stable text subset of workspace/document session state.
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

#include "umicom/studio/session_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Initialise studio session state from caller-provided values so later operations receive
 * a known state.
 */
void umi_studio_session_state_init(UmiStudioSessionState *state)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state != NULL) {
        (void)memset(state, 0, sizeof(*state));
        umi_studio_document_set_init(&state->documents);
    }
}

/*
 * Provide the studio session state set workspace operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_state_set_workspace(
    UmiStudioSessionState *state,
    const char *workspace_path)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state == NULL || workspace_path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(state->workspace_path,
                       sizeof(state->workspace_path),
                       "%s",
                       workspace_path);
    return written >= 0 && (size_t)written < sizeof(state->workspace_path)
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}

/*
 * Write studio session state in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_session_state_encode(
    const UmiStudioSessionState *state,
    char *output,
    size_t output_capacity)
{
    size_t used = 0U;
    size_t index;
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state == NULL || output == NULL || output_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(output,
                       output_capacity,
                       "workspace=%s\n",
                       state->workspace_path);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= output_capacity) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    used = (size_t)written;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < state->documents.count; ++index) {
        const UmiStudioWorkspaceDocument *document =
            &state->documents.documents[index];

        written = snprintf(
            output + used,
            output_capacity - used,
            "doc|%s|%s|%zu|%zu|%u|%u|%u|%u\n",
            document->path,
            document->language,
            document->line,
            document->column,
            document->dirty ? 1U : 0U,
            document->pinned ? 1U : 0U,
            document->preview ? 1U : 0U,
            document->active ? 1U : 0U
        );

        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (written < 0 || (size_t)written >= output_capacity - used) {
            return UMI_STATUS_CAPACITY_EXCEEDED;
        }

        used += (size_t)written;
    }

    return UMI_STATUS_OK;
}

/* Provide the parse unsigned operation used by this module and its client applications. */
static unsigned parse_unsigned(const char *text)
{
    return text != NULL ? (unsigned)strtoul(text, NULL, 10) : 0U;
}

/*
 * Provide the parse document line operation used by this module and its client
 * applications.
 */
static UmiStatus parse_document_line(char *line, UmiStudioSessionState *state)
{
    char *fields[9];
    size_t count = 0U;
    char *cursor = line;

    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (count < 9U) {
        fields[count++] = cursor;
        cursor = strchr(cursor, '|');
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (cursor == NULL) {
            break;
        }
        *cursor = '\0';
        ++cursor;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (count != 9U || strcmp(fields[0], "doc") != 0) {
        return UMI_STATUS_PARSE_ERROR;
    }

    {
        UmiStatus status = umi_studio_document_set_open(
            &state->documents,
            fields[1],
            fields[2],
            parse_unsigned(fields[7]) != 0U
        );
        UmiStudioWorkspaceDocument *document;

        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            return status;
        }

        document = umi_studio_document_set_find(&state->documents, fields[1]);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (document == NULL) {
            return UMI_STATUS_INTERNAL_ERROR;
        }

        document->line = (size_t)strtoull(fields[3], NULL, 10);
        document->column = (size_t)strtoull(fields[4], NULL, 10);
        document->dirty = parse_unsigned(fields[5]) != 0U;
        document->pinned = parse_unsigned(fields[6]) != 0U;
        document->preview = parse_unsigned(fields[7]) != 0U;
        document->active = parse_unsigned(fields[8]) != 0U;
    }

    return UMI_STATUS_OK;
}

/*
 * Read studio session state into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_session_state_decode(
    const char *text,
    UmiStudioSessionState *state)
{
    char buffer[UMI_STUDIO_SESSION_TEXT_CAPACITY];
    char *cursor;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL || state == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (strlen(text) >= sizeof(buffer)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)snprintf(buffer, sizeof(buffer), "%s", text);
    umi_studio_session_state_init(state);

    cursor = buffer;
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (*cursor != '\0') {
        char *newline = strchr(cursor, '\n');
        UmiStatus status = UMI_STATUS_OK;

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (newline != NULL) {
            *newline = '\0';
        }

        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (strncmp(cursor, "workspace=", 10U) == 0) {
            status = umi_studio_session_state_set_workspace(state,
                                                            cursor + 10U);
        } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strncmp(cursor, "doc|", 4U) == 0) {
            status = parse_document_line(cursor, state);
        } else /* Apply this branch only when its contract condition is satisfied. */ if (cursor[0] != '\0') {
            status = UMI_STATUS_PARSE_ERROR;
        }

        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            return status;
        }

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (newline == NULL) {
            break;
        }
        cursor = newline + 1U;
    }

    return UMI_STATUS_OK;
}
