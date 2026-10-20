/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_first_use_gtk4.c
 * PURPOSE: Exercise visible New/Open, native typing and Save from a closed workspace.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/commands.h"
#include "umicom/developer_project/new_project.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

/* Match the real coordinator-owned page; a welcome or Output text view is not
 * sufficient evidence that the source editor is reachable. */
static GtkWidget *FindEditor(GtkWidget *root, const char *viewId)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (id != NULL && strcmp(id, viewId) == 0) {
        GtkWidget *editor = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
        if (GTK_IS_TEXT_VIEW(editor)) return editor;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindEditor(child, viewId);
        if (found != NULL) return found;
    }
    return NULL;
}
static GtkWidget *FindAction(GtkWidget *root, const char *id)
{
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && actual != NULL && strcmp(actual, id) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindAction(child, id);
        if (found != NULL) return found;
    }
    return NULL;
}
static GtkWidget *FindSourceTab(GtkWidget *root, const char *viewId)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, viewId) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindSourceTab(child, viewId);
        if (found != NULL) return found;
    }
    return NULL;
}
/* Inspect user-visible location text rather than private service state. */
static int ContainsLabel(GtkWidget *root, const char *text)
{
    if (GTK_IS_LABEL(root) && strcmp(gtk_label_get_text(GTK_LABEL(root)), text) == 0) return 1;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
        if (ContainsLabel(child, text)) return 1;
    return 0;
}

static int Reachable(GtkWidget *editor, GtkWidget *window)
{
    if (editor == NULL || !gtk_text_view_get_editable(GTK_TEXT_VIEW(editor))) return 0;
    for (GtkWidget *node = editor; node != window; node = gtk_widget_get_parent(node))
        if (node == NULL || !gtk_widget_get_visible(node) || !gtk_widget_get_child_visible(node)) return 0;
    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(window));
    return focus == editor || (focus != NULL && gtk_widget_is_ancestor(focus, editor));
}
/* A host may close from a focus observer. The command must return without
 * using that released workbench to present its completion message. */
static void CloseWhenEditorFocused(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)spec;
    UmiStudioGtkWorkbench **slot = data;
    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(object));
    if (*slot != NULL && GTK_IS_TEXT_VIEW(focus)) {
        UmiStudioGtkWorkbench *closing = *slot;
        *slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
    }
}
/* Real toolchain acceptance: this pumps the ordinary workbench refresh path,
 * which collects worker results and publishes Problems/Output. A successful
 * submission is not treated as a successful configure, compile or run. */
static int WaitForWorkbenchBuild(UmiStudioGtkWorkbench *workbench,
    UmiStudioBuildService *build, UmiBuildResult *latest)
{
    gint64 deadline = g_get_monotonic_time() + 120 * G_TIME_SPAN_SECOND;
    while (g_get_monotonic_time() < deadline) {
        if (umi_studio_gtk_workbench_workspace_synchronise(workbench) != UMI_STATUS_OK) return 0;
        for (unsigned event = 0U; event < 32U && g_main_context_pending(NULL); ++event)
            (void)g_main_context_iteration(NULL, FALSE);
        if (!UmiStudioBuildBusy(build)) {
            UmiStatus read = umi_build_history_latest(umi_studio_build_service_history(build), latest);
            if (read == UMI_STATUS_OK)
                fprintf(stdout, "%s: %s (exit %d)\n%s\n", umi_build_phase_text(latest->phase),
                    umi_status_text(latest->status), latest->exit_code, latest->output);
            return read == UMI_STATUS_OK;
        }
        g_usleep(10000U);
    }
    fputs("Studio build did not complete within the qualification deadline.\n", stderr);
    umi_studio_build_service_cancel(build);
    return 0;
}
/* Keep one reference while exercising the real transient Project Settings
 * form. The test does not bypass the application's apply/trust callback. */
static GtkWindow *FindSettingsWindow(GtkWindow *parent)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *candidate = g_list_model_get_item(windows, index);
        if (gtk_window_get_transient_for(candidate) == parent &&
            umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate), "developer.build.trusted") != NULL)
            return candidate;
        g_object_unref(candidate);
    }
    return NULL;
}

static int ExerciseNativeBuild(UmiStudioGtkWorkbench *workbench,
    UmiStudioServices *services, UmiDocumentCoordinator *documents,
    const char *fixture, int repairFailure)
{
    int failed = 0;
    UmiDeveloperProjectService *projects = NULL;
    UmiBuildResult *result = NULL;
    GtkWindow *settingsWindow = NULL;
    char *root = g_build_filename(fixture, "Source project", NULL);
    char *source = g_build_filename(root, "src", "main.c", NULL);
    UmiDeveloperProjectGenerationRequest request;
    UmiDeveloperProjectGeneratorReport report;
    UmiDeveloperProjectModel model;
    UmiBuildProfile profile;
    UmiStudioBuildService *build = umi_studio_services_build(services);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(umi_developer_project_service_create(&projects) == UMI_STATUS_OK);
    umi_developer_project_generation_request_init(&request);
    g_strlcpy(request.template_id, "developer.template.c23-console", sizeof request.template_id);
    g_strlcpy(request.application_name, "Studio workflow", sizeof request.application_name);
    g_strlcpy(request.application_id, "org.umicom.studio.workflow", sizeof request.application_id);
    g_strlcpy(request.repository_name, "studio-workflow", sizeof request.repository_name);
    g_strlcpy(request.target_name, "studio_workflow", sizeof request.target_name);
    CHECK(g_strlcpy(request.project_root, root, sizeof request.project_root) < sizeof request.project_root);
    CHECK(UmiDeveloperProjectCreateNew(projects, &request, &report, &model, &profile) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
/* Direct trust setup is retained for comparison; the acceptance path now exercises Project Settings. */
#if 0
    /* This fixture owns the generated code. Grant execution deliberately,
     * separately from opening its folder, just as Project Settings requires. */
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
#endif
    /* This fixture owns the generated code. Exercise the same visible settings
     * decision as a user, separately from creating or opening the project. */
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.settings", NULL) == UMI_STATUS_OK);
    settingsWindow = FindSettingsWindow(GTK_WINDOW(window));
    CHECK(settingsWindow != NULL);
    GtkWidget *trust = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(settingsWindow), "developer.build.trusted");
    GtkWidget *apply = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(settingsWindow), "developer.dialog.accept");
    CHECK(GTK_IS_CHECK_BUTTON(trust) && GTK_IS_BUTTON(apply));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(trust), TRUE);
    g_signal_emit_by_name(apply, "clicked");
    UmiStudioWorkspaceSnapshot reviewedWorkspace;
    CHECK(umi_studio_workspace_snapshot(services, &reviewedWorkspace) == UMI_STATUS_OK && reviewedWorkspace.graph.trusted);
    g_clear_object(&settingsWindow);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_BUILD, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    CHECK(umi_build_result_create(&result) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(GTK_IS_TEXT_VIEW(editor));
    const char *validSource = "#include <stdio.h>\nint main(void) { puts(\"Studio native workflow complete\"); return 0; }\n";
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_begin_user_action(buffer);
    gtk_text_buffer_set_text(buffer, repairFailure
        ? "#error deliberate_studio_compile_failure\nint main(void) { return 0; }\n" : validSource, -1);
    gtk_text_buffer_end_user_action(buffer);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.configure", NULL) == UMI_STATUS_OK);
    CHECK(WaitForWorkbenchBuild(workbench, build, result));
    CHECK(result->phase == UMI_BUILD_PHASE_CONFIGURE && result->status == UMI_STATUS_OK && result->exit_code == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && !active.dirty);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.build", NULL) == UMI_STATUS_OK);
    CHECK(WaitForWorkbenchBuild(workbench, build, result));
    if (repairFailure) {
        CHECK(result->status != UMI_STATUS_OK && result->exit_code != 0);
        CHECK(strstr(result->output, "deliberate_studio_compile_failure") != NULL);
        /* The failed compiler must leave a navigable source location even when
         * the user has hidden the editor before opening the diagnostic. */
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "build-error-review", "Build error review") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "navigate.problem.next", NULL) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, source));
        editor = FindEditor(window, active.view_id);
        CHECK(Reachable(editor, window));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
        gtk_text_buffer_begin_user_action(buffer);
        gtk_text_buffer_set_text(buffer, validSource, -1);
        gtk_text_buffer_end_user_action(buffer);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.build", NULL) == UMI_STATUS_OK);
        CHECK(WaitForWorkbenchBuild(workbench, build, result));
    }
    CHECK(result->status == UMI_STATUS_OK && result->exit_code == 0);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.start", NULL) == UMI_STATUS_OK);
    CHECK(WaitForWorkbenchBuild(workbench, build, result));
    CHECK(result->phase == UMI_BUILD_PHASE_RUN && result->status == UMI_STATUS_OK && result->exit_code == 0);
    CHECK(strstr(result->output, "Studio native workflow complete") != NULL);
cleanup:
    if (failed && build != NULL) umi_studio_build_service_cancel(build);
    g_clear_object(&settingsWindow);
    umi_build_result_destroy(result);
    if (projects != NULL) umi_developer_project_service_destroy(projects);
    g_free(source);
    g_free(root);
    return failed;
}

/* Completion feedback is an observable GTK property; a host can be closed
 * by that observer while the command is still unwinding. */
static void CloseWhenFeedbackDisplayed(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)spec;
    UmiStudioGtkWorkbench **workbench = data;
    if (*workbench == NULL || gtk_label_get_text(GTK_LABEL(object))[0] == '\0') return;
    UmiStudioGtkWorkbench *closing = *workbench;
    *workbench = NULL;
    umi_studio_gtk_workbench_destroy(closing);
}

/* Exercise the actual Explorer button and modal form, including a layout with
 * no editor pane. Creation must retain the file even if a focus observer closes
 * the host before the operation can present its completion. */
static int ExerciseProjectEntry(UmiStudioGtkWorkbench **slot, UmiStudioServices *services,
    UmiDocumentCoordinator *documents, const char *root, const char *name)
{
    int failed = 0;
    GtkWindow *prompt = NULL;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    gulong closeHandler = 0U;
    char *source = g_build_filename(root, "created notes.c", NULL);
    char *contents = NULL;
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "project-files", "Project files") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(*slot, UMI_STUDIO_SURFACE_EXPLORER, NULL) == UMI_STATUS_OK);
    GtkWidget *create = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-create-file");
    CHECK(GTK_IS_BUTTON(create));
    if (strcmp(name, "project-create-existing") == 0)
        CHECK(g_file_set_contents(source, "keep existing source\n", -1, NULL));
    g_signal_emit_by_name(create, "clicked");
    GListModel *windows = gtk_window_get_toplevels();
    for (guint index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *candidate = g_list_model_get_item(windows, index);
        if (gtk_window_get_transient_for(candidate) == window &&
            umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate), "studio.prompt.value") != NULL) {
            prompt = candidate;
            break;
        }
        g_object_unref(candidate);
    }
    CHECK(prompt != NULL);
    GtkWidget *value = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(prompt), "studio.prompt.value");
    GtkWidget *accept = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(prompt), "studio.prompt.accept");
    CHECK(GTK_IS_EDITABLE(value) && GTK_IS_BUTTON(accept));
    gtk_editable_set_text(GTK_EDITABLE(value), "created notes.c");
    if (strcmp(name, "close-during-create") == 0)
        closeHandler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
    g_signal_emit_by_name(accept, "clicked");
    CHECK(g_file_get_contents(source, &contents, NULL, NULL));
    if (strcmp(name, "project-create-existing") == 0) {
        CHECK(strcmp(contents, "keep existing source\n") == 0);
        CHECK(umi_document_coordinator_count(documents) == 0U);
    } else {
        CHECK(contents[0] == '\0' && umi_document_coordinator_count(documents) == 1U);
        if (strcmp(name, "close-during-create") == 0) CHECK(*slot == NULL);
        else {
            UmiDocumentWorkingCopySnapshot active;
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
            CHECK(active.has_path);
            CHECK(Reachable(FindEditor(GTK_WIDGET(window), active.view_id), GTK_WIDGET(window)));
        }
    }
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(window, closeHandler);
    if (prompt != NULL) { gtk_window_destroy(prompt); g_object_unref(prompt); }
    g_object_unref(window);
    g_free(source);
    g_free(contents);
    return failed;
}

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *name = argv[1];
    const char *cases[] = {"new-file", "hidden-new", "open-edit-save", "hidden-open",
        "unicode-path", "missing-file", "reopen-dirty", "project-folder", "tab-switch", "build-guidance", "close-during-open", "build-unsaved", "real-build-run", "real-build-repair", "repository-switch", "close-during-feedback", "debug-unsaved", "debug-canonical-unsaved", "project-create-hidden", "project-create-existing", "close-during-create", "workspace-refresh-edit"};
    int known = 0;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(name, cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, changedDirectory = 0;
    char *original = g_get_current_dir(), *directory = NULL, *source = NULL, *disk = NULL;
    char *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GError *error = NULL;
    char *draft = NULL;
    size_t draftLength = 0U;
    /* Keep fixtures under the explicitly supplied build qualification folder. */
    directory = g_build_filename(argv[2], "umicom-first-use-XXXXXX", NULL);
    CHECK(g_mkdtemp(directory) != NULL);
    CHECK(directory != NULL && g_chdir(directory) == 0);
    changedDirectory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions servicesOptions = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&servicesOptions, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioWorkspaceSnapshot startupWorkspace;
    CHECK(umi_studio_workspace_snapshot(services, &startupWorkspace) == UMI_STATUS_OK);
    CHECK(!startupWorkspace.graph.open);
    CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.first-use-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(umi_document_coordinator_count(documents) == 0U);
    if (strcmp(name, "real-build-run") == 0 || strcmp(name, "real-build-repair") == 0) {
        failed = ExerciseNativeBuild(workbench, services, documents, directory,
            strcmp(name, "real-build-repair") == 0);
        goto cleanup;
    }
    if (strcmp(name, "project-create-hidden") == 0 || strcmp(name, "project-create-existing") == 0 ||
        strcmp(name, "close-during-create") == 0) {
        failed = ExerciseProjectEntry(&workbench, services, documents, directory, name);
        goto cleanup;
    }
    if (strcmp(name, "close-during-feedback") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_SOURCE_CONTROL, NULL) == UMI_STATUS_OK);
        GtkWidget *refresh = umi_gtk4_automation_find_tagged_widget(window, "studio.source-control.refresh");
        GtkWidget *notice = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
        CHECK(GTK_IS_BUTTON(refresh) && GTK_IS_LABEL(notice));
        g_object_ref(refresh);
        g_object_ref(notice);
        gulong handler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), &workbench);
        /* No project is open, so this reports unavailable without invoking Git. */
        g_signal_emit_by_name(refresh, "clicked");
        g_signal_handler_disconnect(notice, handler);
        g_object_unref(notice);
        g_object_unref(refresh);
        CHECK(workbench == NULL);
        goto cleanup;
    }
    if (strcmp(name, "repository-switch") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_SOURCE_CONTROL, NULL) == UMI_STATUS_OK);
        GtkWidget *review = umi_gtk4_automation_find_tagged_widget(window, "studio.repository.review");
        CHECK(GTK_IS_EXPANDER(review) && ContainsLabel(review, "Open a project to inspect its repository."));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(ContainsLabel(review, directory));
        source = g_build_filename(directory, "Second project", NULL);
        CHECK(g_mkdir_with_parents(source, 0700) == 0);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", source) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(ContainsLabel(review, source) && !ContainsLabel(review, directory));
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(ContainsLabel(review, "Open a project to inspect its repository.") && !ContainsLabel(review, source));
        goto cleanup;
    }
    if (strcmp(name, "close-during-open") == 0) {
        g_object_ref(window);
        gulong handler = g_signal_connect(window, "notify::focus-widget",
            G_CALLBACK(CloseWhenEditorFocused), &workbench);
        UmiStatus opened = umi_studio_gtk_workbench_execute_command(workbench, "file.new", NULL);
        g_signal_handler_disconnect(window, handler);
        g_object_unref(window);
        CHECK(workbench == NULL);
        CHECK(opened == UMI_STATUS_OK || opened == UMI_STATUS_CANCELLED || opened == UMI_STATUS_UNAVAILABLE);
        CHECK(umi_document_coordinator_count(documents) == 1U);
        goto cleanup;
    }
    if (strncmp(name, "hidden-", 7U) == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "first-use-empty", "Empty layout") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    }
    source = g_build_filename(directory, strcmp(name, "unicode-path") == 0
        ? "source \xe9\x9b\xaa.c" : "source notes.c", NULL);
    const char *initial = "int main(void) { return 0; }\n";
    const char *addition = "/* typed in the actual Studio editor */\n";
    int newFile = strcmp(name, "new-file") == 0 || strcmp(name, "hidden-new") == 0 ||
        strcmp(name, "build-unsaved") == 0 || strcmp(name, "debug-unsaved") == 0 ||
        strcmp(name, "debug-canonical-unsaved") == 0;
    if (strcmp(name, "hidden-new") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.new", NULL) == UMI_STATUS_OK);
    } else if (newFile) {
        GtkWidget *button = FindAction(window, "file.new");
        CHECK(button != NULL && gtk_widget_is_sensitive(button));
        g_signal_emit_by_name(button, "clicked");
    } else {
        CHECK(g_file_set_contents(source, initial, -1, &error));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    }
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_begin_user_action(buffer);
    gtk_text_buffer_insert(buffer, &end, addition, -1);
    gtk_text_buffer_end_user_action(buffer);
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &draft, &draftLength) == UMI_STATUS_OK);
    CHECK(strstr(draft, addition) != NULL);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    if (strcmp(name, "workspace-refresh-edit") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
        UmiFileIndex *index = umi_studio_services_file_index(services);
        CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
        UmiFileIndexRefreshSnapshot before, after;
        CHECK(UmiFileIndexRefreshRead(index, &before) == UMI_STATUS_OK);
        CHECK(g_file_set_contents("added while editing.c", "/* new indexed file */\n", -1, NULL));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_WORKSPACE_REFRESH, NULL) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshRead(index, &after) == UMI_STATUS_OK && after.requestId > before.requestId);
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, "/* editing during refresh */\n", -1);
        CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshRead(index, &after) == UMI_STATUS_OK && after.status == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(FindEditor(window, active.view_id) == editor && gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer);
        char *edited = NULL;
        size_t editedSize = 0U;
        CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &edited, &editedSize) == UMI_STATUS_OK);
        int kept = strstr(edited, addition) != NULL && strstr(edited, "editing during refresh") != NULL;
        UmiUiDocumentViewModelFreeText(edited);
        CHECK(kept);
        goto cleanup;
    }
    if (strcmp(name, "missing-file") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", "missing-source.c") != UMI_STATUS_OK);
        CHECK(umi_document_coordinator_count(documents) == 1U);
        CHECK(FindEditor(window, active.view_id) == editor);
    }
    if (strcmp(name, "reopen-dirty") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_count(documents) == 1U);
        CHECK(FindEditor(window, active.view_id) == editor);
        CHECK(Reachable(editor, window));
    }
    if (strcmp(name, "tab-switch") == 0) {
        char firstId[UMI_UI_ID_CAPACITY];
        g_strlcpy(firstId, active.view_id, sizeof firstId);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.new", NULL) == UMI_STATUS_OK);
        GtkWidget *tab = FindSourceTab(window, firstId);
        CHECK(tab != NULL);
        g_signal_emit_by_name(tab, "clicked");
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(strcmp(active.view_id, firstId) == 0 && active.dirty);
        CHECK(Reachable(editor, window));
    }
    /* Repeated normal status synchronization must not replace the native buffer
     * or erase the draft which the user has just typed. */
    for (unsigned tick = 0; tick < 5U; ++tick)
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    CHECK(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer);
    char *current = NULL;
    size_t currentLength = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &current, &currentLength) == UMI_STATUS_OK);
    int same = currentLength == draftLength && memcmp(current, draft, draftLength) == 0;
    UmiUiDocumentViewModelFreeText(current);
    CHECK(same);
    if (strcmp(name, "build-unsaved") == 0 || strcmp(name, "debug-unsaved") == 0 ||
        strcmp(name, "debug-canonical-unsaved") == 0) {
        CHECK(g_file_set_contents("CMakeLists.txt", "cmake_minimum_required(VERSION 3.25)\nproject(Notes C)\n", -1, &error));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
/* The same unsaved-document scenario now covers Build and both Debug identifiers; retain the earlier build-only assertion for review. */
#if 0
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.build", NULL) != UMI_STATUS_OK);
#endif
        const char *toolCommand = strcmp(name, "build-unsaved") == 0 ? "build.build" :
            strcmp(name, "debug-unsaved") == 0 ? "debug.start" : UMI_STUDIO_COMMAND_DEBUG_START;
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, toolCommand, NULL) != UMI_STATUS_OK);
        CHECK(!UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)));
        CHECK(!UmiStudioBuildBusy(umi_studio_services_build(services)));
        GtkWidget *notice = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
        CHECK(GTK_IS_LABEL(notice));
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(notice)), "Save As") != NULL);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(active.dirty && !active.has_path);
        goto cleanup;
    }
    if (newFile)
        CHECK(UmiDocumentCoordinatorSaveAs(documents, active.document_id, source) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.save", NULL) == UMI_STATUS_OK);
    CHECK(g_file_get_contents(source, &disk, NULL, &error) && strcmp(disk, draft) == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && !active.dirty);
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.open);
    if (strcmp(name, "build-guidance") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_BUILD, NULL) == UMI_STATUS_OK);
        GtkWidget *context = umi_gtk4_automation_find_tagged_widget(window, "studio.build.context");
        CHECK(GTK_IS_LABEL(context) && strstr(gtk_label_get_text(GTK_LABEL(context)), "Open Project Folder") != NULL);
    }
    if (strcmp(name, "project-folder") == 0 || strcmp(name, "build-guidance") == 0) {
        CHECK(g_file_set_contents("CMakeLists.txt", "cmake_minimum_required(VERSION 3.25)\nproject(Notes C)\n", -1, &error));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && workspace.graph.open);
        CHECK(umi_path_equal(workspace.graph.root, directory) && !workspace.graph.trusted);
        /* Opening a project never grants execution permission implicitly. */
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "build.configure", NULL) == UMI_STATUS_PERMISSION_DENIED);
        CHECK(!UmiStudioBuildBusy(umi_studio_services_build(services)));
        if (strcmp(name, "build-guidance") == 0) {
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            GtkWidget *context = umi_gtk4_automation_find_tagged_widget(window, "studio.build.context");
            CHECK(GTK_IS_LABEL(context) && strstr(gtk_label_get_text(GTK_LABEL(context)), "Project Settings") != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(context)), "disabled") != NULL);
            /* Keep the same panel alive while settings change. Its displayed
             * paths must follow the new profile without closing/reopening it. */
            UmiStudioBuildService *build = umi_studio_services_build(services);
            const UmiBuildProfile *currentProfile = umi_studio_build_service_profile(build);
            CHECK(currentProfile != NULL);
            UmiBuildProfile changedProfile = *currentProfile;
            g_strlcpy(changedProfile.profile_id, "reviewed-profile", sizeof changedProfile.profile_id);
            g_strlcpy(changedProfile.build_directory, "build/selected-by-user", sizeof changedProfile.build_directory);
            CHECK(umi_studio_build_service_set_profile(build, &changedProfile) == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(context)), "reviewed-profile") != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(context)), "build/selected-by-user") != NULL);
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(context)), "Open Project Folder") != NULL);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(context)), "reviewed-profile") == NULL);
        }
    }
cleanup:
    UmiUiDocumentViewModelFreeText(draft);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    g_clear_error(&error);
    if (changedDirectory) (void)g_chdir(original);
    if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    /* Preserve failed fixtures for diagnosis; only remove our created tree. */
    if (!failed && directory != NULL) (void)umi_fs_remove_tree(directory);
    g_free(oldData); g_free(disk); g_free(source); g_free(directory); g_free(original);
    return failed;
}
