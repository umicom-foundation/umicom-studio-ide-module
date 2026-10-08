/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/test_archive_comparison.inc
 * PURPOSE: Present immutable outcome comparisons without blocking on database reads.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Show a bounded page so large test suites do not create one enormous GTK text
 * buffer. The summary always covers every row, including rows on other pages. */
static void RuntimeTestArchiveComparisonPage(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioTestService *service = RuntimeTestArchiveService(runtime);
    UmiStudioTestArchiveState state = {0};
    UmiStatus status = UmiStudioTestArchiveStateRead(service, &state);
    uint64_t page = 0;
    if (status != UMI_STATUS_OK || !state.comparison.ready)
    {
        runtime_show_notification(runtime, status != UMI_STATUS_OK ? umi_status_text(status)
                                                                   : "No completed comparison is available.");
        return;
    }
    if (!RuntimeTestArchiveNumber(runtime->test_archive_comparison_page, &page) ||
        page > (state.comparison.summary.row_count + 99U) / 100U)
    {
        runtime_show_notification(runtime, "Enter an available comparison page number starting at 1.");
        return;
    }
    const UmiTestArchiveComparisonSummary *summary = &state.comparison.summary;
    GString *text = g_string_new("Historical comparison. This does not certify the current checkout.\n");
    g_string_append_printf(
        text,
        "Baseline #%" G_GUINT64_FORMAT " -> candidate #%" G_GUINT64_FORMAT
        "\nSource before: %s\nSource after: %s\nSame recorded source root: %s"
        "\nEqual nonempty revision labels: %s (labels are not verified source fingerprints)\n",
        (guint64)summary->baseline.id, (guint64)summary->candidate.id, summary->baseline.origin.source_root,
        summary->candidate.origin.source_root, summary->same_source_root ? "yes" : "no",
        summary->same_recorded_revision ? "yes" : "no");
    for (unsigned kind = 0; kind < (unsigned)UMI_TEST_ARCHIVE_CHANGE_COUNT; ++kind)
        g_string_append_printf(text, "%s: %zu\n", UmiTestArchiveChangeText((UmiTestArchiveChange)kind),
                               summary->counts[kind]);
    size_t first = (size_t)(page - 1U) * 100U;
    size_t end = first + 100U < summary->row_count ? first + 100U : summary->row_count;
    g_string_append_printf(text, "\nPage %" G_GUINT64_FORMAT "; rows %zu-%zu of %zu\n", (guint64)page,
                           first + 1U, end, summary->row_count);
    for (size_t i = first; i < end; ++i)
    {
        UmiTestArchiveComparisonRow row = {0};
        status = UmiStudioTestArchiveComparisonRowAt(service, i, &row);
        if (status != UMI_STATUS_OK)
            break;
        const UmiCtestJobRequest *request = row.after_present ? &row.after_request : &row.before_request;
        g_string_append_printf(text, "\n%s\n%s\nBuild: %s; configuration: %s\n",
                               UmiTestArchiveChangeText(row.change), request->name, request->build_directory,
                               request->configuration[0] ? request->configuration : "(default)");
        /* Print both sides explicitly. Missing selection is different from a
         * selected test whose attempts never started after an early stop. */
        for (unsigned side = 0; side < 2U; ++side)
        {
            bool present = side == 0 ? row.before_present : row.after_present;
            const UmiTestArchiveOutcomes *out = side == 0 ? &row.before : &row.after;
            const UmiCtestJobRequest *source = side == 0 ? &row.before_request : &row.after_request;
            const char *label = side == 0 ? "Before" : "After";
            if (!present)
            {
                g_string_append_printf(text, "%s: absent from selection\n", label);
                continue;
            }
            g_string_append_printf(
                text,
                "%s: %zu/%zu recorded; %zu passed; %zu failed; %zu skipped; %zu timed out; "
                "%zu cancelled; %zu not run; %zu never started; %zu reporting errors; "
                "%" G_GUINT64_FORMAT " ms\nID: %s; enabled: %s; timeout: %u ms\n",
                label, out->recorded, out->planned, out->passed, out->failed, out->skipped, out->timed_out,
                out->cancelled, out->not_run, out->never_started, out->errors, (guint64)out->duration_ms,
                source->test_id, source->enabled ? "yes" : "no", (unsigned)source->timeout_ms);
        }
    }
    if (status == UMI_STATUS_OK)
        RuntimeTestArchiveText(runtime, text->str);
    else
        runtime_show_notification(runtime, umi_status_text(status));
    g_string_free(text, TRUE);
}
static void RuntimeTestArchiveCompare(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    uint64_t baseline = 0, candidate = 0;
    if (!RuntimeTestArchiveNumber(runtime->test_archive_baseline, &baseline) ||
        !RuntimeTestArchiveNumber(runtime->test_archive_id, &candidate))
    {
        runtime_show_notification(runtime, "Enter a baseline run ID and a candidate saved run ID.");
        return;
    }
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveCompare(
        umi_studio_services_tests(services), umi_studio_services_task_queue(services), baseline, candidate);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_comparison_pending = true;
        gtk_editable_set_text(GTK_EDITABLE(runtime->test_archive_comparison_page), "1");
    }
    runtime_show_notification(
        runtime, status == UMI_STATUS_OK
                     ? "Comparing saved outcomes. Studio remains available while the archive is read."
                     : umi_status_text(status));
}
static void RuntimeTestArchiveCompareStop(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStatus status = UmiStudioTestArchiveComparisonCancel(RuntimeTestArchiveService(data));
    runtime_show_notification(
        data, status == UMI_STATUS_OK ? "Comparison stop requested. A completed comparison remains readable."
                                      : umi_status_text(status));
}
