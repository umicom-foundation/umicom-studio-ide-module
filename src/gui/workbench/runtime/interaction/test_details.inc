/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/test_details.inc
 *
 * PURPOSE:
 *   Project detailed Framework Test Platform result, coverage and output
 *   registries into professional Test Explorer evidence panes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_text_view_set(
    GtkWidget *view,
    const char *text)
{
    GtkTextBuffer *buffer;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (view == NULL) return;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_buffer_set_text(
        buffer,
        text != NULL ? text : "",
        -1);
}


#include "umicom/test_platform/evidence.h"

/* Framework selects and owns the captured evidence. Studio supplies only
 * text presentation, keeping the existing aggregate history panes available. */
static void RuntimeSelectedTestEvidenceRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->test_selected_view == NULL) return;
    RuntimeFailureLocationsRefresh(runtime);
    if (runtime->selected_test_id[0] == '\0') {
        runtime_text_view_set(runtime->test_selected_view,
            "Select a test to inspect its retained results and output.");
        return;
    }
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (platform == NULL) {
        runtime_text_view_set(runtime->test_selected_view, "Test evidence is unavailable.");
        return;
    }
    UmiTestEvidence *capture = NULL;
    UmiStatus status = UmiTestEvidenceCreate(umi_test_platform_service_result(platform),
        umi_test_platform_service_output(platform), runtime->selected_test_id, NULL, &capture);
    if (status != UMI_STATUS_OK) {
        runtime_text_view_set(runtime->test_selected_view, "Unable to capture selected-test evidence.");
        return;
    }
    UmiTestEvidenceSummary summary;
    (void)UmiTestEvidenceGetSummary(capture, &summary);
    GString *text = g_string_sized_new(4096U);
    g_string_append_printf(text,
        "Selected test: %s\nRetained results: %zu of %zu | output records: %zu of %zu\n"
        "Newest first. Session labels distinguish separate runs.\n\n",
        summary.item_id, summary.retained_results, summary.matching_results,
        summary.retained_output, summary.matching_output);
    if (summary.retained_results == 0U)
        g_string_append(text, "No result is retained for this test. This is not a pass.\n");
    for (size_t i = 0U; i < summary.retained_results; ++i) {
        UmiTestPlatformResultSnapshot row;
        if (UmiTestEvidenceResultAt(capture, i, &row) != UMI_STATUS_OK) break;
        g_string_append_printf(text, "[%s] %s | %.2f ms | exit %d%s\n%s\n%s\n\n",
            row.session_id[0] != '\0' ? row.session_id : "session unspecified",
            umi_test_platform_outcome_text((UmiTestPlatformOutcome)row.outcome),
            row.duration_ms, row.exit_code, row.flaky ? " | flaky" : "",
            row.message, row.failure_details);
    }
    g_string_append(text, "OUTPUT\n");
    if (summary.retained_output == 0U) g_string_append(text, "No output retained for this test.\n");
    for (size_t i = 0U; i < summary.retained_output; ++i) {
        UmiTestPlatformOutputSnapshot row;
        if (UmiTestEvidenceOutputAt(capture, i, &row) != UMI_STATUS_OK) break;
        g_string_append_printf(text, "[%s / %s] %s\n",
            row.session_id[0] != '\0' ? row.session_id : "session unspecified", row.stream, row.text);
    }
    /* Process output may contain invalid bytes; GTK needs valid UTF-8 text. */
    char *display = g_utf8_make_valid(text->str, (gssize)text->len);
    runtime_text_view_set(runtime->test_selected_view, display);
    g_free(display);
    g_string_free(text, TRUE);
    UmiTestEvidenceDestroy(capture);
}

/*
 * Provide the runtime test details refresh operation used by this module and its client
 * applications.
 */
static void runtime_test_details_refresh(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiTestPlatformService *platform;
    UmiTestPlatformResultRegistry *results;
    UmiTestPlatformCoverageRegistry *coverage;
    UmiTestPlatformOutputRegistry *output;
    GString *result_text;
    GString *coverage_text;
    GString *output_text;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    RuntimeSelectedTestEvidenceRefresh(runtime);
    platform = runtime_test_platform(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return;

    results = umi_test_platform_service_result(platform);
    coverage = umi_test_platform_service_coverage(platform);
    output = umi_test_platform_service_output(platform);

    result_text = g_string_sized_new(4096U);
    coverage_text = g_string_sized_new(2048U);
    output_text = g_string_sized_new(4096U);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (result_text == NULL ||
        coverage_text == NULL ||
        output_text == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (result_text != NULL) g_string_free(result_text, TRUE);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (coverage_text != NULL) g_string_free(coverage_text, TRUE);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (output_text != NULL) g_string_free(output_text, TRUE);
        return;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (results != NULL) {
        const size_t count =
            umi_test_platform_result_registry_count(results);
        const size_t start = count > 100U ? count - 100U : 0U;

        /* Visit each bounded item once so every record receives the same rule. */
        for (index = start; index < count; ++index) {
            UmiTestPlatformResultSnapshot item;

            (void)memset(&item, 0, sizeof(item));
            /* Preserve the original failure result so the caller can respond to the correct cause. */
            if (umi_test_platform_result_registry_at(
                    results,
                    index,
                    &item) != UMI_STATUS_OK) {
                continue;
            }

            g_string_append_printf(
                result_text,
                "%s | %s | %.2f ms | exit %d%s%s\n",
                item.item_id,
                umi_test_platform_outcome_text(
                    (UmiTestPlatformOutcome)item.outcome),
                item.duration_ms,
                item.exit_code,
                item.flaky ? " | flaky" : "",
                item.message[0] != '\0' ? " | " : "");
            /* Apply this branch only when its contract condition is satisfied. */
            if (item.message[0] != '\0') {
                g_string_append_printf(
                    result_text,
                    "  %s\n",
                    item.message);
            }
            /* Apply this branch only when its contract condition is satisfied. */
            if (item.failure_details[0] != '\0') {
                g_string_append_printf(
                    result_text,
                    "  %s\n",
                    item.failure_details);
            }
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (coverage != NULL) {
        const size_t count =
            umi_test_platform_coverage_registry_count(coverage);

        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < count && index < 100U; ++index) {
            UmiTestPlatformCoverageSnapshot item;
            double line_percent = 0.0;
            double branch_percent = 0.0;

            (void)memset(&item, 0, sizeof(item));
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_test_platform_coverage_registry_at(
                    coverage,
                    index,
                    &item) != UMI_STATUS_OK) {
                continue;
            }

            /* Apply this branch only when its contract condition is satisfied. */
            if (item.lines_total > 0U) {
                line_percent =
                    ((double)item.lines_covered /
                     (double)item.lines_total) * 100.0;
            }
            /* Apply this branch only when its contract condition is satisfied. */
            if (item.branches_total > 0U) {
                branch_percent =
                    ((double)item.branches_covered /
                     (double)item.branches_total) * 100.0;
            }

            g_string_append_printf(
                coverage_text,
                "%s | lines %llu/%llu (%.1f%%) | branches %llu/%llu (%.1f%%)\n",
                item.uri,
                (unsigned long long)item.lines_covered,
                (unsigned long long)item.lines_total,
                line_percent,
                (unsigned long long)item.branches_covered,
                (unsigned long long)item.branches_total,
                branch_percent);
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (output != NULL) {
        const size_t count =
            umi_test_platform_output_registry_count(output);
        const size_t start = count > 150U ? count - 150U : 0U;

        /* Visit each bounded item once so every record receives the same rule. */
        for (index = start; index < count; ++index) {
            UmiTestPlatformOutputSnapshot item;

            (void)memset(&item, 0, sizeof(item));
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_test_platform_output_registry_at(
                    output,
                    index,
                    &item) != UMI_STATUS_OK) {
                continue;
            }

            g_string_append_printf(
                output_text,
                "[%s] %s: %s",
                item.stream,
                item.item_id,
                item.text);
            /* Apply this branch only when its contract condition is satisfied. */
            if (output_text->len == 0U ||
                output_text->str[output_text->len - 1U] != '\n') {
                g_string_append_c(output_text, '\n');
            }
        }
    }

    runtime_text_view_set(runtime->test_results_view, result_text->str);
    runtime_text_view_set(runtime->test_coverage_view, coverage_text->str);
    runtime_text_view_set(runtime->test_output_view, output_text->str);

    g_string_free(result_text, TRUE);
    g_string_free(coverage_text, TRUE);
    g_string_free(output_text, TRUE);
}

/*
 * Provide the runtime test details notebook operation used by this module and its client
 * applications.
 */
/* Framework owns evidence capture and CSV rules. Studio only selects the
 * requested test and presents copied bytes through its existing weak-owner
 * action binding, so retained controls cannot access a destroyed workbench. */
#include "umicom/test_platform/evidence_csv.h"
static void RuntimeCopyTestEvidenceCsv(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    if (runtime->selected_test_id[0] == '\0') {
        runtime_show_notification(runtime, "Select a test before copying evidence."); return;
    }
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (platform == NULL) {
        runtime_show_notification(runtime, "Test evidence is unavailable."); return;
    }
    UmiTestEvidence *capture = NULL;
    UmiCsvDocument *document = NULL;
    UmiStatus status = UmiTestEvidenceCreate(umi_test_platform_service_result(platform),
        umi_test_platform_service_output(platform), runtime->selected_test_id, NULL, &capture);
    if (status == UMI_STATUS_OK) status = UmiTestEvidenceExportCsv(capture, &document);
    UmiTestEvidenceDestroy(capture);
    if (status == UMI_STATUS_OK) {
        gdk_clipboard_set_text(gtk_widget_get_clipboard(GTK_WIDGET(button)), UmiCsvDocumentData(document));
        runtime_show_notification(runtime, "Copied selected-test evidence CSV with retained/matching counts. No test was launched.");
    } else {
        char message[160];
        (void)snprintf(message, sizeof(message), "Test evidence was not copied: %s. The clipboard is unchanged.", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
    UmiCsvDocumentDestroy(document);
}

static GtkWidget *runtime_test_details_notebook(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *notebook;
    GtkWidget *results_scroll;
    GtkWidget *coverage_scroll;
    GtkWidget *output_scroll;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return NULL;

    notebook = gtk_notebook_new();
    runtime->test_selected_view = gtk_text_view_new();
    runtime->test_results_view = gtk_text_view_new();
    runtime->test_coverage_view = gtk_text_view_new();
    runtime->test_output_view = gtk_text_view_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (notebook == NULL ||
        runtime->test_selected_view == NULL ||
        runtime->test_results_view == NULL ||
        runtime->test_coverage_view == NULL ||
        runtime->test_output_view == NULL) {
        return notebook;
    }

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->test_results_view),
        FALSE);
    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->test_coverage_view),
        FALSE);
    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->test_output_view),
        FALSE);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->test_results_view),
        TRUE);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->test_coverage_view),
        TRUE);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->test_output_view),
        TRUE);

    results_scroll = runtime_scroll(runtime->test_results_view);
    coverage_scroll = runtime_scroll(runtime->test_coverage_view);
    output_scroll = runtime_scroll(runtime->test_output_view);

    gtk_widget_set_size_request(notebook, -1, 190);

    /* The focused evidence tab supplements Results, Coverage and Output;
     * those established aggregate views remain available beside it. */
    gtk_text_view_set_editable(GTK_TEXT_VIEW(runtime->test_selected_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(runtime->test_selected_view), TRUE);
    (void)umi_gtk4_automation_tag_widget(runtime->test_selected_view, "studio.tests.selected-evidence");
/* The selected-test page now adds a CSV action above the existing evidence view; the report remains Framework-owned. The previous implementation remains for engineering review. */
#if 0
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), runtime_scroll(runtime->test_selected_view),
        gtk_label_new("Selected test"));
#endif
    GtkWidget *selectedPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *copyCsv = gtk_button_new_with_label("Copy selected evidence CSV");
    (void)umi_gtk4_automation_tag_widget(copyCsv, "studio.tests.copy-evidence-csv");
    runtime_bind_editor_action(runtime, copyCsv, RuntimeCopyTestEvidenceCsv);
    gtk_box_append(GTK_BOX(selectedPage), copyCsv);
    gtk_box_append(GTK_BOX(selectedPage), runtime_scroll(runtime->test_selected_view));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), selectedPage, gtk_label_new("Selected test"));

    /* Framework owns parsing and captured provenance. Studio adds a bounded
     * native list beside the complete, unchanged retained evidence text. */
    GtkWidget *locationsPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    runtime->test_failure_summary = gtk_label_new("Select a test to inspect source locations.");
    runtime->test_failure_links = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_label_set_wrap(GTK_LABEL(runtime->test_failure_summary), TRUE);
    gtk_label_set_xalign(GTK_LABEL(runtime->test_failure_summary), 0.0F);
    (void)umi_gtk4_automation_tag_widget(runtime->test_failure_summary, "studio.tests.source-location-summary");
    (void)umi_gtk4_automation_tag_widget(runtime->test_failure_links, "studio.tests.source-locations");
    gtk_box_append(GTK_BOX(locationsPage), runtime->test_failure_summary);
    gtk_box_append(GTK_BOX(locationsPage), runtime_scroll(runtime->test_failure_links));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), locationsPage, gtk_label_new("Source locations"));



    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        results_scroll,
        gtk_label_new("Results"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        coverage_scroll,
        gtk_label_new("Coverage"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        output_scroll,
        gtk_label_new("Output"));

    runtime_test_details_refresh(runtime);
    return notebook;
}
