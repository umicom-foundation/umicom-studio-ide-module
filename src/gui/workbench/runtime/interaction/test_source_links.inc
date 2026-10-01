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
static void RuntimeTestLinksClear(UmiStudioGtkRuntimeChrome *runtime, const char *message)
{
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(runtime->test_failure_links)) != NULL)
        gtk_box_remove(GTK_BOX(runtime->test_failure_links), child);
    g_object_set_data_full(G_OBJECT(runtime->test_failure_links), "umicom-test-link-stamp", NULL, NULL);
    gtk_label_set_text(GTK_LABEL(runtime->test_failure_summary), message);
}
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
