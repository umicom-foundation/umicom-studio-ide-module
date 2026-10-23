/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_evidence_runs_gtk4.c
 * PURPOSE: Exercise the real run picker, evidence panes and stale source controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#include "../src/gui/workbench/workbench_window.c"

static UmiStudioGtkWorkbench **host;
static const char *mode;
static int armed, observed;
static void RunMutation(GObject *object, GParamSpec *property, gpointer data)
{
    (void)object; (void)property; (void)data;
    if (!armed || host == NULL || *host == NULL) return;
    armed = 0; ++observed;
    if (strstr(mode, "-close") != NULL) {
        UmiStudioGtkWorkbench *old = *host; *host = NULL; umi_studio_gtk_workbench_destroy(old);
    } else if (strcmp(mode, "model-reselect") == 0) {
        strcpy((*host)->runtime_chrome->selected_test_id, "test.other");
    } else if (strcmp(mode, "source-reselect") == 0) {
        /* The fixture catalogue is All, run.a, run.b, run.c. */
        gtk_drop_down_set_selected(GTK_DROP_DOWN((*host)->runtime_chrome->test_run_picker), 2U);
    } else RuntimeSelectedTestEvidenceRefresh((*host)->runtime_chrome);
}
static char *RunViewText(GtkWidget *view)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GtkTextIter a, b; gtk_text_buffer_get_bounds(buffer, &a, &b);
    return gtk_text_buffer_get_text(buffer, &a, &b, TRUE);
}
static UmiStatus RunResult(UmiTestPlatformResultRegistry *results, const char *id,
    const char *item, const char *run, const char *source, int outcome)
{
    UmiTestPlatformResultSnapshot row = {0};
    g_strlcpy(row.id, id, sizeof row.id); g_strlcpy(row.item_id, item, sizeof row.item_id);
    g_strlcpy(row.session_id, run, sizeof row.session_id);
    row.outcome = outcome;
    if (source != NULL) (void)snprintf(row.failure_details, sizeof row.failure_details, "%s:2:2: error: %s", source, id);
    else g_strlcpy(row.message, id, sizeof row.message);
    return umi_test_platform_result_registry_upsert(results, &row);
}

/* Keep async clipboard state alive even if cancellation completes after the
 * caller's deadline. Only the main context touches the result text. */
typedef struct RunClipboardRead {
    unsigned references;
    int done;
    char *text;
} RunClipboardRead;
static void RunClipboardRelease(RunClipboardRead *read)
{
    if (--read->references == 0U) { g_free(read->text); g_free(read); }
}
static void RunClipboardReady(GObject *source, GAsyncResult *result, gpointer data)
{
    RunClipboardRead *read = data;
    read->text = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), result, NULL);
    read->done = 1;
    RunClipboardRelease(read);
}
static char *RunClipboardText(GdkClipboard *clipboard)
{
    RunClipboardRead *read = g_new0(RunClipboardRead, 1); read->references = 2U;
    GCancellable *cancel = g_cancellable_new();
    gdk_clipboard_read_text_async(clipboard, cancel, RunClipboardReady, read);
    gint64 end = g_get_monotonic_time() + 3 * G_TIME_SPAN_SECOND;
    while (!read->done && g_get_monotonic_time() < end) {
        (void)g_main_context_iteration(NULL, FALSE); g_usleep(1000U);
    }
    char *text = read->done ? g_strdup(read->text) : NULL;
    if (!read->done) g_cancellable_cancel(cancel);
    RunClipboardRelease(read); g_object_unref(cancel);
    return text;
}

static guint RunIndex(UmiStudioGtkRuntimeChrome *chrome, const char *id)
{
    RuntimeTestRunState *state = RuntimeTestRuns(chrome);
    UmiTestEvidenceRunsSummary summary;
    if (state == NULL || UmiTestEvidenceRunsRead(state->catalogue, &summary) != UMI_STATUS_OK) return GTK_INVALID_LIST_POSITION;
    for (size_t i = 0U; i < summary.runs; ++i) {
        UmiTestEvidenceRun row;
        if (UmiTestEvidenceRunsAt(state->catalogue, i, &row) == UMI_STATUS_OK && strcmp(row.session_id, id) == 0) return (guint)i + 1U;
    }
    return GTK_INVALID_LIST_POSITION;
}
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *cases[] = {"all", "exact", "output-only", "unspecified", "missing-run", "back-all", "test-change", "unchanged", "retained-row", "retained-picker", "model-close", "summary-close", "model-reselect", "refresh-reenter", "invalid-bytes", "csv", "source", "stale-picker", "source-reselect"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(argv[1], cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    mode = argv[1];
    int failed = 0, changed = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "runs-XXXXXX", NULL), *a = NULL, *b = NULL, *text = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *picker = NULL, *view = NULL, *retained = NULL;
    GObject *observation = NULL; gulong handler = 0U;
    UmiTestEvidence *capture = NULL; UmiCsvDocument *csv = NULL;
    CHECK(g_mkdtemp(root) != NULL && g_chdir(root) == 0); changed = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions serviceOptions = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.evidence-runs-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    host = &workbench;
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TEST_EXPLORER, NULL) == UMI_STATUS_OK);
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    UmiTestPlatformService *platform = runtime_test_platform(chrome);
    CHECK(platform != NULL);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    UmiTestPlatformOutputRegistry *output = umi_test_platform_service_output(platform);
    umi_test_platform_result_registry_clear(results); umi_test_platform_output_registry_clear(output);
    a = g_build_filename(root, "older.c", NULL); b = g_build_filename(root, "newer.c", NULL);
    CHECK(g_file_set_contents(a, "first\nsecond\n", -1, NULL));
    CHECK(g_file_set_contents(b, "first\nsecond\n", -1, NULL));
    UmiTestPlatformItemSnapshot item = {0};
    strcpy(item.id, "test.a"); strcpy(item.name, "Selected test"); strcpy(item.framework, "ctest");
    strcpy(item.kind, "test"); item.enabled = 1; item.discovered = 1;
    g_strlcpy(item.working_directory, root, sizeof item.working_directory);
    CHECK(umi_test_platform_item_registry_upsert(umi_test_platform_service_item(platform), &item) == UMI_STATUS_OK);
    CHECK(RunResult(results, "older-result", "test.a", "run.a", a, UMI_TEST_PLATFORM_OUTCOME_FAILED) == UMI_STATUS_OK);
    CHECK(RunResult(results, "newer-result", "test.a", "run.b", b, UMI_TEST_PLATFORM_OUTCOME_FAILED) == UMI_STATUS_OK);
    CHECK(RunResult(results, "unassigned-result", "test.a", "", NULL, UMI_TEST_PLATFORM_OUTCOME_NOT_RUN) == UMI_STATUS_OK);
    CHECK(RunResult(results, "other-test-result", "test.other", "run.a", NULL, UMI_TEST_PLATFORM_OUTCOME_SKIPPED) == UMI_STATUS_OK);
    UmiTestPlatformOutputSnapshot out = {0};
    strcpy(out.id, "output-only"); strcpy(out.item_id, "test.a"); strcpy(out.session_id, "run.c"); strcpy(out.text, "only-output-marker");
    CHECK(umi_test_platform_output_registry_upsert(output, &out) == UMI_STATUS_OK);
    strcpy(chrome->selected_test_id, "test.a");
    RuntimeSelectedTestEvidenceRefresh(chrome);
    CHECK(GTK_IS_DROP_DOWN(chrome->test_run_picker));
    picker = g_object_ref(chrome->test_run_picker); view = g_object_ref(chrome->test_selected_view);
    CHECK(g_list_model_get_n_items(gtk_drop_down_get_model(GTK_DROP_DOWN(picker))) == 4U);
    if (strcmp(mode, "all") == 0 || strcmp(mode, "unspecified") == 0) {
        text = RunViewText(view);
        CHECK(strstr(text, "older-result") && strstr(text, "newer-result") && strstr(text, "unassigned-result") && strstr(text, "only-output-marker"));
        CHECK(strstr(text, "other-test-result") == NULL);
        if (strcmp(mode, "unspecified") == 0) {
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(chrome->test_run_summary)), "1 results and 0 output records") != NULL);
            gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.a"));
            g_clear_pointer(&text, g_free); text = RunViewText(view);
            CHECK(strstr(text, "unassigned-result") == NULL);
        }
    } else if (strcmp(mode, "output-only") == 0) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.c"));
        text = RunViewText(view);
        CHECK(strstr(text, "only-output-marker") && strstr(text, "older-result") == NULL && strstr(text, "newer-result") == NULL);
    } else if (strstr(mode, "-close") || strcmp(mode, "model-reselect") == 0 || strcmp(mode, "refresh-reenter") == 0) {
        observation = g_object_ref(strcmp(mode, "summary-close") == 0 ? G_OBJECT(chrome->test_run_summary) : G_OBJECT(picker));
        handler = g_signal_connect(observation, strcmp(mode, "summary-close") == 0 ? "notify::label" : "notify::model", G_CALLBACK(RunMutation), NULL);
        CHECK(RunResult(results, "additional", "test.a", "run.d", NULL, UMI_TEST_PLATFORM_OUTCOME_CANCELLED) == UMI_STATUS_OK);
        armed = 1; RuntimeSelectedTestEvidenceRefresh(chrome);
        CHECK(observed == 1);
        if (strstr(mode, "-close")) CHECK(workbench == NULL);
        else {
            CHECK(workbench != NULL);
            RuntimeSelectedTestEvidenceRefresh(workbench->runtime_chrome);
            text = RunViewText(view);
            if (strcmp(mode, "model-reselect") == 0) CHECK(strstr(text, "other-test-result") && !strstr(text, "older-result"));
            else CHECK(strstr(text, "additional") != NULL);
        }
    } else if (strcmp(mode, "stale-picker") == 0) {
        CHECK(RunResult(results, "arriving", "test.a", "run.d", NULL, UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT) == UMI_STATUS_OK);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), 2U);
        CHECK(gtk_drop_down_get_selected(GTK_DROP_DOWN(picker)) == 0U);
        CHECK(g_list_model_get_n_items(gtk_drop_down_get_model(GTK_DROP_DOWN(picker))) == 5U);
        text = RunViewText(view); CHECK(strstr(text, "arriving") && strstr(text, "older-result"));
    } else if (strcmp(mode, "source-reselect") == 0) {
        observation = g_object_ref(G_OBJECT(chrome->test_failure_summary));
        handler = g_signal_connect(observation, "notify::label", G_CALLBACK(RunMutation), NULL);
        armed = 1;
        gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.a"));
        CHECK(observed == 1 && strcmp(RuntimeTestRuns(chrome)->session_id, "run.b") == 0);
        RuntimeSelectedTestEvidenceRefresh(chrome);
        text = RunViewText(view); CHECK(strstr(text, "newer-result") && !strstr(text, "older-result"));
        GtkWidget *link = umi_gtk4_automation_find_tagged_widget(
            GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.tests.source-location.0");
        CHECK(GTK_IS_BUTTON(link));
        const UmiStudioTestLinkAction *action = g_object_get_data(G_OBJECT(link), "umicom-test-link");
        CHECK(action != NULL && strcmp(action->stamp.session_id, "run.b") == 0 && strcmp(action->link.session_id, "run.b") == 0);
    } else if (strcmp(mode, "invalid-bytes") == 0) {
        CHECK(RunResult(results, "invalid-id", "test.a", "run.\xff", NULL, UMI_TEST_PLATFORM_OUTCOME_SKIPPED) == UMI_STATUS_OK);
        RuntimeSelectedTestEvidenceRefresh(chrome);
        GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(picker));
        for (guint i = 0U; i < g_list_model_get_n_items(model); ++i) {
            GtkStringObject *object = g_list_model_get_item(model, i);
            int valid = g_utf8_validate(gtk_string_object_get_string(object), -1, NULL); g_object_unref(object); CHECK(valid);
        }
        gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.\xff"));
        text = RunViewText(view); CHECK(strstr(text, "invalid-id") != NULL && g_utf8_validate(text, -1, NULL));
    } else {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.a"));
        text = RunViewText(view);
        CHECK(strstr(text, "older-result") && !strstr(text, "newer-result") && !strstr(text, "only-output-marker") && !strstr(text, "unassigned-result"));
        g_clear_pointer(&text, g_free);
        GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
        GtkWidget *link = umi_gtk4_automation_find_tagged_widget(window, "studio.tests.source-location.0");
        CHECK(GTK_IS_BUTTON(link)); retained = g_object_ref(link);
        if (strcmp(mode, "exact") == 0) {
            CHECK(strcmp(RuntimeTestRuns(chrome)->session_id, "run.a") == 0);
        } else if (strcmp(mode, "back-all") == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), 0U);
            text = RunViewText(view); CHECK(strstr(text, "newer-result") && strstr(text, "older-result"));
        } else if (strcmp(mode, "missing-run") == 0) {
            CHECK(umi_test_platform_result_registry_remove(results, "older-result") == UMI_STATUS_OK);
            RuntimeSelectedTestEvidenceRefresh(chrome);
            CHECK(strcmp(RuntimeTestRuns(chrome)->session_id, "run.a") == 0);
            text = RunViewText(view); CHECK(!strstr(text, "newer-result") && !strstr(text, "older-result"));
            CHECK(RuntimeTestRunCapture(chrome, &capture) == UMI_STATUS_NOT_FOUND && capture == NULL);
            gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), 0U);
            g_clear_pointer(&text, g_free); text = RunViewText(view); CHECK(strstr(text, "newer-result"));
        } else if (strcmp(mode, "test-change") == 0) {
            strcpy(chrome->selected_test_id, "test.other"); RuntimeSelectedTestEvidenceRefresh(chrome);
            CHECK(RuntimeTestRuns(chrome)->session_id[0] == '\0');
            text = RunViewText(view); CHECK(strstr(text, "other-test-result") && !strstr(text, "older-result"));
        } else if (strcmp(mode, "unchanged") == 0) {
            GListModel *before = gtk_drop_down_get_model(GTK_DROP_DOWN(picker));
            RuntimeSelectedTestEvidenceRefresh(chrome);
            CHECK(gtk_drop_down_get_model(GTK_DROP_DOWN(picker)) == before);
            CHECK(umi_gtk4_automation_find_tagged_widget(window, "studio.tests.source-location.0") == retained);
        } else if (strcmp(mode, "retained-row") == 0 || strcmp(mode, "source") == 0) {
            UmiDocumentCoordinator *documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
            size_t before = umi_document_coordinator_count(documents);
            if (strcmp(mode, "retained-row") == 0) {
                gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), RunIndex(chrome, "run.b"));
                g_signal_emit_by_name(retained, "clicked");
                CHECK(umi_document_coordinator_count(documents) == before);
            } else {
                g_signal_emit_by_name(retained, "clicked");
                CHECK(umi_document_coordinator_count(documents) == before + 1U);
                UmiUiDocumentViewSnapshot active;
                UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(umi_studio_bootstrap_ui(bootstrap)));
                UmiDocumentWorkingCopySnapshot working;
                CHECK(umi_document_coordinator_active_snapshot(documents, &working) == UMI_STATUS_OK);
                CHECK(umi_ui_document_view_model_find(views, working.view_id, &active) == UMI_STATUS_OK);
                CHECK(active.cursor_offset == 7U);
            }
        } else if (strcmp(mode, "retained-picker") == 0) {
            umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
            gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), 0U);
        } else if (strcmp(mode, "csv") == 0) {
            CHECK(RuntimeTestRunCapture(chrome, &capture) == UMI_STATUS_OK);
            CHECK(UmiTestEvidenceExportCsv(capture, &csv) == UMI_STATUS_OK);
            CHECK(strstr(UmiCsvDocumentData(csv), "older-result") && !strstr(UmiCsvDocumentData(csv), "newer-result"));
            GtkWidget *copy = umi_gtk4_automation_find_tagged_widget(window, "studio.tests.copy-evidence-csv");
            CHECK(GTK_IS_BUTTON(copy));
            g_signal_emit_by_name(copy, "clicked");
            text = RunClipboardText(gtk_widget_get_clipboard(window));
            CHECK(text != NULL && strcmp(text, UmiCsvDocumentData(csv)) == 0);
        } else CHECK(!"unknown test");
    }
cleanup:
    armed = 0; host = NULL;
    if (handler != 0U) g_signal_handler_disconnect(observation, handler);
    UmiCsvDocumentDestroy(csv); UmiTestEvidenceDestroy(capture);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&picker); g_clear_object(&view); g_clear_object(&retained); g_clear_object(&observation);
    if (changed) (void)g_chdir(original);
    if (isolated) { if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE); else g_unsetenv("UMICOM_STUDIO_DATA_PATH"); }
    g_free(original); g_free(oldData); g_free(root); g_free(a); g_free(b); g_free(text);
    return failed;
}
