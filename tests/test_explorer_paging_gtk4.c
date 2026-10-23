/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_explorer_paging_gtk4.c
 * PURPOSE: Exercise real Test Explorer page controls, refresh cancellation and selected execution.
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
static unsigned executed;
static void PageMutate(void)
{
    if (!armed || host == NULL || *host == NULL) return;
    armed = 0; ++observed;
    UmiStudioGtkRuntimeChrome *chrome = (*host)->runtime_chrome;
    if (strstr(mode, "-close")) {
        UmiStudioGtkWorkbench *old = *host; *host = NULL;
        umi_studio_gtk_workbench_destroy(old);
    } else if (strcmp(mode, "query-away-back") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(chrome->test_search_entry), "test.0906");
        gtk_editable_set_text(GTK_EDITABLE(chrome->test_search_entry), "");
    } else if (strcmp(mode, "rows-query") == 0 || strcmp(mode, "query-reenter") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(chrome->test_search_entry), "test.0906");
    } else runtime_tests_refresh_list(chrome);
}
static void PageNotify(GObject *object, GParamSpec *property, gpointer data)
{ (void)object; (void)property; (void)data; PageMutate(); }
static void PageRows(GListModel *model, guint position, guint removed, guint added, gpointer data)
{ (void)model; (void)position; (void)removed; (void)added; (void)data; PageMutate(); }
static void PageEvidenceChanged(GtkTextBuffer *buffer, gpointer data)
{
    (void)buffer; (void)data;
    if (!armed || host == NULL || *host == NULL) return;
    UmiStudioGtkRuntimeChrome *chrome = (*host)->runtime_chrome;
    if (strcmp(chrome->selected_test_id, "test.0001") != 0) return;
    armed = 0; ++observed;
    strcpy(chrome->selected_test_id, "test.0002");
    UmiStudioServices *services = umi_studio_ui_services(chrome->owner->ui);
    (void)umi_test_workspace_select_item(umi_studio_test_service_workspace(
        umi_studio_services_tests(services)), "test.0002");
}
static UmiStatus PageExecution(void *context, const char *argument, char *message, size_t capacity)
{
    (void)context; (void)argument;
    ++executed;
    if (message != NULL && capacity != 0U) g_strlcpy(message, "Fixture accepted selected operation", capacity);
    return UMI_STATUS_OK;
}
static UmiStatus PageItem(UmiTestPlatformItemRegistry *items, size_t index)
{
    UmiTestPlatformItemSnapshot item = {0};
    (void)snprintf(item.id, sizeof item.id, "test.%04zu", index);
    (void)snprintf(item.name, sizeof item.name, "Test %04zu", index);
    strcpy(item.framework, "ctest"); strcpy(item.kind, "test"); item.enabled = 1; item.discovered = 1;
    return umi_test_platform_item_registry_upsert(items, &item);
}
static size_t PageRowCount(GtkWidget *list)
{
    size_t count = 0U;
    for (GtkWidget *row = gtk_widget_get_first_child(list); row != NULL; row = gtk_widget_get_next_sibling(row))
        if (g_object_get_data(G_OBJECT(row), "umicom-test-id") != NULL) ++count;
    return count;
}
static GtkWidget *PageButton(GtkWidget *list, size_t index, const char *label)
{
    GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), (int)index);
    if (row == NULL) return NULL;
    GtkWidget *box = gtk_list_box_row_get_child(row);
    for (GtkWidget *child = gtk_widget_get_first_child(box); child != NULL; child = gtk_widget_get_next_sibling(child))
        if (GTK_IS_BUTTON(child) && strcmp(gtk_button_get_label(GTK_BUTTON(child)), label) == 0) return child;
    return NULL;
}
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *cases[] = {"first", "next", "last", "previous", "filter", "sort", "shrink", "empty", "unchanged", "stale-next", "stale-row", "retained-row", "retained-search", "range-close", "summary-close", "query-reenter", "query-away-back", "rows-close", "rows-reenter", "rows-query", "invalid-bytes", "select", "run", "debug", "run-reselect", "debug-reselect", "source", "failed-only", "disabled", "capacity-message", "jump-last", "jump-first", "jump-stale", "jump-filter", "jump-shrink", "catalogue-navigation", "jump-retained", "jump-close"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(argv[1], cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    mode = argv[1];
    int failed = 0, changed = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "paging-XXXXXX", NULL);
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *list = NULL, *next = NULL, *previous = NULL, *search = NULL, *retained = NULL;
    GObject *observation = NULL; gulong handler = 0U;
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
    application = gtk_application_new("org.umicom.studio.explorer-paging-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    host = &workbench;
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TEST_EXPLORER, NULL) == UMI_STATUS_OK);
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    UmiTestPlatformService *platform = runtime_test_platform(chrome);
    CHECK(platform != NULL);
    UmiTestPlatformItemRegistry *items = umi_test_platform_service_item(platform);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    umi_test_platform_item_registry_clear(items); umi_test_platform_result_registry_clear(results);
    for (size_t i = 0U; i < 907U; ++i) CHECK(PageItem(items, i) == UMI_STATUS_OK);
    runtime_tests_refresh_list(chrome);
    RuntimeTestPageState *state = RuntimeTestPages(chrome);
    CHECK(state != NULL && state->published != NULL);
    list = g_object_ref(chrome->test_list); next = g_object_ref(state->next);
    previous = g_object_ref(state->previous); search = g_object_ref(state->search);
    CHECK(PageRowCount(list) == 300U && state->offset == 0U);
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "of 4") != NULL);
    CHECK(!gtk_widget_get_sensitive(previous) && gtk_widget_get_sensitive(next));
    if (strncmp(mode, "jump-", 5U) == 0 || strcmp(mode, "catalogue-navigation") == 0) {
        GtkWidget *last = g_object_ref(state->last);
        retained = last;
        CHECK(!gtk_widget_get_sensitive(state->first) && gtk_widget_get_sensitive(last));
        if (strcmp(mode, "jump-retained") == 0) {
            umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
            g_signal_emit_by_name(last, "clicked");
        } else if (strcmp(mode, "jump-stale") == 0) {
            CHECK(PageItem(items, 0U) == UMI_STATUS_OK);
            g_signal_emit_by_name(last, "clicked");
            CHECK(state->published != NULL && state->offset == 0U);
        } else if (strcmp(mode, "jump-close") == 0) {
            observation = g_object_ref(G_OBJECT(state->range));
            handler = g_signal_connect(observation, "notify::label", G_CALLBACK(PageNotify), NULL);
            armed = 1;
            g_signal_emit_by_name(last, "clicked");
            CHECK(observed == 1 && workbench == NULL);
        } else if (strcmp(mode, "catalogue-navigation") == 0) {
            umi_test_platform_item_registry_clear(items);
            for (size_t i = 0U; i < 26032U; ++i) {
                UmiTestPlatformItemSnapshot item = {0};
                snprintf(item.id, sizeof item.id, "test.%04zu", i);
                /* Equal-width names make catalogue order independent of the
                 * varying identifier width beyond ten thousand records. */
                snprintf(item.name, sizeof item.name, "Catalogue %05zu", i);
                strcpy(item.framework, "ctest"); strcpy(item.kind, "test");
                item.enabled = item.discovered = 1;
                CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
            }
            runtime_tests_refresh_list(chrome);
            CHECK(state->published != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "of 26032 matching") != NULL);
            g_signal_emit_by_name(last, "clicked");
            CHECK(state->offset == 25800U && PageRowCount(list) == 232U);
            GtkWidget *select = PageButton(list, 231U, "Select"); CHECK(select != NULL);
            g_signal_emit_by_name(select, "clicked");
            CHECK(strcmp(chrome->selected_test_id, "test.26031") == 0);
            g_signal_emit_by_name(state->first, "clicked");
            CHECK(state->offset == 0U && PageRowCount(list) == 300U);
            CHECK(strcmp(g_object_get_data(G_OBJECT(gtk_widget_get_first_child(list)), "umicom-test-id"), "test.0000") == 0);
        } else {
            g_signal_emit_by_name(last, "clicked");
            CHECK(state->offset == 900U && PageRowCount(list) == 7U);
            CHECK(!gtk_widget_get_sensitive(last) && gtk_widget_get_sensitive(state->first));
            if (strcmp(mode, "jump-first") == 0) {
                g_signal_emit_by_name(state->first, "clicked");
                CHECK(state->offset == 0U && PageRowCount(list) == 300U);
            } else if (strcmp(mode, "jump-filter") == 0) {
                gtk_editable_set_text(GTK_EDITABLE(search), "test.0906");
                CHECK(state->offset == 0U && PageRowCount(list) == 1U);
                CHECK(!gtk_widget_get_sensitive(last) && !gtk_widget_get_sensitive(state->first));
            } else if (strcmp(mode, "jump-shrink") == 0) {
                umi_test_platform_item_registry_clear(items);
                CHECK(PageItem(items, 0U) == UMI_STATUS_OK);
                runtime_tests_refresh_list(chrome);
                CHECK(state->published != NULL && state->offset == 0U && PageRowCount(list) == 1U);
            }
        }
    } else if (strcmp(mode, "capacity-message") == 0) {
/* Retain the old 4,096-cap refusal expectation for review. The replacement proves navigation and action identity beyond that former boundary. Retained for engineering review. */
#if 0
        retained = g_object_ref(PageButton(list, 0U, "Select"));
        for (size_t i = 907U; i <= UMI_TEST_EXPLORER_MODEL_CAPACITY; ++i)
            CHECK(PageItem(items, i) == UMI_STATUS_OK);
        runtime_tests_refresh_list(chrome);
        CHECK(state->published == NULL);
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "4097 discovered tests") != NULL);
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "limit of 4096") != NULL);
        strcpy(chrome->selected_test_id, "test.0002");
        g_signal_emit_by_name(retained, "clicked");
        CHECK(strcmp(chrome->selected_test_id, "test.0002") == 0);

#endif
        /* The former refusal case is now a positive regression at 4,097 rows.
         * Retained old controls still must not target the new catalogue. */
        retained = g_object_ref(PageButton(list, 0U, "Select"));
        for (size_t i = 907U; i <= UMI_TEST_EXPLORER_MODEL_CAPACITY; ++i)
            CHECK(PageItem(items, i) == UMI_STATUS_OK);
        runtime_tests_refresh_list(chrome);
        CHECK(state->published != NULL);
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "of 4097 matching") != NULL);
        strcpy(chrome->selected_test_id, "test.0002");
        g_signal_emit_by_name(retained, "clicked");
        CHECK(strcmp(chrome->selected_test_id, "test.0002") == 0);
        for (size_t i = 0U; i < 13U; ++i) g_signal_emit_by_name(next, "clicked");
        CHECK(state->offset == 3900U && PageRowCount(list) == 197U);
        GtkWidget *last = PageButton(list, 196U, "Select"); CHECK(last != NULL);
        g_signal_emit_by_name(last, "clicked");
        CHECK(strcmp(chrome->selected_test_id, "test.4096") == 0);
    } else if (strcmp(mode, "first") == 0) {
        CHECK(strcmp(g_object_get_data(G_OBJECT(gtk_widget_get_first_child(list)), "umicom-test-id"), "test.0000") == 0);
    } else if (strcmp(mode, "unchanged") == 0) {
        retained = g_object_ref(gtk_widget_get_first_child(list));
        runtime_tests_refresh_list(chrome);
        CHECK(gtk_widget_get_first_child(list) == retained);
    } else if (strcmp(mode, "stale-row") == 0) {
        retained = g_object_ref(PageButton(list, 0U, "Select"));
        CHECK(PageItem(items, 0U) == UMI_STATUS_OK);
        strcpy(chrome->selected_test_id, "test.0002");
        g_signal_emit_by_name(retained, "clicked");
        CHECK(strcmp(chrome->selected_test_id, "test.0002") == 0);
    } else if (strcmp(mode, "retained-search") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        gtk_editable_set_text(GTK_EDITABLE(search), "after close");
        g_signal_emit_by_name(next, "clicked");
    } else if (strstr(mode, "-close") || strcmp(mode, "query-reenter") == 0 ||
               strcmp(mode, "query-away-back") == 0 || strncmp(mode, "rows-", 5U) == 0) {
        if (strncmp(mode, "rows-", 5U) == 0) {
            observation = G_OBJECT(gtk_widget_observe_children(list));
            handler = g_signal_connect(observation, "items-changed", G_CALLBACK(PageRows), NULL);
        } else {
            observation = g_object_ref(G_OBJECT(strcmp(mode, "summary-close") == 0 ? state->summary : state->range));
            handler = g_signal_connect(observation, "notify::label", G_CALLBACK(PageNotify), NULL);
        }
        armed = 1;
        if (strcmp(mode, "summary-close") == 0) {
            CHECK(umi_test_platform_item_registry_remove(items, "test.0906") == UMI_STATUS_OK);
            runtime_tests_refresh_list(chrome);
        } else g_signal_emit_by_name(next, "clicked");
        CHECK(observed == 1);
        if (strstr(mode, "-close")) CHECK(workbench == NULL);
        else {
            CHECK(workbench != NULL);
            runtime_tests_refresh_list(workbench->runtime_chrome);
            state = RuntimeTestPages(workbench->runtime_chrome);
            CHECK(state->published != NULL);
            if (strcmp(mode, "query-reenter") == 0 || strcmp(mode, "rows-query") == 0)
                CHECK(PageRowCount(list) == 1U && state->offset == 0U);
            else if (strcmp(mode, "query-away-back") == 0) CHECK(PageRowCount(list) == 300U && state->offset == 0U);
            else CHECK(state->offset == 300U);
        }
    } else if (strcmp(mode, "invalid-bytes") == 0) {
        UmiTestPlatformItemSnapshot item;
        CHECK(umi_test_platform_item_registry_find(items, "test.0000", &item) == UMI_STATUS_OK);
        item.name[0] = (char)0xff;
        CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
        runtime_tests_refresh_list(chrome);
        CHECK(RuntimeTestPages(chrome)->published != NULL);
    } else if (strcmp(mode, "source") == 0) {
        char *source = g_build_filename(root, "paging-source.c", NULL);
        CHECK(g_file_set_contents(source, "first\nsecond\n", -1, NULL));
        UmiTestPlatformItemSnapshot item;
        CHECK(umi_test_platform_item_registry_find(items, "test.0906", &item) == UMI_STATUS_OK);
        g_strlcpy(item.source_uri, source, sizeof item.source_uri);
        g_strlcpy(item.working_directory, root, sizeof item.working_directory);
        item.source_line = 2U;
        CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
        g_free(source);
        runtime_tests_refresh_list(chrome);
        for (size_t i = 0U; i < 3U; ++i) g_signal_emit_by_name(next, "clicked");
        CHECK(state->offset == 900U);
        GtkWidget *open = PageButton(list, 6U, "Source"); CHECK(open != NULL);
        g_signal_emit_by_name(open, "clicked");
        UmiDocumentWorkingCopySnapshot working;
        UmiDocumentCoordinator *documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
        CHECK(umi_document_coordinator_active_snapshot(documents, &working) == UMI_STATUS_OK);
        UmiUiDocumentViewSnapshot active;
        UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(umi_studio_bootstrap_ui(bootstrap)));
        CHECK(umi_ui_document_view_model_find(views, working.view_id, &active) == UMI_STATUS_OK && active.cursor_offset == 6U);
    } else if (strcmp(mode, "failed-only") == 0) {
        const int outcomes[] = {UMI_TEST_PLATFORM_OUTCOME_PASSED, UMI_TEST_PLATFORM_OUTCOME_FAILED,
            UMI_TEST_PLATFORM_OUTCOME_SKIPPED, UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT};
        for (size_t i = 0U; i < 4U; ++i) {
            UmiTestPlatformResultSnapshot result = {0};
            (void)snprintf(result.id, sizeof result.id, "result.%zu", i);
            (void)snprintf(result.item_id, sizeof result.item_id, "test.%04zu", i);
            result.outcome = outcomes[i]; result.sequence = i + 1U;
            CHECK(umi_test_platform_result_registry_upsert(results, &result) == UMI_STATUS_OK);
        }
        runtime_tests_refresh_list(chrome);
        g_signal_emit_by_name(next, "clicked");
        CHECK(state->offset == 300U);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->failed), TRUE);
        CHECK(state->offset == 0U && PageRowCount(list) == 2U);
        CHECK(strcmp(g_object_get_data(G_OBJECT(gtk_widget_get_first_child(list)), "umicom-test-id"), "test.0001") == 0);
    } else if (strcmp(mode, "disabled") == 0) {
        UmiTestPlatformItemSnapshot item;
        CHECK(umi_test_platform_item_registry_find(items, "test.0001", &item) == UMI_STATUS_OK);
        item.enabled = 0; CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
        runtime_tests_refresh_list(chrome);
        strcpy(chrome->selected_test_id, "test.0002");
        GtkWidget *run = PageButton(list, 1U, "Run"); CHECK(run != NULL && !gtk_widget_get_sensitive(run));
        g_signal_emit_by_name(run, "clicked");
        CHECK(strcmp(chrome->selected_test_id, "test.0002") == 0);
    } else if (strcmp(mode, "select") == 0 || strcmp(mode, "run") == 0 || strcmp(mode, "debug") == 0 ||
               strcmp(mode, "run-reselect") == 0 || strcmp(mode, "debug-reselect") == 0) {
        UmiCommandRegistry *registry = umi_studio_bootstrap_command_registry(bootstrap);
        const char *id = strstr(mode, "debug") ? UMI_STUDIO_COMMAND_TESTS_DEBUG_SELECTED : UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED;
        CHECK(umi_command_registry_unregister(registry, id) == UMI_STATUS_OK);
        UmiCommandDescriptor descriptor = {0};
        descriptor.structure_size = (uint32_t)sizeof descriptor;
        descriptor.command_id = id; descriptor.title = "Paging fixture operation";
        descriptor.category = "Qualification"; descriptor.description = "Accept without a child process";
        descriptor.handler = PageExecution;
        CHECK(umi_command_registry_register(registry, &descriptor) == UMI_STATUS_OK);
        if (strstr(mode, "reselect")) {
            observation = g_object_ref(G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(chrome->test_selected_view))));
            handler = g_signal_connect(observation, "changed", G_CALLBACK(PageEvidenceChanged), NULL);
            armed = 1;
        }
        retained = g_object_ref(PageButton(list, 1U, strcmp(mode, "select") == 0 ? "Select" : strstr(mode, "debug") ? "Debug" : "Run"));
        g_signal_emit_by_name(retained, "clicked");
        if (strstr(mode, "reselect")) CHECK(observed == 1 && executed == 0U);
        else if (strcmp(mode, "select") == 0) CHECK(strcmp(chrome->selected_test_id, "test.0001") == 0 && executed == 0U);
        else CHECK(executed == 1U);
    } else {
        if (strcmp(mode, "stale-next") == 0) {
            CHECK(PageItem(items, 907U) == UMI_STATUS_OK);
            g_signal_emit_by_name(next, "clicked");
            CHECK(state->offset == 0U);
        } else {
            retained = g_object_ref(PageButton(list, 0U, "Select"));
            g_signal_emit_by_name(next, "clicked");
            CHECK(state->offset == 300U && PageRowCount(list) == 300U);
            if (strcmp(mode, "last") == 0 || strcmp(mode, "shrink") == 0) {
                g_signal_emit_by_name(next, "clicked"); g_signal_emit_by_name(next, "clicked");
                CHECK(state->offset == 900U && PageRowCount(list) == 7U && !gtk_widget_get_sensitive(next));
                CHECK(strcmp(g_object_get_data(G_OBJECT(gtk_widget_get_first_child(list)), "umicom-test-id"), "test.0900") == 0);
                if (strcmp(mode, "shrink") == 0) {
                    umi_test_platform_item_registry_clear(items);
                    CHECK(PageItem(items, 10U) == UMI_STATUS_OK);
                    runtime_tests_refresh_list(chrome);
                    CHECK(state->offset == 0U && PageRowCount(list) == 1U);
                }
            } else if (strcmp(mode, "previous") == 0) {
                g_signal_emit_by_name(previous, "clicked");
                CHECK(state->offset == 0U);
            } else if (strcmp(mode, "filter") == 0) {
                gtk_editable_set_text(GTK_EDITABLE(search), "test.0906");
                CHECK(state->offset == 0U && PageRowCount(list) == 1U);
            } else if (strcmp(mode, "sort") == 0) {
                gtk_drop_down_set_selected(GTK_DROP_DOWN(state->sort), 1U);
                CHECK(state->offset == 0U && state->published != NULL);
            } else if (strcmp(mode, "empty") == 0) {
                gtk_editable_set_text(GTK_EDITABLE(search), "no-matching-test");
                CHECK(state->offset == 0U && PageRowCount(list) == 0U);
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(state->range)), "Page 0 of 0") != NULL);
            } else if (strcmp(mode, "retained-row") == 0) {
                strcpy(chrome->selected_test_id, "test.0300");
                g_signal_emit_by_name(retained, "clicked");
                CHECK(strcmp(chrome->selected_test_id, "test.0300") == 0);
            } else CHECK(strcmp(mode, "next") == 0);
        }
    }
cleanup:
    armed = 0; host = NULL;
    if (handler != 0U) g_signal_handler_disconnect(observation, handler);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&list); g_clear_object(&next); g_clear_object(&previous);
    g_clear_object(&search); g_clear_object(&retained); g_clear_object(&observation);
    if (changed) (void)g_chdir(original);
    if (isolated) { if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE); else g_unsetenv("UMICOM_STUDIO_DATA_PATH"); }
    g_free(original); g_free(oldData); g_free(root);
    return failed;
}
