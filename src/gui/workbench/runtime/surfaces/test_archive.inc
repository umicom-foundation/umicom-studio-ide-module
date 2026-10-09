/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/test_archive.inc
 * PURPOSE: Display retained test evidence without restoring it into live test results.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/test_archive.h"
#include "umicom/build/job_history.h"

static UmiStudioTestService *RuntimeTestArchiveService(UmiStudioGtkRuntimeChrome *runtime)
{
    return umi_studio_services_tests(umi_studio_ui_services(runtime->owner->ui));
}
/* Convert only a complete decimal input. Floating-point controls cannot retain
 * every durable uint64 ID, and permissive conversion can silently select a
 * different record when the user types an invalid suffix. */
static bool RuntimeTestArchiveNumber(GtkWidget *entry, uint64_t *out)
{
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    uint64_t value = 0;
    if (text[0] == '\0')
        return false;
    for (const char *p = text; *p != '\0'; ++p)
    {
        if (*p < '0' || *p > '9')
            return false;
        unsigned digit = (unsigned)(*p - '0');
        if (value > (UINT64_MAX - digit) / 10U)
            return false;
        value = value * 10U + digit;
    }
    if (value == 0)
        return false;
    *out = value;
    return true;
}
static void RuntimeTestArchiveText(UmiStudioGtkRuntimeChrome *runtime, const char *text)
{
    /* Stored diagnostic bytes are data. Repair invalid UTF-8 only for display;
     * never interpret them as markup, commands, or instructions to the editor. */
    char *display = g_utf8_make_valid(text, -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(runtime->test_archive_entries)), display,
                             -1);
    g_free(display);
}
/* Catalogue I/O now belongs to the shared Framework reader; the UI renders copied memory on completion. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveList(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiTestArchiveCatalog *catalog = g_new0(UmiTestArchiveCatalog, 1);
    UmiStatus status = UmiStudioTestArchiveList(RuntimeTestArchiveService(runtime), catalog);
    if (status != UMI_STATUS_OK)
    {
        runtime_show_notification(runtime, umi_status_text(status));
        g_free(catalog);
        return;
    }
    /* Compare only the current in-memory profile. A missing or invalid profile
     * leaves comparison unknown instead of inventing matching source evidence. */
    UmiJobIdentity current_identity = {0};
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    const UmiBuildProfile *profile =
        umi_studio_build_service_profile(umi_studio_services_build(services));
    if (profile != NULL)
        (void)UmiBuildProfileJobIdentity(profile, &current_identity);
    GString *text =
        g_string_new("Historical evidence only. These rows do not describe the current checkout.\n");
    for (size_t i = 0; i < catalog->count; ++i)
    {
        const UmiTestArchiveEntry *entry = &catalog->entries[i];
        g_string_append_printf(
            text,
            "\n#%" G_GUINT64_FORMAT "  %zu/%zu attempts recorded\n"
            "%zu passed; %zu failed; %zu skipped; %zu cancelled; %zu timed out; %zu not run\n"
            "%zu never started; task outcome: %s\nSource: %s\nRevision: %s\nDiagnostic text: %s\n",
            (guint64)entry->id, entry->run.completed, entry->run.planned, entry->run.passed,
            entry->run.failed, entry->run.skipped, entry->run.cancelled, entry->run.timed_out,
            entry->run.not_run, entry->run.planned - entry->run.completed, umi_status_text(entry->run.status),
            entry->origin.source_root,
            entry->origin.source_revision[0] ? entry->origin.source_revision
                                             : "Not captured; source files may have changed",
            entry->origin.retain_output ? "retained bounded tail" : "not retained");
        g_string_append_printf(
            text, "Context: %s\nSelection fingerprint: %s\n",
            UmiJobIdentityComparisonText(
                UmiJobIdentityCompare(&entry->origin.identity, &current_identity)),
            entry->selection_digest[0] ? entry->selection_digest : "Not recorded");
        g_string_append(text,
            "Source files, test binaries and tool versions are not captured by this view.\n");
    }
    if (catalog->count == 0)
        g_string_append(text, "\nNo saved runs.");
    RuntimeTestArchiveText(runtime, text->str);
    g_string_free(text, TRUE);
    g_free(catalog);
}

#endif
static void RuntimeTestArchiveRenderCatalog(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiTestArchiveCatalog *catalog = g_new0(UmiTestArchiveCatalog, 1);
    UmiStatus status = UmiStudioTestArchiveCopyCatalog(RuntimeTestArchiveService(runtime), catalog);
    if (status != UMI_STATUS_OK)
    {
        runtime_show_notification(runtime, umi_status_text(status));
        g_free(catalog);
        return;
    }
    /* Compare only the current in-memory profile. A missing or invalid profile
     * leaves comparison unknown instead of inventing matching source evidence. */
    UmiJobIdentity current_identity = {0};
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    const UmiBuildProfile *profile =
        umi_studio_build_service_profile(umi_studio_services_build(services));
    if (profile != NULL)
        (void)UmiBuildProfileJobIdentity(profile, &current_identity);
    GString *text =
        g_string_new("Historical evidence only. These rows do not describe the current checkout.\n");
    for (size_t i = 0; i < catalog->count; ++i)
    {
        const UmiTestArchiveEntry *entry = &catalog->entries[i];
        g_string_append_printf(
            text,
            "\n#%" G_GUINT64_FORMAT "  %zu/%zu attempts recorded\n"
            "%zu passed; %zu failed; %zu skipped; %zu cancelled; %zu timed out; %zu not run\n"
            "%zu never started; task outcome: %s\nSource: %s\nRevision: %s\nDiagnostic text: %s\n",
            (guint64)entry->id, entry->run.completed, entry->run.planned, entry->run.passed,
            entry->run.failed, entry->run.skipped, entry->run.cancelled, entry->run.timed_out,
            entry->run.not_run, entry->run.planned - entry->run.completed, umi_status_text(entry->run.status),
            entry->origin.source_root,
            entry->origin.source_revision[0] ? entry->origin.source_revision
                                             : "Not captured; source files may have changed",
            entry->origin.retain_output ? "retained bounded tail" : "not retained");
        g_string_append_printf(
            text, "Context: %s\nSelection fingerprint: %s\n",
            UmiJobIdentityComparisonText(
                UmiJobIdentityCompare(&entry->origin.identity, &current_identity)),
            entry->selection_digest[0] ? entry->selection_digest : "Not recorded");
        g_string_append(text,
            "Source files, test binaries and tool versions are not captured by this view.\n");
    }
    if (catalog->count == 0)
        g_string_append(text, "\nNo saved runs.");
    RuntimeTestArchiveText(runtime, text->str);
    g_string_free(text, TRUE);
    g_free(catalog);
}
/* The event handler submits work only. Periodic refresh renders the copied
 * observation once it is complete, without performing a database read. */
/* A newly accepted archive action retires an older removal completion notice. The previous action binding remains for review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveBeginRead(UmiStudioGtkRuntimeChrome *runtime,
    UmiTestArchiveReadKind kind, uint64_t id, size_t index)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveBrowse(umi_studio_services_tests(services),
        umi_studio_services_task_queue(services), kind, id, index);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_read_pending = true;
        /* Only the latest accepted display request may replace the text pane. */
        runtime->test_archive_comparison_pending = false;
    }
    runtime_show_notification(runtime, status == UMI_STATUS_OK ?
        "Reading saved evidence. Studio remains available." : umi_status_text(status));
}
#endif
static void RuntimeTestArchiveBeginRead(UmiStudioGtkRuntimeChrome *runtime,
    UmiTestArchiveReadKind kind, uint64_t id, size_t index)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveBrowse(umi_studio_services_tests(services),
        umi_studio_services_task_queue(services), kind, id, index);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_remove_pending = false;
        runtime->test_archive_read_pending = true;
        /* Only the latest accepted display request may replace the text pane. */
        runtime->test_archive_comparison_pending = false;
    }
    runtime_show_notification(runtime, status == UMI_STATUS_OK ?
        "Reading saved evidence. Studio remains available." : umi_status_text(status));
}
static void RuntimeTestArchiveList(GtkButton *button, gpointer data)
{
    (void)button;
    RuntimeTestArchiveBeginRead(data, UMI_TEST_ARCHIVE_READ_CATALOG, 0, 0);
}
static void RuntimeTestArchiveStopRead(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStatus status = UmiStudioTestArchiveReadCancel(RuntimeTestArchiveService(data));
    runtime_show_notification(data, status == UMI_STATUS_OK ?
        "Read stop requested. A completed observation remains available." : umi_status_text(status));
}
/* Archive acquisition now runs in Framework so opening and catalogue validation do not block the event handler; the synchronous path remains available to headless clients. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    bool detach = g_object_get_data(G_OBJECT(button), "umicom-test-archive-detach") != NULL;
    const char *path = detach ? NULL : gtk_editable_get_text(GTK_EDITABLE(runtime->test_archive_path));
    UmiStatus status = UmiStudioTestArchiveOpen(RuntimeTestArchiveService(runtime), path);
    runtime_show_notification(runtime, status != UMI_STATUS_OK ? umi_status_text(status)
                                       : detach ? "Test archive detached. The database remains on disk."
                                                : "Private test archive opened.");
    if (status == UMI_STATUS_OK)
    {
        /* Attachment changes discard the service's old comparison. A stale UI
         * completion flag must not announce it as a new result after reopen. */
        runtime->test_archive_comparison_pending = false;
        runtime->test_archive_read_pending = false;
        if (detach)
            RuntimeTestArchiveText(runtime, "No test archive attached.");
        else
            RuntimeTestArchiveList(button, data);
    }
}

#endif
/* A newly accepted archive action retires an older removal completion notice. The previous action binding remains for review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    bool detach = g_object_get_data(G_OBJECT(button), "umicom-test-archive-detach") != NULL;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioTestService *service = umi_studio_services_tests(services);
    const char *path = gtk_editable_get_text(GTK_EDITABLE(runtime->test_archive_path));
    UmiStatus status = detach ? UmiStudioTestArchiveOpen(service, NULL) :
        UmiStudioTestArchiveOpenBegin(service, umi_studio_services_task_queue(services), path);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_open_pending = !detach;
        runtime->test_archive_comparison_pending = false;
        runtime->test_archive_read_pending = false;
        if (detach)
            RuntimeTestArchiveText(runtime, "No test archive attached.");
    }
    runtime_show_notification(runtime, status != UMI_STATUS_OK ? umi_status_text(status) :
        detach ? "Test archive detached. The database remains on disk." :
        "Opening private test archive. The current attachment is kept until it is ready.");
}
#endif
static void RuntimeTestArchiveOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    bool detach = g_object_get_data(G_OBJECT(button), "umicom-test-archive-detach") != NULL;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioTestService *service = umi_studio_services_tests(services);
    const char *path = gtk_editable_get_text(GTK_EDITABLE(runtime->test_archive_path));
    UmiStatus status = detach ? UmiStudioTestArchiveOpen(service, NULL) :
        UmiStudioTestArchiveOpenBegin(service, umi_studio_services_task_queue(services), path);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_remove_pending = false;
        runtime->test_archive_open_pending = !detach;
        runtime->test_archive_comparison_pending = false;
        runtime->test_archive_read_pending = false;
        if (detach)
            RuntimeTestArchiveText(runtime, "No test archive attached.");
    }
    runtime_show_notification(runtime, status != UMI_STATUS_OK ? umi_status_text(status) :
        detach ? "Test archive detached. The database remains on disk." :
        "Opening private test archive. The current attachment is kept until it is ready.");
}
static void RuntimeTestArchiveStopOpen(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStatus status = UmiStudioTestArchiveOpenCancel(RuntimeTestArchiveService(data));
    runtime_show_notification(data, status == UMI_STATUS_OK ?
        "Opening stop requested. Any already created database file remains on disk." :
        umi_status_text(status));
}

/* A newly accepted archive action retires an older removal completion notice. The previous action binding remains for review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveSave(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    bool retain = gtk_check_button_get_active(GTK_CHECK_BUTTON(runtime->test_archive_output));
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveSave(umi_studio_services_tests(services),
                                                umi_studio_services_task_queue(services), retain);
    runtime_show_notification(
        runtime, status == UMI_STATUS_OK
                     ? "Saving the completed run. Use Refresh saved runs after the save finishes."
                     : umi_status_text(status));
}
#endif
static void RuntimeTestArchiveSave(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    bool retain = gtk_check_button_get_active(GTK_CHECK_BUTTON(runtime->test_archive_output));
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveSave(umi_studio_services_tests(services),
                                                umi_studio_services_task_queue(services), retain);
    if (status == UMI_STATUS_OK)
        runtime->test_archive_remove_pending = false;
    runtime_show_notification(
        runtime, status == UMI_STATUS_OK
                     ? "Saving the completed run. Use Refresh saved runs after the save finishes."
                     : umi_status_text(status));
}
static void RuntimeTestArchiveStop(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStatus status = UmiStudioTestArchiveCancel(RuntimeTestArchiveService(runtime));
    runtime_show_notification(runtime, status == UMI_STATUS_OK
                                           ? "Stop requested. A save already committed remains available."
                                           : umi_status_text(status));
}
/* One worker observation now supplies metadata, selection and result together, replacing separate UI database reads. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveReadAttempt(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    uint64_t id = 0, row = 0;
    if (!RuntimeTestArchiveNumber(runtime->test_archive_id, &id) ||
        !RuntimeTestArchiveNumber(runtime->test_archive_attempt, &row) || row > UMI_CTEST_JOB_MAX_ATTEMPTS)
    {
        runtime_show_notification(runtime,
                                  "Enter a saved run ID and a completed attempt number starting at 1.");
        return;
    }
    UmiStudioTestService *service = RuntimeTestArchiveService(runtime);
    UmiTestArchiveEntry entry = {0};
    UmiTestResult *result = g_new0(UmiTestResult, 1);
    UmiCtestJobRequest request = {0};
    uint32_t pass = 0;
    UmiStatus status = UmiStudioTestArchiveRead(service, id, &entry);
    size_t index = (size_t)(row - 1U);
    if (status == UMI_STATUS_OK)
        status = UmiStudioTestArchiveResultAt(service, id, index, result, &pass);
    if (status == UMI_STATUS_OK)
        status = UmiStudioTestArchiveRequestAt(service, id, index % entry.plan.request_count, &request);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, umi_status_text(status));
    else
    {
        GString *text = g_string_new("Saved attempt; current results are unchanged.\n");
        g_string_append_printf(
            text,
            "\nRun #%" G_GUINT64_FORMAT "; attempt %" G_GUINT64_FORMAT
            "; repeat %u\n%s\nTest ID: %s\nBuild: %s\nConfiguration: %s\n"
            "Status: %s; outcome: %s; exit code: %d; duration: %" G_GUINT64_FORMAT " ms\n\n%s",
            (guint64)id, (guint64)row, (unsigned)pass, result->name, result->test_id, request.build_directory,
            request.configuration[0] ? request.configuration : "(default)", umi_status_text(result->status),
            umi_test_state_text(result->state), result->exit_code, (guint64)result->duration_ms,
            entry.origin.retain_output ? result->output : "Diagnostic text was not retained for this run.");
        RuntimeTestArchiveText(runtime, text->str);
        g_string_free(text, TRUE);
    }
    g_free(result);
}

#endif
static void RuntimeTestArchiveRenderAttempt(UmiStudioGtkRuntimeChrome *runtime,
    const UmiTestArchiveReaderSnapshot *read)
{
    UmiTestArchiveAttempt *packet = g_new0(UmiTestArchiveAttempt, 1);
    UmiStatus status = UmiStudioTestArchiveCopyAttempt(RuntimeTestArchiveService(runtime), packet);
    UmiTestResult *result = &packet->result;
    uint64_t id = read->id, row = (uint64_t)read->index + 1U;
    uint32_t pass = packet->attempt;
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, umi_status_text(status));
    else
    {
        GString *text = g_string_new("Saved attempt; current results are unchanged.\n");
        g_string_append_printf(
            text,
            "\nRun #%" G_GUINT64_FORMAT "; attempt %" G_GUINT64_FORMAT
            "; repeat %u\n%s\nTest ID: %s\nBuild: %s\nConfiguration: %s\n"
            "Status: %s; outcome: %s; exit code: %d; duration: %" G_GUINT64_FORMAT " ms\n\n%s",
            (guint64)id, (guint64)row, (unsigned)pass, result->name, result->test_id, packet->request.build_directory,
            packet->request.configuration[0] ? packet->request.configuration : "(default)", umi_status_text(result->status),
            umi_test_state_text(result->state), result->exit_code, (guint64)result->duration_ms,
            packet->entry.origin.retain_output ? result->output : "Diagnostic text was not retained for this run.");
        RuntimeTestArchiveText(runtime, text->str);
        g_string_free(text, TRUE);
    }
    g_free(packet);
}
static void RuntimeTestArchiveReadAttempt(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    uint64_t id = 0, row = 0;
    if (!RuntimeTestArchiveNumber(runtime->test_archive_id, &id) ||
        !RuntimeTestArchiveNumber(runtime->test_archive_attempt, &row) || row > UMI_CTEST_JOB_MAX_ATTEMPTS)
    {
        runtime_show_notification(runtime,
                                  "Enter a saved run ID and a completed attempt number starting at 1.");
        return;
    }
    RuntimeTestArchiveBeginRead(runtime, UMI_TEST_ARCHIVE_READ_ATTEMPT, id, (size_t)(row - 1U));
}
/* Storage deletion moves off the GTK event loop into the Framework worker. The former synchronous action is retained for engineering review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveRemove(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    uint64_t id = 0;
    if (!RuntimeTestArchiveNumber(runtime->test_archive_id, &id))
    {
        runtime_show_notification(runtime, "Enter the exact saved run ID to remove.");
        return;
    }
    UmiStatus status = UmiStudioTestArchiveRemove(RuntimeTestArchiveService(runtime), id);
    runtime_show_notification(
        runtime, status == UMI_STATUS_OK
                     ? "The selected saved run was removed. Source files and live results were not changed."
                     : umi_status_text(status));
    if (status == UMI_STATUS_OK)
        RuntimeTestArchiveList(button, data);
}
#endif
static void RuntimeTestArchiveRemove(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    uint64_t id = 0;
    if (!RuntimeTestArchiveNumber(runtime->test_archive_id, &id))
    {
        runtime_show_notification(runtime, "Enter the exact saved run ID to remove.");
        return;
    }
    /* The opening worker may already be terminal while its attachment still
     * awaits UI publication. Do not remove from the old archive in that interval. */
    if (runtime->test_archive_open_pending)
    {
        runtime_show_notification(runtime, "Wait for archive opening to finish before removing a saved run.");
        return;
    }
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioTestArchiveRemoveBegin(umi_studio_services_tests(services),
        umi_studio_services_task_queue(services), id);
    if (status == UMI_STATUS_OK)
    {
        runtime->test_archive_remove_pending = true;
        runtime->test_archive_read_pending = false;
        runtime->test_archive_comparison_pending = false;
        RuntimeTestArchiveText(runtime,
            "Removing the selected saved run. Stop removal remains available until commit.");
    }
    runtime_show_notification(runtime, status == UMI_STATUS_OK
        ? "Removal queued. Source files and current test results are unchanged."
        : umi_status_text(status));
}
static void RuntimeTestArchiveStopRemoval(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStatus status = UmiStudioTestArchiveRemoveCancel(RuntimeTestArchiveService(runtime));
    runtime_show_notification(runtime, status == UMI_STATUS_OK
        ? "Stop requested. A removal already committed cannot be undone."
        : umi_status_text(status));
}
#include "test_archive_comparison.inc"

/* Periodic refresh never reads the database. It cannot contend with a save's
 * SQLite transaction or turn a historical outcome into a live test result. */
/* Archive progress now includes a background comparison and bounded result pages. The previous save-only refresh is retained for review.
 * The previous implementation remains here for engineering review. */
#if 0
static void RuntimeTestArchiveRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime->test_archive_status == NULL)
        return;
    UmiStudioTestArchiveState state = {0};
    UmiStatus status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
    char message[256];
    if (status != UMI_STATUS_OK)
        (void)snprintf(message, sizeof(message), "%s", umi_status_text(status));
    else if (!state.attached)
        (void)snprintf(message, sizeof(message), "No private test archive attached.");
    else if (state.pending)
        (void)snprintf(message, sizeof(message), "Saving test evidence; Stop remains available.");
    else if (state.write.saved.id != 0)
        (void)snprintf(message, sizeof(message),
                       "Committed saved run #%" G_GUINT64_FORMAT ". Refresh saved runs to view it.",
                       (guint64)state.write.saved.id);
    else if (state.write.state == UMI_TASK_CREATED)
        (void)snprintf(message, sizeof(message),
                       "Archive ready. Saving a completed run is an explicit action.");
    else
        (void)snprintf(message, sizeof(message), "Save did not confirm a commit: %s",
                       umi_status_text(state.write.status));
    gtk_label_set_text(GTK_LABEL(runtime->test_archive_status), message);
}
#endif
/* Progress polling reports the storage commit outcome without running deletion on the event loop. The former refresh remains for review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeTestArchiveRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime->test_archive_status == NULL)
        return;
    UmiStudioTestArchiveState state = {0};
    UmiStatus status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
    /* Publication is an owner-thread decision, not a worker callback into GTK.
     * This makes a late Stop meaningful and prevents a failed candidate from
     * clearing the previous archive. Retiring an old attachment can close its
     * storage handle; all opening and catalogue reads remain on the queue. */
    if (status == UMI_STATUS_OK && runtime->test_archive_open_pending &&
        state.opening.state != UMI_TASK_QUEUED && state.opening.state != UMI_TASK_RUNNING)
    {
        UmiStatus published = UmiStudioTestArchiveOpenPublish(RuntimeTestArchiveService(runtime));
        if (published != UMI_STATUS_BUSY)
        {
            runtime->test_archive_open_pending = false;
            if (published == UMI_STATUS_OK)
                RuntimeTestArchiveList(NULL, runtime);
            else
                runtime_show_notification(runtime, umi_status_text(published));
            status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
        }
    }
    /* The worker owns database reads. Rendering here consumes only immutable
     * memory, and the page limit bounds each completion update. */
    if (status == UMI_STATUS_OK && runtime->test_archive_comparison_pending &&
        state.comparison.state != UMI_TASK_QUEUED && state.comparison.state != UMI_TASK_RUNNING)
    {
        runtime->test_archive_comparison_pending = false;
        if (state.comparison.ready)
            RuntimeTestArchiveComparisonPage(NULL, runtime);
        else
            runtime_show_notification(runtime, umi_status_text(state.comparison.status));
    }
    if (status == UMI_STATUS_OK && runtime->test_archive_read_pending &&
        state.reader.state != UMI_TASK_QUEUED && state.reader.state != UMI_TASK_RUNNING)
    {
        runtime->test_archive_read_pending = false;
        if (!state.reader.ready)
            runtime_show_notification(runtime, umi_status_text(state.reader.status));
        else if (state.reader.kind == UMI_TEST_ARCHIVE_READ_CATALOG)
            RuntimeTestArchiveRenderCatalog(NULL, runtime);
        else
            RuntimeTestArchiveRenderAttempt(runtime, &state.reader);
    }
    char message[256];
    if (status != UMI_STATUS_OK)
        (void)snprintf(message, sizeof(message), "%s", umi_status_text(status));
    else if (runtime->test_archive_open_pending)
        (void)snprintf(message, sizeof(message),
            "Opening private test archive; Stop opening remains available.");
    else if (!state.attached)
        (void)snprintf(message, sizeof(message), "No private test archive attached.");
    else if (state.reader.state == UMI_TASK_QUEUED || state.reader.state == UMI_TASK_RUNNING)
        (void)snprintf(message, sizeof(message), "Reading saved evidence; Stop reading remains available.");
    else if (state.comparison.state == UMI_TASK_QUEUED || state.comparison.state == UMI_TASK_RUNNING)
        (void)snprintf(message, sizeof(message),
                       "Comparing saved outcomes; Stop comparison remains available.");
    else if (state.pending)
        (void)snprintf(message, sizeof(message), "Saving test evidence; Stop remains available.");
    else if (state.write.saved.id != 0)
        (void)snprintf(message, sizeof(message),
                       "Committed saved run #%" G_GUINT64_FORMAT ". Refresh saved runs to view it.",
                       (guint64)state.write.saved.id);
    else if (state.write.state == UMI_TASK_CREATED)
        (void)snprintf(message, sizeof(message),
                       "Archive ready. Saving a completed run is an explicit action.");
    else
        (void)snprintf(message, sizeof(message), "Save did not confirm a commit: %s",
                       umi_status_text(state.write.status));
    gtk_label_set_text(GTK_LABEL(runtime->test_archive_status), message);
}
#endif
static void RuntimeTestArchiveRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime->test_archive_status == NULL)
        return;
    UmiStudioTestArchiveState state = {0};
    UmiStatus status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
    /* Publication is an owner-thread decision, not a worker callback into GTK.
     * This makes a late Stop meaningful and prevents a failed candidate from
     * clearing the previous archive. Retiring an old attachment can close its
     * storage handle; all opening and catalogue reads remain on the queue. */
    if (status == UMI_STATUS_OK && runtime->test_archive_open_pending &&
        state.opening.state != UMI_TASK_QUEUED && state.opening.state != UMI_TASK_RUNNING)
    {
        UmiStatus published = UmiStudioTestArchiveOpenPublish(RuntimeTestArchiveService(runtime));
        if (published != UMI_STATUS_BUSY)
        {
            runtime->test_archive_open_pending = false;
            if (published == UMI_STATUS_OK)
                RuntimeTestArchiveList(NULL, runtime);
            else
                runtime_show_notification(runtime, umi_status_text(published));
            status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
        }
    }
    /* The worker owns database reads. Rendering here consumes only immutable
     * memory, and the page limit bounds each completion update. */
    if (status == UMI_STATUS_OK && runtime->test_archive_comparison_pending &&
        state.comparison.state != UMI_TASK_QUEUED && state.comparison.state != UMI_TASK_RUNNING)
    {
        runtime->test_archive_comparison_pending = false;
        if (state.comparison.ready)
            RuntimeTestArchiveComparisonPage(NULL, runtime);
        else
            runtime_show_notification(runtime, umi_status_text(state.comparison.status));
    }
    if (status == UMI_STATUS_OK && runtime->test_archive_read_pending &&
        state.reader.state != UMI_TASK_QUEUED && state.reader.state != UMI_TASK_RUNNING)
    {
        runtime->test_archive_read_pending = false;
        if (!state.reader.ready)
            runtime_show_notification(runtime, umi_status_text(state.reader.status));
        else if (state.reader.kind == UMI_TEST_ARCHIVE_READ_CATALOG)
            RuntimeTestArchiveRenderCatalog(NULL, runtime);
        else
            RuntimeTestArchiveRenderAttempt(runtime, &state.reader);
    }
    /* A committed outcome survives late Stop. Refresh the list only for this
     * accepted action; a newer open, read or comparison retires its notice. */
    if (status == UMI_STATUS_OK && runtime->test_archive_remove_pending &&
        state.removal.state != UMI_TASK_QUEUED && state.removal.state != UMI_TASK_RUNNING)
    {
        runtime->test_archive_remove_pending = false;
        if (state.removal.committed)
        {
            RuntimeTestArchiveList(NULL, runtime);
            runtime_show_notification(runtime, "The selected saved run was removed.");
        }
        else
        {
            RuntimeTestArchiveText(runtime, "Removal did not confirm a commit. Refresh saved runs to inspect the archive.");
            runtime_show_notification(runtime, umi_status_text(state.removal.status));
        }
        status = UmiStudioTestArchiveStateRead(RuntimeTestArchiveService(runtime), &state);
    }
    char message[256];
    if (status != UMI_STATUS_OK)
        (void)snprintf(message, sizeof(message), "%s", umi_status_text(status));
    else if (runtime->test_archive_open_pending)
        (void)snprintf(message, sizeof(message),
            "Opening private test archive; Stop opening remains available.");
    else if (!state.attached)
        (void)snprintf(message, sizeof(message), "No private test archive attached.");
    else if (state.removal.state == UMI_TASK_QUEUED || state.removal.state == UMI_TASK_RUNNING)
        (void)snprintf(message, sizeof(message), "Removing saved evidence; Stop removal remains available.");
    else if (state.reader.state == UMI_TASK_QUEUED || state.reader.state == UMI_TASK_RUNNING)
        (void)snprintf(message, sizeof(message), "Reading saved evidence; Stop reading remains available.");
    else if (state.comparison.state == UMI_TASK_QUEUED || state.comparison.state == UMI_TASK_RUNNING)
        (void)snprintf(message, sizeof(message),
                       "Comparing saved outcomes; Stop comparison remains available.");
    else if (state.pending)
        (void)snprintf(message, sizeof(message), "Saving test evidence; Stop remains available.");
    else if (state.write.saved.id != 0)
        (void)snprintf(message, sizeof(message),
                       "Committed saved run #%" G_GUINT64_FORMAT ". Refresh saved runs to view it.",
                       (guint64)state.write.saved.id);
    else if (state.write.state == UMI_TASK_CREATED)
        (void)snprintf(message, sizeof(message),
                       "Archive ready. Saving a completed run is an explicit action.");
    else
        (void)snprintf(message, sizeof(message), "Save did not confirm a commit: %s",
                       umi_status_text(state.write.status));
    gtk_label_set_text(GTK_LABEL(runtime->test_archive_status), message);
}
static GtkWidget *RuntimeTestArchiveButton(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *box,
                                           const char *label, const char *tag,
                                           void (*callback)(GtkButton *, gpointer))
{
    GtkWidget *button = gtk_button_new_with_label(label);
    (void)umi_gtk4_automation_tag_widget(button, tag);
    runtime_bind_editor_action(runtime, button, callback);
    gtk_box_append(GTK_BOX(box), button);
    return button;
}
static void RuntimeTestArchiveControls(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *expander = gtk_expander_new("Private test history");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_expander_set_child(GTK_EXPANDER(expander), box);
    GtkWidget *help = gtk_label_new(
        "Choose a private SQLite file outside source repositories. "
        "Reopen it after restarting Studio. Names and paths are retained; diagnostic output is optional. "
        "The database is not encrypted. Saved runs never launch tests or replace current results.");
    gtk_label_set_wrap(GTK_LABEL(help), TRUE);
    gtk_label_set_xalign(GTK_LABEL(help), 0.0F);
    gtk_box_append(GTK_BOX(box), help);
    runtime->test_archive_path = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->test_archive_path),
                                   "Absolute path to a private test archive");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->test_archive_path), (int)UMI_PATH_CAPACITY - 1);
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_path, "studio.tests.archive.path");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_path);
    GtkWidget *files = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    RuntimeTestArchiveButton(runtime, files, "Open archive", "studio.tests.archive.open",
                             RuntimeTestArchiveOpen);
    RuntimeTestArchiveButton(runtime, files, "Stop opening", "studio.tests.archive.open.stop",
                             RuntimeTestArchiveStopOpen);
    RuntimeTestArchiveButton(runtime, files, "Refresh saved runs", "studio.tests.archive.refresh",
                             RuntimeTestArchiveList);
    GtkWidget *detach = RuntimeTestArchiveButton(runtime, files, "Detach", "studio.tests.archive.detach",
                                                 RuntimeTestArchiveOpen);
    g_object_set_data(G_OBJECT(detach), "umicom-test-archive-detach", GINT_TO_POINTER(1));
    RuntimeTestArchiveButton(runtime, files, "Stop reading", "studio.tests.archive.read.stop",
                             RuntimeTestArchiveStopRead);
    gtk_box_append(GTK_BOX(box), files);
    runtime->test_archive_output =
        gtk_check_button_new_with_label("Retain diagnostic text (may contain private data)");
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_output, "studio.tests.archive.output");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_output);
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    RuntimeTestArchiveButton(runtime, actions, "Save completed run", "studio.tests.archive.save",
                             RuntimeTestArchiveSave);
    RuntimeTestArchiveButton(runtime, actions, "Stop save", "studio.tests.archive.stop",
                             RuntimeTestArchiveStop);
    gtk_box_append(GTK_BOX(box), actions);
    runtime->test_archive_id = gtk_entry_new();
    runtime->test_archive_attempt = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->test_archive_id), "Saved run ID");
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->test_archive_attempt),
                                   "Completed attempt number (1, 2, ...)");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->test_archive_id), 20);
    gtk_entry_set_max_length(GTK_ENTRY(runtime->test_archive_attempt), 10);
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_id, "studio.tests.archive.id");
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_attempt, "studio.tests.archive.attempt");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_id);
    gtk_box_append(GTK_BOX(box), runtime->test_archive_attempt);
    GtkWidget *records = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    RuntimeTestArchiveButton(runtime, records, "Read saved attempt", "studio.tests.archive.read",
                             RuntimeTestArchiveReadAttempt);
    RuntimeTestArchiveButton(runtime, records, "Remove this saved run", "studio.tests.archive.remove",
                             RuntimeTestArchiveRemove);
    RuntimeTestArchiveButton(runtime, records, "Stop removal", "studio.tests.archive.remove.stop",
                             RuntimeTestArchiveStopRemoval);
    gtk_box_append(GTK_BOX(box), records);
    /* Exact decimal IDs avoid rounding large durable identifiers. The candidate
     * is the existing saved-run field, so read and compare address the same run. */
    runtime->test_archive_baseline = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->test_archive_baseline),
                                   "Baseline run ID for comparison");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->test_archive_baseline), 20);
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_baseline, "studio.tests.archive.baseline");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_baseline);
    runtime->test_archive_comparison_page = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->test_archive_comparison_page),
                                   "Comparison page (100 rows per page)");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->test_archive_comparison_page), 4);
    gtk_editable_set_text(GTK_EDITABLE(runtime->test_archive_comparison_page), "1");
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_comparison_page, "studio.tests.archive.page");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_comparison_page);
    GtkWidget *compare = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    RuntimeTestArchiveButton(runtime, compare, "Compare saved runs", "studio.tests.archive.compare",
                             RuntimeTestArchiveCompare);
    RuntimeTestArchiveButton(runtime, compare, "Stop comparison", "studio.tests.archive.compare.stop",
                             RuntimeTestArchiveCompareStop);
    RuntimeTestArchiveButton(runtime, compare, "View comparison page", "studio.tests.archive.compare.page",
                             RuntimeTestArchiveComparisonPage);
    gtk_box_append(GTK_BOX(box), compare);
    runtime->test_archive_status = gtk_label_new(NULL);
    gtk_label_set_wrap(GTK_LABEL(runtime->test_archive_status), TRUE);
    gtk_label_set_xalign(GTK_LABEL(runtime->test_archive_status), 0.0F);
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_status, "studio.tests.archive.status");
    gtk_box_append(GTK_BOX(box), runtime->test_archive_status);
    runtime->test_archive_entries = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(runtime->test_archive_entries), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(runtime->test_archive_entries), TRUE);
    (void)umi_gtk4_automation_tag_widget(runtime->test_archive_entries, "studio.tests.archive.entries");
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 200);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), runtime->test_archive_entries);
    gtk_box_append(GTK_BOX(box), scroll);
    gtk_box_append(GTK_BOX(panel), expander);
    RuntimeTestArchiveRefresh(runtime);
}
