/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/test_run_selection.inc
 * PURPOSE: Present Framework run identities without merging evidence from different executions.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/test_platform/evidence_runs.h"

typedef struct RuntimeTestRunState {
    char item_id[128], session_id[128];
    uint64_t generation;
    int publishing;
    UmiTestEvidenceRuns *catalogue;
} RuntimeTestRunState;
static void RuntimeSelectedTestEvidenceRefresh(UmiStudioGtkRuntimeChrome *runtime);

static void RuntimeTestRunStateFree(gpointer data)
{
    RuntimeTestRunState *state = data;
    UmiTestEvidenceRunsDestroy(state->catalogue);
    g_free(state);
}
static RuntimeTestRunState *RuntimeTestRuns(UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL && runtime->test_run_picker != NULL
        ? g_object_get_data(G_OBJECT(runtime->test_run_picker), "umicom-test-runs") : NULL;
}
/* Copy scope before any native notification. A generation also detects an
 * away-and-back selection during a source-row refresh. */
static int RuntimeTestRunChoice(UmiStudioGtkRuntimeChrome *runtime, char session[128], uint64_t *generation)
{
    session[0] = '\0'; *generation = 0U;
    RuntimeTestRunState *state = RuntimeTestRuns(runtime);
    if (state == NULL) return runtime != NULL && runtime->test_run_picker == NULL;
    if (state->publishing || strcmp(state->item_id, runtime->selected_test_id) != 0) return 0;
    strcpy(session, state->session_id); *generation = state->generation;
    return state->catalogue != NULL;
}
static UmiStatus RuntimeTestRunCapture(UmiStudioGtkRuntimeChrome *runtime, UmiTestEvidence **capture)
{
    *capture = NULL;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (platform == NULL) return UMI_STATUS_UNAVAILABLE;
    RuntimeTestRunState *state = RuntimeTestRuns(runtime);
    if (state == NULL && runtime->test_run_picker == NULL)
        return UmiTestEvidenceCreate(umi_test_platform_service_result(platform),
            umi_test_platform_service_output(platform), runtime->selected_test_id, NULL, capture);
    if (state == NULL || state->publishing ||
        strcmp(state->item_id, runtime->selected_test_id) != 0 || state->catalogue == NULL)
        return UMI_STATUS_INVALID_STATE;
    return UmiTestEvidenceRunsCapture(state->catalogue,
        umi_test_platform_service_result(platform), umi_test_platform_service_output(platform),
        state->session_id, capture);
}
static UmiStudioGtkRuntimeChrome *RuntimeTestRunOwner(UmiStudioFileDialogRequest *request,
    GObject **window, GtkWidget *picker, const char *item)
{
    UmiStudioGtkRuntimeChrome *runtime = RuntimeResolveOwner(request, window);
    if (runtime == NULL || runtime->test_run_picker != picker ||
        (item != NULL && strcmp(runtime->selected_test_id, item) != 0)) return NULL;
    return runtime;
}

static int RuntimeTestRunsRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL) return 0;
    if (runtime->test_run_picker == NULL) return 1;
    RuntimeTestRunState *state = RuntimeTestRuns(runtime);
    if (state == NULL || state->publishing) return 0;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (state->catalogue != NULL && platform != NULL &&
        strcmp(state->item_id, runtime->selected_test_id) == 0 &&
        UmiTestEvidenceRunsValidate(state->catalogue, umi_test_platform_service_result(platform),
            umi_test_platform_service_output(platform)) == UMI_STATUS_OK) return 1;
    if (state->generation == UINT64_MAX) return 0;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return 0;
    GtkWidget *picker = g_object_ref(runtime->test_run_picker);
    GtkWidget *summaryLabel = g_object_ref(runtime->test_run_summary);
    GObject *window = NULL;
    UmiTestEvidenceRuns *next = NULL;
    GtkStringList *strings = gtk_string_list_new(NULL);
    int current = 0;
    char item[128]; g_strlcpy(item, runtime->selected_test_id, sizeof item);
    state->publishing = 1;
    ++state->generation;
    if (strcmp(state->item_id, item) != 0) state->session_id[0] = '\0';
    strcpy(state->item_id, item);
    UmiTestEvidenceRunsDestroy(state->catalogue); state->catalogue = NULL;
    UmiStatus status = platform != NULL && item[0] != '\0'
        ? UmiTestEvidenceRunsCreate(umi_test_platform_service_result(platform),
            umi_test_platform_service_output(platform), item, &next)
        : UMI_STATUS_UNAVAILABLE;
    UmiTestEvidenceRunsSummary summary = {0};
    if (status == UMI_STATUS_OK) status = UmiTestEvidenceRunsRead(next, &summary);
    gtk_string_list_append(strings, "All retained runs");
    guint selected = 0U;
    if (status == UMI_STATUS_OK) {
        for (size_t i = 0U; i < summary.runs; ++i) {
            UmiTestEvidenceRun row;
            if (UmiTestEvidenceRunsAt(next, i, &row) != UMI_STATUS_OK) goto cleanup;
            char *raw = g_strdup_printf("%s | %zu results, %zu output records", row.session_id, row.results, row.output);
            char *display = g_utf8_make_valid(raw, -1);
            gtk_string_list_append(strings, display);
            g_free(display); g_free(raw);
            if (strcmp(state->session_id, row.session_id) == 0) selected = (guint)i + 1U;
        }
    }
    /* Removing retained records must not silently broaden a chosen run to all
     * runs. Keep its identity visible and allow an explicit all-runs choice. */
    if (state->session_id[0] != '\0' && selected == 0U) {
        char *raw = g_strdup_printf("Unavailable run: %s", state->session_id);
        char *display = g_utf8_make_valid(raw, -1);
        selected = g_list_model_get_n_items(G_LIST_MODEL(strings));
        gtk_string_list_append(strings, display);
        g_free(display); g_free(raw);
    }
    gtk_drop_down_set_model(GTK_DROP_DOWN(picker), G_LIST_MODEL(strings));
    runtime = RuntimeTestRunOwner(request, &window, picker, item);
    if (runtime == NULL) goto cleanup;
    gtk_drop_down_set_selected(GTK_DROP_DOWN(picker), selected);
    runtime = RuntimeTestRunOwner(request, &window, picker, item);
    if (runtime == NULL || gtk_drop_down_get_model(GTK_DROP_DOWN(picker)) != G_LIST_MODEL(strings) ||
        gtk_drop_down_get_selected(GTK_DROP_DOWN(picker)) != selected) goto cleanup;
    char *text = status == UMI_STATUS_OK
        ? g_strdup_printf("%zu named runs. %zu results and %zu output records have no run ID and appear only in All retained runs.",
            summary.runs, summary.unspecified_results, summary.unspecified_output)
        : g_strdup(item[0] == '\0' ? "Select a test to choose its retained run." : "Run evidence is unavailable; refresh before selecting a run.");
    gtk_label_set_text(GTK_LABEL(summaryLabel), text); g_free(text);
    runtime = RuntimeTestRunOwner(request, &window, picker, item);
    if (runtime == NULL || runtime->test_run_summary != summaryLabel ||
        gtk_drop_down_get_model(GTK_DROP_DOWN(picker)) != G_LIST_MODEL(strings) ||
        gtk_drop_down_get_selected(GTK_DROP_DOWN(picker)) != selected) goto cleanup;
    if (next != NULL) {
        platform = runtime_test_platform(runtime);
        if (platform == NULL || UmiTestEvidenceRunsValidate(next,
                umi_test_platform_service_result(platform), umi_test_platform_service_output(platform)) != UMI_STATUS_OK) goto cleanup;
    }
    state->catalogue = next; next = NULL;
    current = 1;
cleanup:
    state->publishing = 0;
    UmiTestEvidenceRunsDestroy(next);
    g_object_unref(strings); g_object_unref(summaryLabel); g_object_unref(picker);
    g_clear_object(&window); runtime_file_dialog_request_free(request);
    return current;
}

static void RuntimeTestRunBindingFree(gpointer data, GClosure *closure)
{ (void)closure; runtime_file_dialog_request_free(data); }
static void RuntimeTestRunChanged(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    /* Refresh and notification callbacks may retire the pane while this
     * emission is still active. Retain its control through the final check. */
    g_object_ref(object);
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeTestRunOwner(data, &window, GTK_WIDGET(object), NULL);
    if (runtime == NULL || gtk_widget_get_root(GTK_WIDGET(object)) != GTK_ROOT(window)) goto cleanup;
    RuntimeTestRunState *state = RuntimeTestRuns(runtime);
    if (state == NULL || state->publishing || state->catalogue == NULL ||
        strcmp(state->item_id, runtime->selected_test_id) != 0 || state->generation == UINT64_MAX) goto cleanup;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (platform == NULL || UmiTestEvidenceRunsValidate(state->catalogue,
            umi_test_platform_service_result(platform), umi_test_platform_service_output(platform)) != UMI_STATUS_OK) {
        /* A native index refers to the displayed catalogue, not a newly
         * arrived run. Refresh first and require a deliberate current choice. */
        RuntimeSelectedTestEvidenceRefresh(runtime);
        runtime = RuntimeTestRunOwner(data, &window, GTK_WIDGET(object), NULL);
        if (runtime != NULL) runtime_show_notification(runtime, "Run evidence changed. Choose a run from the refreshed list.");
        goto cleanup;
    }
    guint index = gtk_drop_down_get_selected(GTK_DROP_DOWN(object));
    char session[128] = "";
    if (index != 0U) {
        UmiTestEvidenceRun row;
        if (index == GTK_INVALID_LIST_POSITION ||
            UmiTestEvidenceRunsAt(state->catalogue, (size_t)index - 1U, &row) != UMI_STATUS_OK) goto cleanup;
        strcpy(session, row.session_id);
    }
    if (strcmp(session, state->session_id) == 0) goto cleanup;
    strcpy(state->session_id, session); ++state->generation;
    RuntimeSelectedTestEvidenceRefresh(runtime);
cleanup:
    g_clear_object(&window);
    g_object_unref(object);
}
/* Widget teardown invalidates the borrowed chrome fields. The closure and
 * finalization observer each own a separate weak application request. */
static void RuntimeTestRunFinalized(gpointer data, GObject *object)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeResolveOwner(data, &window);
    if (runtime != NULL && (GObject *)runtime->test_run_picker == object) {
        runtime->test_run_picker = NULL; runtime->test_run_summary = NULL;
    }
    g_clear_object(&window); runtime_file_dialog_request_free(data);
}
static GtkWidget *RuntimeTestRunControls(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    runtime->test_run_picker = gtk_drop_down_new_from_strings((const char *[]){"All retained runs", NULL});
    runtime->test_run_summary = gtk_label_new("Select a test to choose its retained run.");
    gtk_label_set_wrap(GTK_LABEL(runtime->test_run_summary), TRUE);
    gtk_label_set_xalign(GTK_LABEL(runtime->test_run_summary), 0.0F);
    RuntimeTestRunState *state = g_new0(RuntimeTestRunState, 1);
    g_object_set_data_full(G_OBJECT(runtime->test_run_picker), "umicom-test-runs", state, RuntimeTestRunStateFree);
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request != NULL) g_signal_connect_data(runtime->test_run_picker, "notify::selected",
        G_CALLBACK(RuntimeTestRunChanged), request, RuntimeTestRunBindingFree, 0);
    request = runtime_file_dialog_request_new(runtime, 0U);
    if (request != NULL) g_object_weak_ref(G_OBJECT(runtime->test_run_picker), RuntimeTestRunFinalized, request);
    (void)umi_gtk4_automation_tag_widget(runtime->test_run_picker, "studio.tests.evidence-run");
    (void)umi_gtk4_automation_tag_widget(runtime->test_run_summary, "studio.tests.evidence-run-summary");
    gtk_box_append(GTK_BOX(box), gtk_label_new("Evidence run"));
    gtk_box_append(GTK_BOX(box), runtime->test_run_picker);
    gtk_box_append(GTK_BOX(box), runtime->test_run_summary);
    return box;
}
