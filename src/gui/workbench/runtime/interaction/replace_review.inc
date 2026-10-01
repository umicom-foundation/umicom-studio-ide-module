/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/replace_review.inc
 * PURPOSE: Present Framework-owned before-and-after replacement captures.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/replacement.h"

/* The confirmation owns its plan through accept, cancel and window teardown. */
static void runtime_replacement_plan_destroy(gpointer data)
{
    UmiDocumentReplacementPlanDestroy(data);
}

/* The captured document, not the currently selected tab, receives the edit. */
static UmiStatus runtime_replacement_accepted(UmiStudioGtkRuntimeChrome *runtime, void *data)
{
    size_t count = 0U;
    UmiStatus status = UmiDocumentCoordinatorApplyReplacement(runtime_documents(runtime), data, &count);
    if (status == UMI_STATUS_OK) {
        char message[192];
        (void)snprintf(message, sizeof message,
            "Applied %zu reviewed replacement%s. Undo restores the previous draft; use Save to write the file.",
            count, count == 1U ? "" : "s");
        runtime_show_notification(runtime, message);
        (void)runtime_refresh_services(runtime);
    }
    return status;
}

/* Studio supplies presentation only. Search semantics, immutable text, stale
 * checks and the undo transaction all stay in the reusable document service. */
static void runtime_on_editor_replace_all(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentReplacementSummary summary;
    UmiDocumentReplacementPlan *plan = NULL;
    GtkWidget *review = NULL;
    (void)button;
    if (runtime == NULL || runtime->editor_find_entry == NULL || runtime->editor_replace_entry == NULL) return;
    const char *needle = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_find_entry));
    const char *replacement = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_replace_entry));
    UmiStatus status = umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active);
    if (status == UMI_STATUS_OK)
        status = UmiDocumentCoordinatorPrepareReplacement(runtime_documents(runtime), active.document_id, needle, replacement, &plan);
    if (status == UMI_STATUS_OK) status = UmiDocumentReplacementPlanSummary(plan, &summary);
    if (status == UMI_STATUS_OK && !summary.text_changes) {
        runtime_show_notification(runtime, summary.match_count == 0U
            ? "No matches in this document. The draft was not changed."
            : "The replacement produces the same text. The draft was not changed.");
        UmiDocumentReplacementPlanDestroy(plan);
        return;
    }
    if (status == UMI_STATUS_OK) {
        const char *previous = NULL, *proposed = NULL;
        size_t previousBytes = 0U, proposedBytes = 0U;
        status = UmiDocumentReplacementPlanTexts(plan, &previous, &previousBytes, &proposed, &proposedBytes);
        if (status == UMI_STATUS_OK)
            status = UmiGtk4TextComparisonCreate(previous, previousBytes, proposed, proposedBytes,
                "Current draft (captured)", "After Replace All (captured)", &review);
        if (review != NULL) g_object_ref_sink(review);
    }
    if (status == UMI_STATUS_OK) {
        char message[1024];
        (void)snprintf(message, sizeof message,
            "Review %zu replacement%s in %s.\n\nCurrent text: %zu bytes. Proposed text: %zu bytes. %s\n\n"
            "Only this captured document is affected. Cancel keeps the draft unchanged. "
            "Apply changes the draft, and Undo restores its previous text. Save writes the file separately. "
            "Changing the document while this question is open requires a new review.",
            summary.match_count, summary.match_count == 1U ? "" : "s", summary.display_name,
            summary.previous_bytes, summary.proposed_bytes,
            summary.has_unsaved_changes ? "The capture includes your unsaved changes." : "The draft has no unsaved changes.");
        status = runtime_confirm_owned(runtime, "Review Replace All", message, "Apply replacements",
            summary.document_id, runtime_replacement_accepted, plan, runtime_replacement_plan_destroy, review);
        if (status == UMI_STATUS_OK) plan = NULL;
    }
    g_clear_object(&review);
    UmiDocumentReplacementPlanDestroy(plan);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
}
