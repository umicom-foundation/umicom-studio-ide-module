/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/document_baseline.h
 *
 * PURPOSE:
 *   Capture complete source identities, text and selections across workspace-only commands.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_TEST_DOCUMENT_BASELINE_H
#define UMICOM_STUDIO_TEST_DOCUMENT_BASELINE_H
#include "umicom/document/coordinator.h"
#include "umicom/ui/document_view.h"
#include <stdlib.h>
#include <string.h>

typedef struct StudioTestDocumentEntry {
    UmiDocumentId id;
    char view_id[UMI_UI_ID_CAPACITY];
    char document_id[UMI_UI_ID_CAPACITY];
    char *text;
    size_t length, cursor, selection;
    int dirty;
} StudioTestDocumentEntry;
typedef struct StudioTestDocumentBaseline {
    StudioTestDocumentEntry *entries;
    size_t count;
} StudioTestDocumentBaseline;

/* Each successful copy belongs to this baseline, including an empty draft.
 * Cleanup also accepts a partially captured set after an allocation failure. */
static void StudioTestDocumentsRelease(StudioTestDocumentBaseline *baseline)
{
    if (baseline == NULL) return;
    for (size_t i = 0U; i < baseline->count; ++i)
        UmiUiDocumentViewModelFreeText(baseline->entries[i].text);
    free(baseline->entries);
    memset(baseline, 0, sizeof *baseline);
}

/* Capture every adopted document, including contributed Welcome content. A
 * fixed count of one would confuse that real document with an accidental edit.
 * Call once on a zero-initialized baseline; preserve it until after assertions. */
static UmiStatus StudioTestDocumentsCapture(UmiDocumentCoordinator *documents,
    UmiUiDocumentViewModel *views, StudioTestDocumentBaseline *baseline)
{
    if (documents == NULL || views == NULL || baseline == NULL || baseline->entries != NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    size_t count = umi_document_coordinator_count(documents);
    if (count != umi_ui_document_view_model_count(views) || count > UMI_UI_DOCUMENT_VIEW_MAX)
        return UMI_STATUS_INVALID_STATE;
    if (count == 0U) return UMI_STATUS_OK;
    baseline->entries = calloc(count, sizeof *baseline->entries);
    if (baseline->entries == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    baseline->count = count;
    UmiStatus status = UMI_STATUS_OK;
    for (size_t i = 0U; i < count && status == UMI_STATUS_OK; ++i) {
        UmiDocumentWorkingCopySnapshot copy;
        UmiUiDocumentViewSnapshot view;
        status = umi_document_coordinator_at(documents, i, &copy);
        if (status == UMI_STATUS_OK)
            status = umi_ui_document_view_model_find(views, copy.view_id, &view);
        if (status != UMI_STATUS_OK) break;
        StudioTestDocumentEntry *entry = &baseline->entries[i];
        entry->id = copy.document_id;
        strcpy(entry->view_id, view.view_id);
        strcpy(entry->document_id, view.document_id);
        entry->cursor = view.cursor_offset; entry->selection = view.selection_length;
        entry->dirty = view.dirty;
        status = UmiUiDocumentViewModelCopyText(views, view.view_id, &entry->text, &entry->length);
    }
    if (status != UMI_STATUS_OK) StudioTestDocumentsRelease(baseline);
    return status;
}

/* Layouts may change which panel owns focus. They may not replace a source
 * identity, silently add/drop a document, or change any complete draft bytes.
 * Compare by identity, not by incidental document-list order. */
/* Welcome navigation may deliberately create one additional draft; keep comparison of every prior identity and byte while separately specifying the exact permitted count. The former implementation is retained for owner review. */
#if 0
static int StudioTestDocumentsUnchanged(UmiDocumentCoordinator *documents,
    UmiUiDocumentViewModel *views, const StudioTestDocumentBaseline *baseline)
{
    if (documents == NULL || views == NULL || baseline == NULL ||
        umi_document_coordinator_count(documents) != baseline->count ||
        umi_ui_document_view_model_count(views) != baseline->count) return 0;
    for (size_t i = 0U; i < baseline->count; ++i) {
        const StudioTestDocumentEntry *entry = &baseline->entries[i];
        UmiUiDocumentViewSnapshot view;
        if (umi_ui_document_view_model_find(views, entry->view_id, &view) != UMI_STATUS_OK ||
            strcmp(view.document_id, entry->document_id) != 0 || view.dirty != entry->dirty ||
            view.cursor_offset != entry->cursor || view.selection_length != entry->selection) return 0;
        int found = 0;
        for (size_t j = 0U; j < baseline->count; ++j) {
            UmiDocumentWorkingCopySnapshot copy;
            if (umi_document_coordinator_at(documents, j, &copy) != UMI_STATUS_OK) return 0;
            if (copy.document_id == entry->id && strcmp(copy.view_id, entry->view_id) == 0)
                found = 1;
        }
        if (!found) return 0;
        char *text = NULL; size_t length = 0U;
        UmiStatus status = UmiUiDocumentViewModelCopyText(views, entry->view_id, &text, &length);
        int equal = status == UMI_STATUS_OK && length == entry->length &&
            memcmp(text, entry->text, length) == 0;
        UmiUiDocumentViewModelFreeText(text);
        if (!equal) return 0;
    }
    return 1;
}
#endif
static int StudioTestDocumentsMatchCount(UmiDocumentCoordinator *documents,
    UmiUiDocumentViewModel *views, const StudioTestDocumentBaseline *baseline, size_t additional)
{
    if (documents == NULL || views == NULL || baseline == NULL || additional > UMI_UI_DOCUMENT_VIEW_MAX ||
        baseline->count > UMI_UI_DOCUMENT_VIEW_MAX - additional ||
        umi_document_coordinator_count(documents) != baseline->count + additional ||
        umi_ui_document_view_model_count(views) != baseline->count + additional) return 0;
    for (size_t i = 0U; i < baseline->count; ++i) {
        const StudioTestDocumentEntry *entry = &baseline->entries[i];
        UmiUiDocumentViewSnapshot view;
        if (umi_ui_document_view_model_find(views, entry->view_id, &view) != UMI_STATUS_OK ||
            strcmp(view.document_id, entry->document_id) != 0 || view.dirty != entry->dirty ||
            view.cursor_offset != entry->cursor || view.selection_length != entry->selection) return 0;
        int found = 0;
        for (size_t j = 0U; j < baseline->count + additional; ++j) {
            UmiDocumentWorkingCopySnapshot copy;
            if (umi_document_coordinator_at(documents, j, &copy) != UMI_STATUS_OK) return 0;
            if (copy.document_id == entry->id && strcmp(copy.view_id, entry->view_id) == 0)
                found = 1;
        }
        if (!found) return 0;
        char *text = NULL; size_t length = 0U;
        UmiStatus status = UmiUiDocumentViewModelCopyText(views, entry->view_id, &text, &length);
        int equal = status == UMI_STATUS_OK && length == entry->length &&
            memcmp(text, entry->text, length) == 0;
        UmiUiDocumentViewModelFreeText(text);
        if (!equal) return 0;
    }
    return 1;
}

static inline int StudioTestDocumentsUnchanged(UmiDocumentCoordinator *documents,
    UmiUiDocumentViewModel *views, const StudioTestDocumentBaseline *baseline)
{
    return StudioTestDocumentsMatchCount(documents, views, baseline, 0U);
}

#endif
