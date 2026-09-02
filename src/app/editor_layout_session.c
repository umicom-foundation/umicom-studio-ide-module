/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/editor_layout_session.c
 *
 * PURPOSE:
 *   Store ordered editor-group metadata as bounded Session Store entries so
 *   each workspace can restore its own tab arrangement safely.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The count plus indexed entries avoid placing an entire large layout in one
 * session value.  Surplus entries from an older, larger layout are left alone
 * and ignored, following Umicom's non-destructive persistence policy.
 */

#include "umicom/studio/editor_layout_session.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UMI_STUDIO_EDITOR_LAYOUT_COUNT_KEY "studio.ui.editor-layout.count"
#define UMI_STUDIO_EDITOR_LAYOUT_ENTRY_KEY_PREFIX \
    "studio.ui.editor-layout.entry."

/* Provide the entry key operation used by this module and its client applications. */
static UmiStatus entry_key(size_t index,
                           char *out_key,
                           size_t capacity)
{
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_key == NULL || capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(out_key, capacity, "%s%zu",
                       UMI_STUDIO_EDITOR_LAYOUT_ENTRY_KEY_PREFIX, index);
    return written < 0 || (size_t)written >= capacity
        ? UMI_STATUS_CAPACITY_EXCEEDED
        : UMI_STATUS_OK;
}

/* Return the number of records represented by decode without changing their state. */
static UmiStatus decode_count(const char *text, size_t *out_count)
{
    char *end = NULL;
    unsigned long value;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL || out_count == NULL || text[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    errno = 0;
    value = strtoul(text, &end, 10);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (errno != 0 || end == NULL || *end != '\0' ||
        value > UMI_UI_DOCUMENT_LAYOUT_ENTRY_MAX) {
        return UMI_STATUS_PARSE_ERROR;
    }
    *out_count = (size_t)value;
    return UMI_STATUS_OK;
}

/*
 * Write studio editor layout session in its stable representation and report capacity or
 * input failures to the caller.
 */
UmiStatus umi_studio_editor_layout_session_save(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session)
{
    UmiUiDocumentLayoutSnapshot layout;
    char count_text[32];
    size_t index;
    int written;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || session == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_ui_document_layout_capture(
        umi_ui_workbench_documents(workbench), &layout);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    written = snprintf(count_text, sizeof(count_text), "%zu", layout.count);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(count_text)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    status = umi_session_store_set(
        session, UMI_STUDIO_EDITOR_LAYOUT_COUNT_KEY, count_text);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < layout.count; ++index) {
        char key[UMI_SESSION_KEY_CAPACITY];
        char value[UMI_UI_DOCUMENT_LAYOUT_ENTRY_TEXT_CAPACITY];
        status = entry_key(index, key, sizeof(key));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_ui_document_layout_entry_encode(
            &layout.entries[index], value, sizeof(value));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_session_store_set(session, key, value);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio editor layout session restore operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_editor_layout_session_restore(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session,
    UmiUiDocumentLayoutApplyResult *out_result)
{
    UmiUiDocumentLayoutSnapshot layout;
    char count_text[32];
    size_t count;
    size_t index;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || session == NULL || out_result == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(out_result, 0, sizeof(*out_result));
    status = umi_session_store_get(session,
                                   UMI_STUDIO_EDITOR_LAYOUT_COUNT_KEY,
                                   count_text,
                                   sizeof(count_text));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_NOT_FOUND) return UMI_STATUS_OK;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = decode_count(count_text, &count);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    umi_ui_document_layout_init(&layout);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < count; ++index) {
        char key[UMI_SESSION_KEY_CAPACITY];
        char value[UMI_SESSION_VALUE_CAPACITY];
        status = entry_key(index, key, sizeof(key));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_session_store_get(session, key, value, sizeof(value));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_ui_document_layout_entry_decode(
            value, &layout.entries[layout.count]);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        layout.count += 1U;
    }
    return umi_ui_document_layout_apply(
        umi_ui_workbench_documents(workbench), &layout, out_result);
}
