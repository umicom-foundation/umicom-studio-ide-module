/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_problems_publication_gtk4.c
 * PURPOSE: Exercise live diagnostic filters, row ownership and source opening.
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
static UmiStudioGtkWorkbench **testHost;
static UmiDiagnosticModel *testModel;
static GtkWidget *testSearch;
static const char *testMode;
static int armed, mutations;
static void RefreshProblems(void);
static void CloseProblemsHost(void)
{
    if (testHost != NULL && *testHost != NULL) {
        UmiStudioGtkWorkbench *old = *testHost; *testHost = NULL;
        umi_studio_gtk_workbench_destroy(old);
    }
}
static void MutateProblems(void)
{
    if (!armed) return;
    armed = 0; ++mutations;
    if (strstr(testMode, "-close") != NULL) CloseProblemsHost();
    else if (strstr(testMode, "filter-") != NULL) {
        gtk_editable_set_text(GTK_EDITABLE(testSearch), "beta");
        if (strstr(testMode, "return-") != NULL) gtk_editable_set_text(GTK_EDITABLE(testSearch), "");
    } else if (strstr(testMode, "model-") != NULL)
        (void)umi_diagnostic_model_remove(testModel, "problem.beta");
    else if (strstr(testMode, "reenter-") != NULL) RefreshProblems();
}
static void ProblemsPropertyChanged(GObject *object, GParamSpec *property, gpointer data)
{ (void)object; (void)property; (void)data; MutateProblems(); }
static void ProblemsRowSelected(GtkListBox *list, GtkListBoxRow *row, gpointer data)
{ (void)list; (void)data; if (row != NULL) MutateProblems(); }
/* Only construction is intercepted to attach a real GTK parent observer.
 * The Framework publisher and every Studio model/control handler are real. */
static GtkWidget *ObservedProblemRow(void)
{
    GtkWidget *row = gtk_list_box_row_new();
    if (armed && strstr(testMode, "append") != NULL)
        g_signal_connect(row, "notify::parent", G_CALLBACK(ProblemsPropertyChanged), NULL);
    return row;
}
#define gtk_list_box_row_new ObservedProblemRow
#include "../src/gui/workbench/workbench_window.c"
#undef gtk_list_box_row_new
static void RefreshProblems(void)
{
    if (testHost != NULL && *testHost != NULL)
        runtime_problems_refresh_list((*testHost)->runtime_chrome);
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
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%d: %s\n", __LINE__, #expr); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *cases[] = {"normal", "search", "severity", "sort", "resolved", "empty", "selection", "filter-selection", "sort-selection", "unchanged", "bounded", "remove-close", "append-close", "summary-close", "selection-close", "filter-during-remove", "filter-return-during-remove", "model-during-remove", "reenter-during-remove", "filter-during-append", "model-during-append", "retained-row", "retained-controls", "clear", "clear-close", "source-dirty"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(argv[1], cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    testMode = argv[1];
    int failed = 0, changedDirectory = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "problems-XXXXXX", NULL), *source = NULL, *bufferText = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *list = NULL, *summary = NULL, *severity = NULL, *sort = NULL, *resolved = NULL, *clear = NULL, *open = NULL;
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
    application = gtk_application_new("org.umicom.studio.problems-publication-test", G_APPLICATION_NON_UNIQUE);
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
    CHECK(AddProblem("problem.alpha", UMI_DIAGNOSTIC_ERROR, "alpha error", source) == UMI_STATUS_OK);
    CHECK(AddProblem("problem.beta", UMI_DIAGNOSTIC_WARNING, "beta warning", source) == UMI_STATUS_OK);
    CHECK(AddProblem("problem.gamma", UMI_DIAGNOSTIC_FATAL, "gamma resolved", source) == UMI_STATUS_OK);
    CHECK(umi_diagnostic_model_resolve(testModel, "problem.gamma", 1) == UMI_STATUS_OK);
    RefreshProblems();
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    CHECK(GTK_IS_LIST_BOX(chrome->problems_list) && GTK_IS_LABEL(chrome->problems_summary_label));
    list = g_object_ref(chrome->problems_list); summary = g_object_ref(chrome->problems_summary_label);
    testSearch = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-search"));
    severity = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-severity"));
    sort = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-sort"));
    resolved = g_object_ref(RuntimeProblemsControl(chrome, "umicom-problems-resolved"));
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *foundClear = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.clear");
    GtkWidget *foundOpen = umi_gtk4_automation_find_tagged_widget(window, "studio.problems.open");
    CHECK(GTK_IS_BUTTON(foundClear) && GTK_IS_BUTTON(foundOpen));
    clear = g_object_ref(foundClear); open = g_object_ref(foundOpen);
    CHECK(ProblemCount(list) == 2U && FindProblem(list, "problem.alpha") != NULL);
    oldRow = g_object_ref(FindProblem(list, "problem.alpha"));
    if (strcmp(testMode, "normal") == 0) {
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(summary)), "2 matching / 2 active") != NULL);
    } else if (strcmp(testMode, "search") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(testSearch), "beta");
        CHECK(ProblemCount(list) == 1U && FindProblem(list, "problem.beta") != NULL);
    } else if (strcmp(testMode, "severity") == 0) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(severity), 2U);
        CHECK(ProblemCount(list) == 1U && FindProblem(list, "problem.alpha") != NULL);
    } else if (strcmp(testMode, "sort") == 0) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 2U);
        CHECK(gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 0) == FindProblem(list, "problem.alpha"));
    } else if (strcmp(testMode, "resolved") == 0) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(resolved), TRUE);
        CHECK(ProblemCount(list) == 3U && FindProblem(list, "problem.gamma") != NULL);
    } else if (strcmp(testMode, "empty") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(testSearch), "no such compiler message");
        CHECK(ProblemCount(list) == 0U && gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 0) != NULL);
        CHECK(chrome->problemsSelectedId[0] == '\0');
    } else if (strcmp(testMode, "filter-selection") == 0 || strcmp(testMode, "sort-selection") == 0) {
        gtk_list_box_select_row(GTK_LIST_BOX(list), oldRow);
        if (strcmp(testMode, "filter-selection") == 0) gtk_editable_set_text(GTK_EDITABLE(testSearch), "alpha");
        else gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 2U);
        CHECK(strcmp(chrome->problemsSelectedId, "problem.alpha") == 0);
        CHECK(gtk_list_box_get_selected_row(GTK_LIST_BOX(list)) == FindProblem(list, "problem.alpha"));
    } else if (strcmp(testMode, "unchanged") == 0) {
        RefreshProblems(); CHECK(FindProblem(list, "problem.alpha") == oldRow);
    } else if (strcmp(testMode, "bounded") == 0) {
        for (unsigned i = 0U; i < 310U; ++i) {
            char id[64]; snprintf(id, sizeof id, "extra.problem.%u", i);
            CHECK(AddProblem(id, UMI_DIAGNOSTIC_INFO, "additional", source) == UMI_STATUS_OK);
        }
        RefreshProblems(); CHECK(ProblemCount(list) == 300U);
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(summary)), "312 matching") != NULL);
    } else if (strcmp(testMode, "retained-controls") == 0) {
        size_t before = umi_diagnostic_model_count(testModel), documentsBefore = umi_document_coordinator_count(documents);
        CloseProblemsHost();
        gtk_editable_set_text(GTK_EDITABLE(testSearch), "late change");
        gtk_drop_down_set_selected(GTK_DROP_DOWN(severity), 3U);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(sort), 1U);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(resolved), TRUE);
        g_signal_emit_by_name(clear, "clicked"); g_signal_emit_by_name(open, "clicked");
        g_signal_emit_by_name(list, "row-activated", oldRow);
        CHECK(umi_diagnostic_model_count(testModel) == before && umi_document_coordinator_count(documents) == documentsBefore);
    } else if (strcmp(testMode, "source-dirty") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        GtkWidget *editor = ProblemEditor(window, active.view_id); CHECK(GTK_IS_TEXT_VIEW(editor));
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), "first\nunsaved source\nthird\n", -1);
        RefreshProblems();
        GtkListBoxRow *row = FindProblem(list, "problem.alpha"); CHECK(row != NULL);
        g_signal_emit_by_name(list, "row-activated", row);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
        CHECK(umi_path_equal(active.path, source));
        editor = ProblemEditor(window, active.view_id); CHECK(GTK_IS_TEXT_VIEW(editor) && gtk_widget_get_visible(editor));
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
        GtkTextIter first, last, caret; gtk_text_buffer_get_bounds(buffer, &first, &last);
        bufferText = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
        CHECK(strcmp(bufferText, "first\nunsaved source\nthird\n") == 0);
        gtk_text_buffer_get_iter_at_mark(buffer, &caret, gtk_text_buffer_get_insert(buffer));
        CHECK(gtk_text_iter_get_line(&caret) == 1 && gtk_text_iter_get_line_index(&caret) == 2);
    } else if (strcmp(testMode, "clear") == 0 || strcmp(testMode, "clear-close") == 0) {
        if (strcmp(testMode, "clear-close") == 0) {
            observation = g_object_ref(G_OBJECT(oldRow));
            handler = g_signal_connect(observation, "notify::parent", G_CALLBACK(ProblemsPropertyChanged), NULL); armed = 1;
        }
        g_signal_emit_by_name(clear, "clicked");
        if (strcmp(testMode, "clear-close") == 0) CHECK(workbench == NULL && mutations == 1);
        else CHECK(ProblemCount(list) == 0U && umi_diagnostic_model_count(testModel) == 0U);
    } else {
        gtk_list_box_select_row(GTK_LIST_BOX(list), oldRow);
        CHECK(strcmp(chrome->problemsSelectedId, "problem.alpha") == 0);
        if (strcmp(testMode, "summary-close") == 0) {
            observation = g_object_ref(G_OBJECT(summary));
            handler = g_signal_connect(observation, "notify::label", G_CALLBACK(ProblemsPropertyChanged), NULL);
        } else if (strcmp(testMode, "selection-close") == 0) {
            observation = g_object_ref(G_OBJECT(list));
            handler = g_signal_connect(observation, "row-selected", G_CALLBACK(ProblemsRowSelected), NULL);
        } else if (strstr(testMode, "remove") != NULL) {
            observation = g_object_ref(G_OBJECT(oldRow));
            handler = g_signal_connect(observation, "notify::parent", G_CALLBACK(ProblemsPropertyChanged), NULL);
        }
        CHECK(AddProblem("problem.delta", UMI_DIAGNOSTIC_INFO, "delta message", source) == UMI_STATUS_OK);
        armed = strcmp(testMode, "selection") != 0 && strcmp(testMode, "retained-row") != 0;
        RefreshProblems(); armed = 0;
        if (strstr(testMode, "-close") != NULL) CHECK(workbench == NULL && mutations == 1);
        else {
            CHECK(workbench != NULL);
            size_t before = umi_document_coordinator_count(documents);
            g_signal_emit_by_name(list, "row-activated", oldRow);
            CHECK(umi_document_coordinator_count(documents) == before);
            if (strstr(testMode, "filter-") != NULL || strstr(testMode, "model-") != NULL) {
                CHECK(mutations == 1 && !chrome->problemsRendered);
                CHECK(g_object_get_data(G_OBJECT(list), "studio-problems-page") == NULL);
                CHECK(chrome->problemsSelectedId[0] == '\0');
                RefreshProblems(); CHECK(chrome->problemsRendered);
                if (strstr(testMode, "filter-return") != NULL) CHECK(ProblemCount(list) == 3U);
                else if (strstr(testMode, "filter-") != NULL) CHECK(ProblemCount(list) == 1U && FindProblem(list, "problem.beta") != NULL);
                else CHECK(FindProblem(list, "problem.beta") == NULL);
            } else {
                CHECK(chrome->problemsRendered && strcmp(chrome->problemsSelectedId, "problem.alpha") == 0);
                CHECK(gtk_list_box_get_selected_row(GTK_LIST_BOX(list)) == FindProblem(list, "problem.alpha"));
                if (strstr(testMode, "reenter") != NULL) CHECK(mutations == 1);
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
    g_clear_object(&list); g_clear_object(&summary); g_clear_object(&testSearch);
    g_clear_object(&severity); g_clear_object(&sort); g_clear_object(&resolved); g_clear_object(&clear); g_clear_object(&open);
    if (changedDirectory) (void)g_chdir(original);
    if (isolated) {
        if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    g_free(root); g_free(source); g_free(bufferText); g_free(original); g_free(oldData);
    return failed;
}
