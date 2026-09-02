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
    runtime->test_results_view = gtk_text_view_new();
    runtime->test_coverage_view = gtk_text_view_new();
    runtime->test_output_view = gtk_text_view_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (notebook == NULL ||
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
