/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/test_source_links.inc
 * PURPOSE: Present Framework source locations without borrowing retained evidence or native owners.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/test_ui/source_navigation.h"
#include "umicom/platform/path.h"
typedef struct UmiStudioTestLinkStamp {
    char item_id[128];
    uint64_t item_revision, result_revision, output_revision;
    int have_item;
} UmiStudioTestLinkStamp;
typedef struct UmiStudioTestLinkAction {
    UmiStudioTestLinkStamp stamp;
    UmiTestSourceLink link;
    char base[1024];
} UmiStudioTestLinkAction;
static int RuntimeTestLinkStampEqual(const UmiStudioTestLinkStamp *a, const UmiStudioTestLinkStamp *b)
{
    return strcmp(a->item_id, b->item_id) == 0 && a->have_item == b->have_item &&
        a->item_revision == b->item_revision && a->result_revision == b->result_revision &&
        a->output_revision == b->output_revision;
}
/* Native row removal and summary notification can close Studio. Retain the pane objects and reacquire their application before continuing; the original loop remains for review. */
#if 0
static void RuntimeTestLinksClear(UmiStudioGtkRuntimeChrome *runtime, const char *message)
{
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(runtime->test_failure_links)) != NULL)
        gtk_box_remove(GTK_BOX(runtime->test_failure_links), child);
    g_object_set_data_full(G_OBJECT(runtime->test_failure_links), "umicom-test-link-stamp", NULL, NULL);
    gtk_label_set_text(GTK_LABEL(runtime->test_failure_summary), message);
}

#endif
static int RuntimeTestLinksClear(UmiStudioGtkRuntimeChrome *runtime, const char *message)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return 0;
    GtkWidget *list = g_object_ref(runtime->test_failure_links);
    GtkWidget *summary = g_object_ref(runtime->test_failure_summary);
    GObject *window = NULL;
    int current = 0;
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(list)) != NULL) {
        gtk_box_remove(GTK_BOX(list), child);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL || runtime->test_failure_links != list || runtime->test_failure_summary != summary) goto cleanup;
    }
    g_object_set_data_full(G_OBJECT(list), "umicom-test-link-stamp", NULL, NULL);
    gtk_label_set_text(GTK_LABEL(summary), message);
    runtime = RuntimeResolveOwner(request, &window);
    current = runtime != NULL && runtime->test_failure_links == list && runtime->test_failure_summary == summary;
cleanup:
    g_clear_object(&window); g_object_unref(list); g_object_unref(summary);
    runtime_file_dialog_request_free(request);
    return current;
}
/* Explicit test-source navigation now reveals the native editor and checks host lifetime after document notifications. The former refresh-only action remains for review. */
#if 0
static void RuntimeTestLinkOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    const UmiStudioTestLinkAction *action = g_object_get_data(G_OBJECT(button), "umicom-test-link");
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    UmiTestPlatformItemSnapshot current;
    if (action == NULL || platform == NULL || strcmp(runtime->selected_test_id, action->stamp.item_id) != 0 ||
        umi_test_platform_item_registry_find(umi_test_platform_service_item(platform), action->stamp.item_id, &current) != UMI_STATUS_OK ||
        current.revision != action->stamp.item_revision ||
        umi_test_platform_result_registry_revision(umi_test_platform_service_result(platform)) != action->stamp.result_revision ||
        umi_test_platform_output_registry_revision(umi_test_platform_service_output(platform)) != action->stamp.output_revision) {
        runtime_show_notification(runtime, "Test evidence changed. Refresh and choose its current source location again."); return;
    }
    UmiStatus status = UmiTestSourceLinkOpen(umi_studio_ui_documents(runtime->owner->ui),
        &action->link, action->base, NULL);
    /* Refresh can remove the clicked row: do not borrow action afterwards. */
    (void)runtime_refresh_services(runtime);
    if (status != UMI_STATUS_OK) {
        char message[192];
        (void)snprintf(message, sizeof(message), "Test source location was not reached: %s. No text was changed.", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
}

#endif
static void RuntimeTestLinkOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    const UmiStudioTestLinkAction *borrowed = g_object_get_data(G_OBJECT(button), "umicom-test-link");
    if (runtime == NULL || runtime->owner == NULL || borrowed == NULL) return;
    /* Opening a document can rebuild the Test Explorer. Own the selected
     * evidence while Framework resolves it, independently of its button row. */
    UmiStudioTestLinkAction action = *borrowed;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    UmiTestPlatformItemSnapshot current;
    if (platform == NULL || strcmp(runtime->selected_test_id, action.stamp.item_id) != 0 ||
        umi_test_platform_item_registry_find(umi_test_platform_service_item(platform), action.stamp.item_id, &current) != UMI_STATUS_OK ||
        current.revision != action.stamp.item_revision ||
        umi_test_platform_result_registry_revision(umi_test_platform_service_result(platform)) != action.stamp.result_revision ||
        umi_test_platform_output_registry_revision(umi_test_platform_service_output(platform)) != action.stamp.output_revision) {
        runtime_show_notification(runtime, "Test evidence changed. Refresh and choose its current source location again."); return;
    }
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return;
    UmiStatus status = UmiTestSourceLinkOpen(umi_studio_ui_documents(runtime->owner->ui),
        &action.link, action.base, NULL);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status == UMI_STATUS_OK) status = RuntimePresentActiveSource(runtime);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK) {
        char message[192];
        (void)snprintf(message, sizeof message, "Test source location was not reached: %s. No text was changed.", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* Evidence and native ownership can change during pane construction. The replacement owns its objects, reserves the refresh and checks the selected capture between native updates; retain the original renderer for review. */
#if 0
static void RuntimeFailureLocationsRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->test_failure_links == NULL || runtime->test_failure_summary == NULL) return;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (runtime->selected_test_id[0] == '\0' || platform == NULL) {
        RuntimeTestLinksClear(runtime, "Select a test to inspect source locations in its retained evidence."); return;
    }
    UmiTestPlatformItemSnapshot item = {0};
    UmiStudioTestLinkStamp stamp = {0};
    g_strlcpy(stamp.item_id, runtime->selected_test_id, sizeof(stamp.item_id));
    stamp.have_item = umi_test_platform_item_registry_find(umi_test_platform_service_item(platform), stamp.item_id, &item) == UMI_STATUS_OK;
    stamp.item_revision = item.revision;
    stamp.result_revision = umi_test_platform_result_registry_revision(umi_test_platform_service_result(platform));
    stamp.output_revision = umi_test_platform_output_registry_revision(umi_test_platform_service_output(platform));
    const UmiStudioTestLinkStamp *previous = g_object_get_data(G_OBJECT(runtime->test_failure_links), "umicom-test-link-stamp");
    /* Preserve keyboard focus and button identity during unchanged timer ticks. */
    if (previous != NULL && RuntimeTestLinkStampEqual(previous, &stamp)) return;
    UmiTestEvidence *evidence = NULL;
    UmiTestSourceLinks *links = NULL;
    UmiStatus status = UmiTestEvidenceCreate(umi_test_platform_service_result(platform),
        umi_test_platform_service_output(platform), stamp.item_id, NULL, &evidence);
    if (status == UMI_STATUS_OK) status = UmiTestSourceLinksCreate(evidence, &links);
    UmiTestEvidenceDestroy(evidence);
    if (status != UMI_STATUS_OK) {
        RuntimeTestLinksClear(runtime, "Source locations are unavailable. The original evidence remains in Selected test."); return;
    }
    UmiTestSourceLinksSummary summary;
    (void)UmiTestSourceLinksGetSummary(links, &summary);
    char *heading = g_strdup_printf(
        "%zu source locations | %zu duplicates | %zu unparsed lines | %zu capacity omissions.\n"
        "Evidence retained: %zu/%zu results, %zu/%zu output records.\n"
        "Locations may describe earlier runs. Relative paths use the current test working directory shown below; verify it before opening.",
        summary.retained_links, summary.duplicate_lines, summary.unparsed_lines, summary.capacity_dropped,
        summary.evidence.retained_results, summary.evidence.matching_results,
        summary.evidence.retained_output, summary.evidence.matching_output);
    RuntimeTestLinksClear(runtime, heading); g_free(heading);
    UmiStudioTestLinkStamp *cached = g_new(UmiStudioTestLinkStamp, 1); *cached = stamp;
    g_object_set_data_full(G_OBJECT(runtime->test_failure_links), "umicom-test-link-stamp", cached, g_free);
    for (size_t i = 0U; i < summary.retained_links; ++i) {
        UmiStudioTestLinkAction *action = g_new0(UmiStudioTestLinkAction, 1);
        (void)UmiTestSourceLinksAt(links, i, &action->link);
        action->stamp = stamp;
        g_strlcpy(action->base, item.working_directory, sizeof(action->base));
        int relative = !umi_path_is_absolute(action->link.location.path);
        int canOpen = stamp.have_item && (!relative || umi_path_is_absolute(action->base));
        char *raw = g_strdup_printf("%s:%zu:%zu\nRun: %s | %s\n%s%s",
            action->link.location.path, action->link.location.line, action->link.location.column,
            action->link.session_id[0] != '\0' ? action->link.session_id : "unspecified",
            action->link.origin == UMI_TEST_SOURCE_OUTPUT ? "output" : "result",
            relative ? "Current working directory: " : "Absolute source path",
            relative ? (action->base[0] != '\0' ? action->base : "unavailable; cannot open") : "");
        char *display = g_utf8_make_valid(raw, -1); g_free(raw);
        GtkWidget *button = gtk_button_new();
        GtkWidget *label = gtk_label_new(display); g_free(display);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0F);
        gtk_button_set_child(GTK_BUTTON(button), label);
        char *tip = g_utf8_make_valid(action->link.location.message, -1);
        gtk_widget_set_tooltip_text(button, tip); g_free(tip);
        g_object_set_data_full(G_OBJECT(button), "umicom-test-link", action, g_free);
        char tag[80]; (void)snprintf(tag, sizeof(tag), "studio.tests.source-location.%zu", i);
        (void)umi_gtk4_automation_tag_widget(button, tag);
        gtk_widget_set_sensitive(button, canOpen);
        runtime_bind_editor_action(runtime, button, RuntimeTestLinkOpen);
        gtk_box_append(GTK_BOX(runtime->test_failure_links), button);
    }
    if (summary.retained_links == 0U)
        gtk_box_append(GTK_BOX(runtime->test_failure_links), gtk_label_new(
            "No supported file-and-line reference was found. Read the original Selected test evidence."));
    UmiTestSourceLinksDestroy(links);
}

#endif
/* Reentrant selection or evidence publication invalidates the remaining rows,
 * even when the same native pane survives. Never mix two captures in one list. */
static UmiStudioGtkRuntimeChrome *RuntimeTestLinkPaneCurrent(UmiStudioFileDialogRequest *request,
    GObject **window, GtkWidget *list, GtkWidget *summary, const UmiStudioTestLinkStamp *stamp)
{
    UmiStudioGtkRuntimeChrome *runtime = RuntimeResolveOwner(request, window);
    if (runtime == NULL || runtime->test_failure_links != list || runtime->test_failure_summary != summary ||
        strcmp(runtime->selected_test_id, stamp->item_id) != 0) return NULL;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    UmiTestPlatformItemSnapshot item = {0};
    if (platform == NULL) return NULL;
    int haveItem = umi_test_platform_item_registry_find(umi_test_platform_service_item(platform), stamp->item_id, &item) == UMI_STATUS_OK;
    if (haveItem != stamp->have_item || item.revision != stamp->item_revision ||
        umi_test_platform_result_registry_revision(umi_test_platform_service_result(platform)) != stamp->result_revision ||
        umi_test_platform_output_registry_revision(umi_test_platform_service_output(platform)) != stamp->output_revision) return NULL;
    return runtime;
}

static void RuntimeFailureLocationsRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->test_failure_links == NULL || runtime->test_failure_summary == NULL) return;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return;
    GtkWidget *list = g_object_ref(runtime->test_failure_links), *summaryLabel = g_object_ref(runtime->test_failure_summary);
    GObject *window = NULL;
    UmiTestEvidence *evidence = NULL;
    UmiTestSourceLinks *links = NULL;
    int reserved = 0;
    if (g_object_get_data(G_OBJECT(list), "umicom-test-links-refreshing") != NULL) goto cleanup;
    g_object_set_data(G_OBJECT(list), "umicom-test-links-refreshing", GINT_TO_POINTER(1));
    reserved = 1;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (runtime->selected_test_id[0] == '\0' || platform == NULL) {
        (void)RuntimeTestLinksClear(runtime, "Select a test to inspect source locations in its retained evidence."); goto cleanup;
    }
    UmiTestPlatformItemSnapshot item = {0};
    UmiStudioTestLinkStamp stamp = {0};
    g_strlcpy(stamp.item_id, runtime->selected_test_id, sizeof(stamp.item_id));
    stamp.have_item = umi_test_platform_item_registry_find(umi_test_platform_service_item(platform), stamp.item_id, &item) == UMI_STATUS_OK;
    stamp.item_revision = item.revision;
    stamp.result_revision = umi_test_platform_result_registry_revision(umi_test_platform_service_result(platform));
    stamp.output_revision = umi_test_platform_output_registry_revision(umi_test_platform_service_output(platform));
    const UmiStudioTestLinkStamp *previous = g_object_get_data(G_OBJECT(runtime->test_failure_links), "umicom-test-link-stamp");
    /* Preserve keyboard focus and button identity during unchanged timer ticks. */
    if (previous != NULL && RuntimeTestLinkStampEqual(previous, &stamp)) goto cleanup;
    UmiStatus status = UmiTestEvidenceCreate(umi_test_platform_service_result(platform),
        umi_test_platform_service_output(platform), stamp.item_id, NULL, &evidence);
    if (status == UMI_STATUS_OK) status = UmiTestSourceLinksCreate(evidence, &links);
    UmiTestEvidenceDestroy(evidence); evidence = NULL;
    if (status != UMI_STATUS_OK) {
        (void)RuntimeTestLinksClear(runtime, "Source locations are unavailable. The original evidence remains in Selected test."); goto cleanup;
    }
    UmiTestSourceLinksSummary summary;
    (void)UmiTestSourceLinksGetSummary(links, &summary);
    char *heading = g_strdup_printf(
        "%zu source locations | %zu duplicates | %zu unparsed lines | %zu capacity omissions.\n"
        "Evidence retained: %zu/%zu results, %zu/%zu output records.\n"
        "Locations may describe earlier runs. Relative paths use the current test working directory shown below; verify it before opening.",
        summary.retained_links, summary.duplicate_lines, summary.unparsed_lines, summary.capacity_dropped,
        summary.evidence.retained_results, summary.evidence.matching_results,
        summary.evidence.retained_output, summary.evidence.matching_output);
    int cleared = RuntimeTestLinksClear(runtime, heading); g_free(heading);
    if (!cleared) goto cleanup;
    runtime = RuntimeTestLinkPaneCurrent(request, &window, list, summaryLabel, &stamp);
    if (runtime == NULL) goto cleanup;
    for (size_t i = 0U; i < summary.retained_links; ++i) {
        UmiStudioTestLinkAction *action = g_new0(UmiStudioTestLinkAction, 1);
        (void)UmiTestSourceLinksAt(links, i, &action->link);
        action->stamp = stamp;
        g_strlcpy(action->base, item.working_directory, sizeof(action->base));
        int relative = !umi_path_is_absolute(action->link.location.path);
        int canOpen = stamp.have_item && (!relative || umi_path_is_absolute(action->base));
        char *raw = g_strdup_printf("%s:%zu:%zu\nRun: %s | %s\n%s%s",
            action->link.location.path, action->link.location.line, action->link.location.column,
            action->link.session_id[0] != '\0' ? action->link.session_id : "unspecified",
            action->link.origin == UMI_TEST_SOURCE_OUTPUT ? "output" : "result",
            relative ? "Current working directory: " : "Absolute source path",
            relative ? (action->base[0] != '\0' ? action->base : "unavailable; cannot open") : "");
        char *display = g_utf8_make_valid(raw, -1); g_free(raw);
        GtkWidget *button = g_object_ref_sink(gtk_button_new());
        GtkWidget *label = gtk_label_new(display); g_free(display);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0F);
        gtk_button_set_child(GTK_BUTTON(button), label);
        char *tip = g_utf8_make_valid(action->link.location.message, -1);
        gtk_widget_set_tooltip_text(button, tip); g_free(tip);
        g_object_set_data_full(G_OBJECT(button), "umicom-test-link", action, g_free);
        char tag[80]; (void)snprintf(tag, sizeof(tag), "studio.tests.source-location.%zu", i);
        (void)umi_gtk4_automation_tag_widget(button, tag);
        gtk_widget_set_sensitive(button, canOpen);
        runtime = RuntimeTestLinkPaneCurrent(request, &window, list, summaryLabel, &stamp);
        if (runtime == NULL) { g_object_unref(button); goto cleanup; }
        runtime_bind_editor_action(runtime, button, RuntimeTestLinkOpen);
        gtk_box_append(GTK_BOX(list), button);
        g_object_unref(button);
        runtime = RuntimeTestLinkPaneCurrent(request, &window, list, summaryLabel, &stamp);
        if (runtime == NULL) goto cleanup;
    }
    if (summary.retained_links == 0U) {
        GtkWidget *empty = g_object_ref_sink(gtk_label_new(
            "No supported file-and-line reference was found. Read the original Selected test evidence."));
        runtime = RuntimeTestLinkPaneCurrent(request, &window, list, summaryLabel, &stamp);
        if (runtime != NULL) gtk_box_append(GTK_BOX(list), empty);
        g_object_unref(empty);
    }
    runtime = RuntimeTestLinkPaneCurrent(request, &window, list, summaryLabel, &stamp);
    if (runtime != NULL) {
        UmiStudioTestLinkStamp *cached = g_new(UmiStudioTestLinkStamp, 1); *cached = stamp;
        g_object_set_data_full(G_OBJECT(list), "umicom-test-link-stamp", cached, g_free);
    }
cleanup:
    if (reserved) g_object_set_data(G_OBJECT(list), "umicom-test-links-refreshing", NULL);
    UmiTestEvidenceDestroy(evidence); UmiTestSourceLinksDestroy(links);
    g_clear_object(&window); g_object_unref(list); g_object_unref(summaryLabel);
    runtime_file_dialog_request_free(request);
}
