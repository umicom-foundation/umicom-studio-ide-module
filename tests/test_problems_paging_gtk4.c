/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_problems_paging_gtk4.c
 * PURPOSE: Exercise diagnostic pagination through the production GTK Problems surface.
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
static UmiStudioGtkWorkbench **testHost;
static UmiDiagnosticModel *testModel;
static GtkWidget *testSearch;
static const char *testMode;
static int armed, mutations;
static void CloseProblemsHost(void)
{
    if (testHost != NULL && *testHost != NULL) {
        UmiStudioGtkWorkbench *old = *testHost; *testHost = NULL;
        umi_studio_gtk_workbench_destroy(old);
    }
}
static void RefreshProblems(void)
{
    if (testHost != NULL && *testHost != NULL) runtime_problems_refresh_list((*testHost)->runtime_chrome);
}
static void PagePropertyChanged(GObject *object, GParamSpec *property, gpointer data)
{
    (void)object; (void)property; (void)data;
    if (!armed) return;
    armed = 0; ++mutations;
    if (strstr(testMode, "-close") != NULL) CloseProblemsHost();
    else if (strcmp(testMode, "range-reenter") == 0) RefreshProblems();
    else {
        gtk_editable_set_text(GTK_EDITABLE(testSearch), "tail-only");
        if (strcmp(testMode, "range-return") == 0) gtk_editable_set_text(GTK_EDITABLE(testSearch), "");
    }
}
static GtkListBoxRow *FindProblem(GtkWidget *list, const char *id)
{
    for (int index = 0; ; ++index) {
        GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), index);
        if (row == NULL) return NULL;
        const char *actual = g_object_get_data(G_OBJECT(row), "umicom-problem-id");
        if (actual != NULL && strcmp(actual, id) == 0) return row;
    }
}
static size_t ProblemCount(GtkWidget *list)
{
    size_t count = 0U;
    for (int i = 0; ; ++i) {
        GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), i);
        if (row == NULL) return count;
        if (g_object_get_data(G_OBJECT(row), "umicom-problem-id") != NULL) ++count;
    }
}
static GtkWidget *ProblemEditor(GtkWidget *root, const char *view)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (id != NULL && strcmp(id, view) == 0) {
        GtkWidget *editor = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
        if (GTK_IS_TEXT_VIEW(editor)) return editor;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = ProblemEditor(child, view); if (found != NULL) return found;
    }
    return NULL;
}
static UmiStatus AddProblem(const char *id, UmiDiagnosticSeverity severity, const char *message, const char *source)
{
    UmiDiagnosticSnapshot item;
    UmiStatus status = umi_diagnostic_snapshot_init(&item, id, severity,
        UMI_DIAGNOSTIC_KIND_COMPILER, "qualification", message);
    if (status != UMI_STATUS_OK) return status;
    g_strlcpy(item.uri, source, sizeof item.uri); item.line = 2U; item.column = 3U;
    return umi_diagnostic_model_upsert(testModel, &item);
}

static int ProblemTextIsValid(GtkWidget *widget)
{
    if (GTK_IS_LABEL(widget) && !g_utf8_validate(gtk_label_get_text(GTK_LABEL(widget)), -1, NULL)) return 0;
    const char *tooltip = gtk_widget_get_tooltip_text(widget);
    if (tooltip != NULL && !g_utf8_validate(tooltip, -1, NULL)) return 0;
    for (GtkWidget *child = gtk_widget_get_first_child(widget); child != NULL; child = gtk_widget_get_next_sibling(child))
        if (!ProblemTextIsValid(child)) return 0;
    return 1;
}
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%d: %s\n", __LINE__, #expr); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *cases[] = {"first", "next", "last", "previous", "filter", "sort", "severity", "resolved", "shrink", "clear", "empty", "unchanged", "stale-next", "retained-row", "retained-controls", "source", "range-close", "previous-close", "next-close", "range-query", "range-return", "range-reenter", "invalid-bytes"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(argv[1], cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    testMode = argv[1];
    int failed = 0, changedDirectory = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "problems-paging-XXXXXX", NULL), *source = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *list = NULL, *range = NULL, *previous = NULL, *next = NULL, *sort = NULL;
    GtkWidget *severity = NULL, *resolved = NULL, *clear = NULL;
    GtkListBoxRow *oldRow = NULL;
    GObject *observation = NULL;
    gulong handler = 0U;
    CHECK(g_mkdtemp(root) != NULL && g_chdir(root) == 0); changedDirectory = 1;
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
    application = gtk_application_new("org.umicom.studio.problems-paging-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    testHost = &workbench;
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_PROBLEMS, NULL) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
    testModel = umi_diagnostic_pipeline_model(umi_studio_services_diagnostic_pipeline(services));
    umi_diagnostic_model_clear(testModel);
    source = g_build_filename(root, "compiler source.c", NULL);
    CHECK(g_file_set_contents(source, "first\nsecond\nthird\n", -1, NULL));

    for (unsigned i = 0U; i < 907U; ++i) {
        char id[64]; snprintf(id, sizeof id, "problem.%04u", i);
        CHECK(AddProblem(id, i % 2U == 0U ? UMI_DIAGNOSTIC_ERROR : UMI_DIAGNOSTIC_WARNING,
            i >= 900U ? "tail-only" : "compiler diagnostic", source) == UMI_STATUS_OK);
    }
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    CHECK(GTK_IS_LIST_BOX(chrome->problems_list));
    list = g_object_ref(chrome->problems_list);
    testSearch = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-search"));
    sort = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-sort"));
    severity = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-severity"));
    resolved = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-resolved"));
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *widget = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.clear");
    CHECK(GTK_IS_BUTTON(widget)); clear = g_object_ref(widget);
    widget = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.previous-page");
    CHECK(GTK_IS_BUTTON(widget)); previous = g_object_ref(widget);
    widget = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.next-page");
    CHECK(GTK_IS_BUTTON(widget)); next = g_object_ref(widget);
    widget = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.page-range");
    CHECK(GTK_IS_LABEL(widget)); range = g_object_ref(widget);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 2U); /* Oldest first gives stable IDs for source checks. */
    RefreshProblems();
    CHECK(ProblemCount(list) == 300U && FindProblem(list, "problem.0000") != NULL);
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 4") != NULL);
    CHECK(!gtk_widget_get_sensitive(previous) && gtk_widget_get_sensitive(next));
    oldRow = g_object_ref(FindProblem(list, "problem.0000"));
    if (strcmp(testMode, "invalid-bytes") == 0) {
        UmiDiagnosticSnapshot item;
        CHECK(umi_diagnostic_snapshot_init(&item, "invalid-display", UMI_DIAGNOSTIC_ERROR,
            UMI_DIAGNOSTIC_KIND_COMPILER, "cc", "message") == UMI_STATUS_OK);
        item.source[0] = (char)0xff; item.source[1] = '\0';
        item.message[0] = (char)0xfe; item.message[1] = '\0';
        item.code[0] = (char)0xff; item.code[1] = '\0';
        item.detail[0] = (char)0xff; item.detail[1] = '\0';
        item.uri[0] = (char)0xff; item.uri[1] = '\0';
        CHECK(umi_diagnostic_model_upsert(testModel, &item) == UMI_STATUS_OK);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 0U);
        GtkListBoxRow *row = FindProblem(list, "invalid-display"); CHECK(row != NULL);
        CHECK(ProblemTextIsValid(GTK_WIDGET(row)));
        GObject *token = g_object_get_data(G_OBJECT(row), "studio-problem-page");
        const UmiDiagnosticSnapshot *originalItem = UmiDiagnosticPageItem(g_object_get_data(token, "page"), 0U);
        CHECK(originalItem != NULL && (unsigned char)originalItem->message[0] == 0xfeU);
    } else if (strcmp(testMode, "first") == 0) {
        CHECK(FindProblem(list, "problem.0299") != NULL && FindProblem(list, "problem.0300") == NULL);
    } else if (strcmp(testMode, "unchanged") == 0) {
        RefreshProblems(); CHECK(FindProblem(list, "problem.0000") == oldRow);
    } else if (strcmp(testMode, "retained-controls") == 0) {
        size_t before = umi_diagnostic_model_count(testModel);
        CloseProblemsHost();
        g_signal_emit_by_name(next, "clicked"); g_signal_emit_by_name(previous, "clicked");
        CHECK(umi_diagnostic_model_count(testModel) == before);
    } else if (strcmp(testMode, "stale-next") == 0) {
        CHECK(umi_diagnostic_model_remove(testModel, "problem.0000") == UMI_STATUS_OK);
        g_signal_emit_by_name(next, "clicked");
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 4") != NULL);
        CHECK(FindProblem(list, "problem.0001") != NULL);
        g_signal_emit_by_name(next, "clicked");
        CHECK(FindProblem(list, "problem.0301") != NULL);
    } else if (strstr(testMode, "range-") != NULL || strstr(testMode, "-close") != NULL) {
        if (strcmp(testMode, "next-close") == 0) {
            g_signal_emit_by_name(next, "clicked"); g_signal_emit_by_name(next, "clicked");
        }
        observation = g_object_ref(G_OBJECT(strcmp(testMode, "previous-close") == 0 ? previous :
            strcmp(testMode, "next-close") == 0 ? next : range));
        handler = g_signal_connect(observation, strstr(testMode, "range-") != NULL ?
            "notify::label" : "notify::sensitive", G_CALLBACK(PagePropertyChanged), NULL);
        armed = 1; g_signal_emit_by_name(next, "clicked"); armed = 0;
        CHECK(mutations == 1);
        if (strstr(testMode, "-close") != NULL) CHECK(workbench == NULL);
        else if (strcmp(testMode, "range-reenter") == 0) {
            CHECK(chrome->problemsRendered && FindProblem(list, "problem.0300") != NULL);
        } else {
            CHECK(workbench != NULL && !chrome->problemsRendered);
            CHECK(g_object_get_data(G_OBJECT(list), "studio-problems-page") == NULL);
            RefreshProblems(); CHECK(chrome->problemsRendered);
            if (strcmp(testMode, "range-query") == 0) {
                CHECK(ProblemCount(list) == 7U && FindProblem(list, "problem.0906") != NULL);
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 1") != NULL);
            } else CHECK(FindProblem(list, "problem.0000") != NULL);
        }
    } else {
        g_signal_emit_by_name(next, "clicked");
        CHECK(ProblemCount(list) == 300U && FindProblem(list, "problem.0300") != NULL);
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 2 of 4") != NULL);
        if (strcmp(testMode, "next") == 0) CHECK(FindProblem(list, "problem.0599") != NULL);
        else if (strcmp(testMode, "previous") == 0) {
            g_signal_emit_by_name(previous, "clicked"); CHECK(FindProblem(list, "problem.0000") != NULL);
        } else if (strcmp(testMode, "filter") == 0) {
            gtk_editable_set_text(GTK_EDITABLE(testSearch), "tail-only");
            CHECK(ProblemCount(list) == 7U && FindProblem(list, "problem.0906") != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 1") != NULL);
        } else if (strcmp(testMode, "sort") == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 0U);
            CHECK(gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 0) == FindProblem(list, "problem.0906"));
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 4") != NULL);
        } else if (strcmp(testMode, "severity") == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(severity), 2U);
            CHECK(ProblemCount(list) == 300U && FindProblem(list, "problem.0000") != NULL);
            CHECK(FindProblem(list, "problem.0001") == NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 1 of 2") != NULL);
        } else if (strcmp(testMode, "empty") == 0) {
            gtk_editable_set_text(GTK_EDITABLE(testSearch), "absent message");
            CHECK(ProblemCount(list) == 0U && !gtk_widget_get_sensitive(previous) && !gtk_widget_get_sensitive(next));
            CHECK(strcmp(gtk_label_get_text(GTK_LABEL(range)), "No matching diagnostics") == 0);
        } else if (strcmp(testMode, "clear") == 0) {
            g_signal_emit_by_name(clear, "clicked");
            CHECK(ProblemCount(list) == 0U && umi_diagnostic_model_count(testModel) == 0U);
            CHECK(!gtk_widget_get_sensitive(previous) && !gtk_widget_get_sensitive(next));
        } else if (strcmp(testMode, "retained-row") == 0) {
            size_t before = umi_document_coordinator_count(documents);
            g_signal_emit_by_name(list, "row-activated", oldRow);
            CHECK(umi_document_coordinator_count(documents) == before);
            g_signal_emit_by_name(previous, "clicked");
            g_signal_emit_by_name(list, "row-activated", oldRow);
            CHECK(umi_document_coordinator_count(documents) == before);
        } else {
            g_signal_emit_by_name(next, "clicked"); g_signal_emit_by_name(next, "clicked");
            CHECK(ProblemCount(list) == 7U && FindProblem(list, "problem.0906") != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 4 of 4") != NULL && !gtk_widget_get_sensitive(next));
            if (strcmp(testMode, "shrink") == 0) {
                for (unsigned i = 301U; i < 907U; ++i) {
                    char id[64]; snprintf(id, sizeof id, "problem.%04u", i);
                    CHECK(umi_diagnostic_model_remove(testModel, id) == UMI_STATUS_OK);
                }
                RefreshProblems();
                CHECK(ProblemCount(list) == 1U && FindProblem(list, "problem.0300") != NULL);
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(range)), "Page 2 of 2") != NULL);
            } else if (strcmp(testMode, "resolved") == 0) {
                for (unsigned i = 0U; i < 900U; ++i) {
                    char id[64]; snprintf(id, sizeof id, "problem.%04u", i);
                    CHECK(umi_diagnostic_model_resolve(testModel, id, 1) == UMI_STATUS_OK);
                }
                RefreshProblems(); CHECK(ProblemCount(list) == 7U && !gtk_widget_get_sensitive(previous));
                gtk_check_button_set_active(GTK_CHECK_BUTTON(resolved), TRUE);
                CHECK(ProblemCount(list) == 300U && FindProblem(list, "problem.0000") != NULL);
            } else if (strcmp(testMode, "source") == 0) {
                g_signal_emit_by_name(list, "row-activated", FindProblem(list, "problem.0906"));
                UmiDocumentWorkingCopySnapshot active;
                CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
                CHECK(umi_path_equal(active.path, source));
                GtkWidget *editor = ProblemEditor(window, active.view_id); CHECK(GTK_IS_TEXT_VIEW(editor));
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
                GtkTextIter caret; gtk_text_buffer_get_iter_at_mark(buffer, &caret, gtk_text_buffer_get_insert(buffer));
                CHECK(gtk_text_iter_get_line(&caret) == 1 && gtk_text_iter_get_line_index(&caret) == 2);
            }
        }
    }
cleanup:
    armed = 0;
    if (handler != 0U) g_signal_handler_disconnect(observation, handler);
    testHost = NULL;
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&observation); g_clear_object(&oldRow);
    g_clear_object(&list); g_clear_object(&range); g_clear_object(&previous); g_clear_object(&next);
    g_clear_object(&testSearch); g_clear_object(&sort); g_clear_object(&severity); g_clear_object(&resolved); g_clear_object(&clear);
    if (changedDirectory) (void)g_chdir(original);
    if (isolated) {
        if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    g_free(root); g_free(source); g_free(original); g_free(oldData);
    return failed;
}
