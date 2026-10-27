/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_first_use_gtk4.c
 *
 * PURPOSE:
 *   Exercise visible New/Open, native typing and Save from a closed workspace.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/test_discovery.h"
#include "umicom/studio/test_execution.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "umicom/developer_project/new_project.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/diagnostics/pipeline.h"
#include "umicom/platform/file_index.h"
#include "umicom/test_platform/coverage.h"
#include "umicom/test_platform/service.h"
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
    /* Collapsed expanders keep logical content outside their rendered child
     * tree. Inspect that owned content without expanding or changing focus. */
    if (GTK_IS_EXPANDER(root)) {
        GtkWidget *content = gtk_expander_get_child(GTK_EXPANDER(root));
        if (content != NULL && !gtk_widget_is_ancestor(content, root) &&
            ContainsLabel(content, text)) return 1;
    }
    return 0;
}

/* Compare repository identity using Framework path rules. Native Windows paths
 * may use backslashes while the workspace normalizes them to forward slashes.
 * Read the actual visible label; a service snapshot alone would miss a stale UI. */
static int RepositoryRootMatches(GtkWidget *review, const char *path)
{
    GtkWidget *content = GTK_IS_EXPANDER(review)
        ? gtk_expander_get_child(GTK_EXPANDER(review)) : review;
    if (content == NULL) return 0;
    GtkWidget *label = umi_gtk4_automation_find_tagged_widget(content, "vcs.review.root");
    return GTK_IS_LABEL(label) && umi_path_equal(gtk_label_get_text(GTK_LABEL(label)), path);
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
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "build-error-review", "Build error review") == UMI_STATUS_OK);
#endif
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "umicom.studio.layout.build-error-review", "Build error review") == UMI_STATUS_OK);
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
    GtkWidget *retainedAccept = NULL;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    gulong closeHandler = 0U;
    char *source = g_build_filename(root, "created notes.c", NULL);
    char *contents = NULL;
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "project-files", "Project files") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.project-files", "Project files") == UMI_STATUS_OK);
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
    if (strcmp(name, "project-prompt-retired") == 0 || strcmp(name, "project-prompt-retained-window") == 0) {
        retainedAccept = g_object_ref(accept);
        gtk_window_destroy(prompt);
        if (strcmp(name, "project-prompt-retired") == 0) g_clear_object(&prompt);
        /* A late click must not dereference a released context or create a file,
         * even if another client retains the dialog object after dismissal. */
        g_signal_emit_by_name(retainedAccept, "clicked");
        CHECK(!umi_fs_exists(source) && umi_document_coordinator_count(documents) == 0U);
        goto cleanup;
    }
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
    g_clear_object(&retainedAccept);
    g_object_unref(window);
    g_free(source);
    g_free(contents);
    return failed;
}

/* Native forms are located through stable control identifiers. The returned
 * window reference survives acceptance so the test can inspect dismissal. */
static GtkWindow *FindProjectForm(GtkWindow *parent, const char *control)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *candidate = g_list_model_get_item(windows, index);
        if (gtk_window_get_transient_for(candidate) == parent &&
            gtk_widget_get_visible(GTK_WIDGET(candidate)) &&
            umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate), control) != NULL)
            return candidate;
        g_object_unref(candidate);
    }
    return NULL;
}

/* Drive the same New Project and Project Settings forms a first-time user sees.
 * The generated source is edited through its native text buffer and saved through
 * the command route. This case grants trust only after an explicit form action;
 * it does not invoke a compiler or debugger. */
static int ExerciseProjectWizard(UmiStudioGtkWorkbench **slot, UmiStudioServices *services,
    UmiDocumentCoordinator *documents, const char *root, const char *name)
{
    int failed = 0;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkWindow *form = NULL;
    gulong closeHandler = 0U;
    char *project = g_build_filename(root, "My first project", NULL);
    char *source = g_build_filename(project, "src", "main.c", NULL);
    char *disk = NULL;
    if (strcmp(name, "wizard-hidden-editor") == 0) {
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "new-project-empty", "Empty") == UMI_STATUS_OK);
#endif
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.new-project-empty", "Empty") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "project.new", NULL) == UMI_STATUS_OK);
    form = FindProjectForm(window, "developer.project.folder");
    CHECK(form != NULL);
    GtkWidget *folder = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.project.folder");
    GtkWidget *target = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.project.target");
    GtkWidget *accept = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.dialog.accept");
    CHECK(GTK_IS_EDITABLE(folder) && GTK_IS_EDITABLE(target) && GTK_IS_BUTTON(accept));
    CHECK(gtk_editable_get_text(GTK_EDITABLE(folder))[0] == '\0');
    gtk_editable_set_text(GTK_EDITABLE(folder), project);
    gtk_editable_set_text(GTK_EDITABLE(target), "first_project");
    if (strcmp(name, "wizard-close-during-adoption") == 0)
        closeHandler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
    g_signal_emit_by_name(accept, "clicked");
    CHECK(umi_fs_is_file(source));
    if (strcmp(name, "wizard-close-during-adoption") == 0) {
        CHECK(*slot == NULL && umi_document_coordinator_count(documents) == 1U);
        goto cleanup;
    }
    CHECK(!gtk_widget_get_visible(GTK_WIDGET(form)));
    g_clear_object(&form);
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
    CHECK(workspace.graph.open && !workspace.graph.trusted && umi_path_equal(workspace.graph.root, project));
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.has_path);
    GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(Reachable(editor, GTK_WIDGET(window)));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, "\n/* edited after native project creation */\n", -1);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.save", NULL) == UMI_STATUS_OK);
    CHECK(g_file_get_contents(source, &disk, NULL, NULL));
    CHECK(strstr(disk, "edited after native project creation") != NULL);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && !active.dirty);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    const UmiBuildProfile *profile = umi_studio_build_service_profile(build);
    CHECK(profile != NULL && umi_path_equal(profile->source_directory, project));
    CHECK(profile->build_target[0] == '\0' && strstr(profile->run_program, "first_project") != NULL);
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "build.settings", NULL) == UMI_STATUS_OK);
    form = FindProjectForm(window, "developer.build.trusted");
    CHECK(form != NULL);
    GtkWidget *trust = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.build.trusted");
    GtkWidget *profileName = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.build.field.0");
    accept = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "developer.dialog.accept");
    CHECK(GTK_IS_CHECK_BUTTON(trust) && GTK_IS_EDITABLE(profileName) && GTK_IS_BUTTON(accept));
    CHECK(!gtk_check_button_get_active(GTK_CHECK_BUTTON(trust)));
    gtk_editable_set_text(GTK_EDITABLE(profileName), "first-project-review");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(trust), TRUE);
    g_signal_emit_by_name(accept, "clicked");
    CHECK(!gtk_widget_get_visible(GTK_WIDGET(form)));
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && workspace.graph.trusted);
    profile = umi_studio_build_service_profile(build);
    CHECK(strcmp(profile->profile_id, "first-project-review") == 0);
    UmiBuildProfile reviewed = *profile;
    CHECK(!UmiStudioBuildBusy(build) && !UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)));
    /* Reopening loads settings but requires a fresh trust decision. The native
     * editor remains attached to the same document, with its saved content. */
    CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", project) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.trusted);
    CHECK(umi_build_profile_equal(&reviewed, umi_studio_build_service_profile(build)));
    CHECK(FindEditor(GTK_WIDGET(window), active.view_id) == editor);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(window, closeHandler);
    if (form != NULL) { gtk_window_destroy(form); g_object_unref(form); }
    g_object_unref(window);
    g_free(project);
    g_free(source);
    g_free(disk);
    return failed;
}

/* Observe the actual first presentation, before the command has returned.
 * A fully assembled form must already contain its optional configuration
 * controls; closing or reopening from that notification tests host ownership. */
typedef struct SettingsPresentationProbe {
    UmiStudioGtkWorkbench **slot;
    GtkWindow *parent;
    GPtrArray *windows;
    const char *mode;
    unsigned observed;
    int complete;
    UmiStatus replacement;
} SettingsPresentationProbe;

static void SettingsPresented(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    SettingsPresentationProbe *probe = data;
    if (probe->observed != 0U || !gtk_widget_get_visible(GTK_WIDGET(object)) ||
        gtk_window_get_transient_for(GTK_WINDOW(object)) != probe->parent) return;
    if (umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(object), "developer.build.trusted") == NULL) return;
    ++probe->observed;
    probe->complete = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(object), "developer.configurations.refresh") != NULL &&
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(object), "developer.configurations.rename") != NULL;
    if (strcmp(probe->mode, "settings-present-close") == 0) {
        UmiStudioGtkWorkbench *closing = *probe->slot;
        *probe->slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
    } else if (strcmp(probe->mode, "settings-present-replace") == 0) {
        probe->replacement = umi_studio_gtk_workbench_execute_command(*probe->slot, "build.settings", NULL);
    }
}

static void SettingsWindowAdded(GListModel *model, guint position, guint removed, guint added, gpointer data)
{
    (void)removed;
    SettingsPresentationProbe *probe = data;
    for (guint index = 0U; index < added; ++index) {
        GtkWindow *window = g_list_model_get_item(model, position + index);
        if (window == NULL) continue;
        g_signal_connect(window, "notify::visible", G_CALLBACK(SettingsPresented), probe);
        g_ptr_array_add(probe->windows, window);
    }
}

static int ExerciseSettingsPresentation(UmiStudioGtkWorkbench **slot,
    UmiStudioServices *services, UmiDocumentCoordinator *documents,
    const char *root, const char *mode)
{
    int failed = 0;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkWindow *form = NULL;
    SettingsPresentationProbe probe = {slot, window, g_ptr_array_new_with_free_func(g_object_unref),
        mode, 0U, 0, UMI_STATUS_INVALID_STATE};
    GListModel *windows = gtk_window_get_toplevels();
    gulong handler = 0U;
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.new", NULL) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_count(documents) == 1U);
    handler = g_signal_connect(windows, "items-changed", G_CALLBACK(SettingsWindowAdded), &probe);
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot, "build.settings", NULL);
    CHECK(probe.observed == 1U && probe.complete);
    CHECK(umi_document_coordinator_count(documents) == 1U);
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.trusted);
    if (strcmp(mode, "settings-present-close") == 0) {
        CHECK(*slot == NULL && status == UMI_STATUS_CANCELLED);
    } else if (strcmp(mode, "settings-present-replace") == 0) {
        CHECK(*slot != NULL && status == UMI_STATUS_CANCELLED && probe.replacement == UMI_STATUS_OK);
        form = FindProjectForm(window, "developer.configurations.rename");
        CHECK(form != NULL);
    } else {
        CHECK(status == UMI_STATUS_OK);
        form = FindProjectForm(window, "developer.configurations.rename");
        CHECK(form != NULL);
    }
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(windows, handler);
    for (guint index = 0U; index < probe.windows->len; ++index)
        g_signal_handlers_disconnect_by_data(g_ptr_array_index(probe.windows, index), &probe);
    g_ptr_array_unref(probe.windows);
    g_clear_object(&form);
    g_object_unref(window);
    return failed;
}

/* Project creation retains its original host through native construction,
 * replacement and presentation. No source is created by these lifecycle cases. */
typedef struct ProjectPresentationProbe {
    UmiStudioGtkWorkbench **slot;
    GtkWindow *parent;
    GPtrArray *windows;
    const char *mode, *command;
    unsigned observed;
    int complete;
    UmiStatus nested;
    GtkWindow *replaced;
} ProjectPresentationProbe;
static void ProjectHostClose(ProjectPresentationProbe *probe)
{
    if (*probe->slot == NULL) return;
    UmiStudioGtkWorkbench *closing = *probe->slot;
    *probe->slot = NULL;
    umi_studio_gtk_workbench_destroy(closing);
}
static void ProjectPresented(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    ProjectPresentationProbe *probe = data;
    if (probe->observed != 0U || !gtk_widget_get_visible(GTK_WIDGET(object)) ||
        gtk_window_get_transient_for(GTK_WINDOW(object)) != probe->parent) return;
    ++probe->observed;
    probe->complete = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(object),
        strcmp(probe->command, "project.new") == 0 ? "developer.project.choose-parent" : "developer.application.preview") != NULL;
    if (strstr(probe->mode, "present-close") != NULL) ProjectHostClose(probe);
    else if (strstr(probe->mode, "present-reenter") != NULL)
        probe->nested = umi_studio_gtk_workbench_execute_command(*probe->slot, probe->command, NULL);
}
static void ProjectWindowAdded(GListModel *model, guint position, guint removed, guint added, gpointer data)
{
    (void)removed;
    ProjectPresentationProbe *probe = data;
    /* GTK retirement is removal from the toplevel model, not necessarily a
     * visible-property transition on an externally retained window. */
    if (probe->replaced != NULL && !UmiGtk4WindowIsOpen(probe->replaced)) {
        probe->replaced = NULL;
        ++probe->observed;
        ProjectHostClose(probe);
        return;
    }
    for (guint i = 0U; i < added; ++i) {
        GtkWindow *window = g_list_model_get_item(model, position + i);
        if (window == NULL) continue;
        g_ptr_array_add(probe->windows, window);
        if (probe->observed == 0U && strstr(probe->mode, "create-close") != NULL) {
            ++probe->observed; ProjectHostClose(probe);
        } else if (probe->observed == 0U && strstr(probe->mode, "create-reenter") != NULL) {
            ++probe->observed;
            probe->nested = umi_studio_gtk_workbench_execute_command(*probe->slot, probe->command, NULL);
        } else g_signal_connect(window, "notify::visible", G_CALLBACK(ProjectPresented), probe);
    }
}
/* Preserve the old visibility probe for review. The toplevel-model probe
 * above observes actual retirement on platforms that retain widget visibility. */
#if 0
static void PreviousProjectHidden(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    if (!gtk_widget_get_visible(GTK_WIDGET(object))) ProjectHostClose(data);
}
#endif
static int ExerciseProjectPresentation(UmiStudioGtkWorkbench **slot,
    UmiDocumentCoordinator *documents, const char *mode)
{
    int failed = 0;
    GtkWindow *parent = g_object_ref(umi_studio_gtk_workbench_window(*slot)), *previous = NULL;
    const char *command = strstr(mode, "-application-") != NULL ? "project.new-application" : "project.new";
    ProjectPresentationProbe probe = {slot, parent, g_ptr_array_new_with_free_func(g_object_unref),
        mode, command, 0U, 0, UMI_STATUS_INVALID_STATE, NULL};
    GListModel *windows = gtk_window_get_toplevels();
    gulong handler = 0U;
    if (strstr(mode, "replace-close") != NULL) {
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, command, NULL) == UMI_STATUS_OK);
        previous = FindProjectForm(parent, "developer.dialog.accept");
        CHECK(previous != NULL);
        /* Observe removal through the shared model handler installed below. */
#if 0
        g_signal_connect(previous, "notify::visible", G_CALLBACK(PreviousProjectHidden), &probe);
#endif
        probe.replaced = previous;
    }
    handler = g_signal_connect(windows, "items-changed", G_CALLBACK(ProjectWindowAdded), &probe);
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot, command, NULL);
    CHECK(umi_document_coordinator_count(documents) == 0U);
    if (strstr(mode, "close") != NULL) CHECK(*slot == NULL && status == UMI_STATUS_CANCELLED);
    else {
        CHECK(*slot != NULL && status == UMI_STATUS_OK && probe.observed == 1U);
        if (strstr(mode, "reenter") != NULL) CHECK(probe.nested == UMI_STATUS_BUSY);
        if (strstr(mode, "create-reenter") == NULL) CHECK(probe.complete);
        /* A reservation ends with this call; an ordinary later request works. */
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, command, NULL) == UMI_STATUS_OK);
    }
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(windows, handler);
    if (previous != NULL) g_signal_handlers_disconnect_by_data(previous, &probe);
    for (guint i = 0U; i < probe.windows->len; ++i)
        g_signal_handlers_disconnect_by_data(g_ptr_array_index(probe.windows, i), &probe);
    g_clear_object(&previous);
    g_ptr_array_unref(probe.windows);
    g_object_unref(parent);
    return failed;
}

/* Synthetic retained compiler records exercise the real Next/Previous route,
 * while separate real-build cases own compiler execution. A stale file must
 * not trap navigation or hide the still-dirty target editor. */
static int ExerciseAvailableProblem(UmiStudioGtkWorkbench *workbench,
    UmiStudioServices *services, UmiDocumentCoordinator *documents, const char *root, int backwards)
{
    int failed = 0;
    char *source = g_build_filename(root, "diagnostic source.c", NULL);
    char *missing = g_build_filename(root, "deleted source.c", NULL);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(!umi_fs_exists(missing) && g_file_set_contents(source, "first\nsecond\nthird\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, "first\nunsaved diagnostic draft\nthird\n", -1);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    UmiDiagnosticModel *model = umi_diagnostic_pipeline_model(umi_studio_services_diagnostic_pipeline(services));
    for (size_t index = 0U; index < 2U; ++index) {
        int available = backwards ? index == 0U : index == 1U;
        UmiDiagnosticSnapshot problem;
        CHECK(umi_diagnostic_snapshot_init(&problem, available ? "current-source" : "stale-source",
            UMI_DIAGNOSTIC_ERROR, UMI_DIAGNOSTIC_KIND_COMPILER, "qualification", "Compiler source location") == UMI_STATUS_OK);
        CHECK(g_strlcpy(problem.uri, available ? source : missing, sizeof problem.uri) < sizeof problem.uri);
        problem.line = 2U; problem.column = 4U;
        CHECK(umi_diagnostic_model_upsert(model, &problem) == UMI_STATUS_OK);
    }
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench, "diagnostic-review", "Diagnostics") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench, "umicom.studio.layout.diagnostic-review", "Diagnostics") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench,
        backwards ? "navigate.problem.previous" : "navigate.problem.next", NULL) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(umi_path_equal(active.path, source));
    editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    GtkTextIter caret;
    gtk_text_buffer_get_iter_at_mark(buffer, &caret, gtk_text_buffer_get_insert(buffer));
    CHECK(gtk_text_iter_get_line(&caret) == 1 && gtk_text_iter_get_line_index(&caret) == 3);
cleanup:
    g_free(source); g_free(missing);
    return failed;
}

/* Inspect the named native routes, retaining controllers to verify that host
 * teardown revokes their borrowed command authority. No OS keystroke is sent. */
static GtkEventController *FirstUseKeyboard(GtkWidget *window, const char *name)
{
    GListModel *controllers = gtk_widget_observe_controllers(window);
    GtkEventController *found = NULL;
    for (guint index = 0U; index < g_list_model_get_n_items(controllers); ++index) {
        GtkEventController *item = g_list_model_get_item(controllers, index);
        if (g_strcmp0(gtk_event_controller_get_name(item), name) == 0) { found = item; break; }
        g_object_unref(item);
    }
    g_object_unref(controllers);
    return found;
}
static gboolean FirstUseKey(GtkEventController *controller, guint key, GdkModifierType modifiers)
{
    gboolean handled = FALSE;
    g_signal_emit_by_name(controller, "key-pressed", key, 0U, modifiers, &handled);
    return handled;
}
static int ExerciseFileKeyboard(UmiStudioGtkWorkbench **slot,
    UmiDocumentCoordinator *documents, const char *root, const char *mode)
{
    int failed = 0;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    GtkEventController *capture = FirstUseKeyboard(window, "umicom-studio-document-navigation");
    GtkEventController *bubble = FirstUseKeyboard(window, "umicom-studio-keyboard");
    char *source = g_build_filename(root, "keyboard source.c", NULL), *saved = NULL;
    CHECK(capture != NULL && bubble != NULL);
    CHECK(gtk_event_controller_get_propagation_phase(capture) == GTK_PHASE_CAPTURE);
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "keyboard-edit", "Keyboard editing") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.keyboard-edit", "Keyboard editing") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    if (strcmp(mode, "keyboard-save") == 0) {
        CHECK(g_file_set_contents(source, "before\n", -1, NULL));
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    } else {
        if (strcmp(mode, "keyboard-modifiers") == 0) {
            const GdkModifierType extra[] = {GDK_ALT_MASK, GDK_SHIFT_MASK, GDK_SUPER_MASK, GDK_META_MASK};
            for (size_t index = 0U; index < sizeof extra / sizeof extra[0]; ++index) {
                CHECK(!FirstUseKey(capture, GDK_KEY_n, GDK_CONTROL_MASK | extra[index]));
                CHECK(!FirstUseKey(bubble, GDK_KEY_n, GDK_CONTROL_MASK | extra[index]));
            }
            CHECK(!FirstUseKey(capture, GDK_KEY_n, 0));
            CHECK(umi_document_coordinator_count(documents) == 0U);
        }
        CHECK(FirstUseKey(capture, GDK_KEY_N, GDK_CONTROL_MASK | GDK_LOCK_MASK));
        CHECK(umi_document_coordinator_count(documents) == 1U);
    }
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, "edited through the native keyboard route\n", -1);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    if (strcmp(mode, "keyboard-save") == 0) {
        /* Extra desktop modifiers must not save a draft by accident. */
        CHECK(!FirstUseKey(capture, GDK_KEY_s, GDK_CONTROL_MASK | GDK_SUPER_MASK));
        CHECK(!FirstUseKey(bubble, GDK_KEY_s, GDK_CONTROL_MASK | GDK_META_MASK));
        CHECK(g_file_get_contents(source, &saved, NULL, NULL));
        CHECK(strcmp(saved, "before\n") == 0);
        g_clear_pointer(&saved, g_free);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
        CHECK(FirstUseKey(capture, GDK_KEY_s, GDK_CONTROL_MASK));
        CHECK(g_file_get_contents(source, &saved, NULL, NULL));
        CHECK(strcmp(saved, "edited through the native keyboard route\n") == 0);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && !active.dirty);
    } else if (strcmp(mode, "keyboard-retired") == 0) {
        size_t count = umi_document_coordinator_count(documents);
        UmiStudioGtkWorkbench *closing = *slot; *slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
        CHECK(!FirstUseKey(capture, GDK_KEY_n, GDK_CONTROL_MASK));
        CHECK(!FirstUseKey(bubble, GDK_KEY_n, GDK_CONTROL_MASK));
        CHECK(!FirstUseKey(capture, GDK_KEY_Tab, GDK_CONTROL_MASK));
        CHECK(umi_document_coordinator_count(documents) == count);
    }
cleanup:
    g_clear_object(&capture); g_clear_object(&bubble);
    g_free(source); g_free(saved);
    return failed;
}

/* Drive the native field signals and verify model and visible selections agree.
 * Saved source remains unchanged: search works on the unsaved complete draft. */
static int FirstUseSearchSelection(UmiUiDocumentViewModel *views, const char *id,
    GtkTextBuffer *buffer, size_t expected)
{
    UmiUiDocumentViewSnapshot view;
    GtkTextIter start, end;
    return umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
        view.cursor_offset == expected && view.selection_length == 5U &&
        gtk_text_buffer_get_selection_bounds(buffer, &start, &end) &&
        gtk_text_iter_get_offset(&start) == (int)expected &&
        gtk_text_iter_get_offset(&end) == (int)expected + 5;
}
static int ExerciseSearchKeyboard(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *root, const char *mode)
{
    int failed = 0;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    GtkWidget *entry = NULL, *notice = NULL, *locationField = NULL;
    GtkEventController *capture = FirstUseKeyboard(window, "umicom-studio-document-navigation");
    GtkEventController *fieldKeys = NULL;
    char *source = g_build_filename(root, "search source.c", NULL), *saved = NULL, *text = NULL;
    gulong closeHandler = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    const char *draft = "alpha ALPHA alphabet\nalpha\n";
    CHECK(capture != NULL);
    CHECK(g_file_set_contents(source, "saved baseline\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, draft, -1);
    GtkTextIter begin;
    gtk_text_buffer_get_start_iter(buffer, &begin);
    gtk_text_buffer_place_cursor(buffer, &begin);
    CHECK(FirstUseKey(capture, GDK_KEY_f, GDK_CONTROL_MASK));
    GtkWidget *foundEntry = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.find-text");
    CHECK(GTK_IS_ENTRY(foundEntry));
    entry = g_object_ref(foundEntry);
    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(window));
    CHECK(focus == entry || (focus != NULL && gtk_widget_is_ancestor(focus, entry)));
    fieldKeys = FirstUseKeyboard(entry, "studio.editor.find-keys");
    CHECK(fieldKeys != NULL);
    gtk_editable_set_text(GTK_EDITABLE(entry), "alpha");
    g_signal_emit_by_name(entry, "activate");
    CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 0U));
    if (strcmp(mode, "search-location") == 0) {
        GtkWidget *foundLocation = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.location-text");
        CHECK(GTK_IS_ENTRY(foundLocation));
        locationField = g_object_ref(foundLocation);
        gtk_editable_set_text(GTK_EDITABLE(locationField), "2:3");
        g_signal_emit_by_name(locationField, "activate");
        UmiUiDocumentViewSnapshot located;
        CHECK(umi_ui_document_view_model_find(views, active.view_id, &located) == UMI_STATUS_OK);
        CHECK(located.cursor_offset == 23U && located.selection_length == 0U);
        CHECK(Reachable(editor, window));
        gtk_editable_set_text(GTK_EDITABLE(locationField), "0:3");
        g_signal_emit_by_name(locationField, "activate");
        CHECK(umi_ui_document_view_model_find(views, active.view_id, &located) == UMI_STATUS_OK);
        CHECK(located.cursor_offset == 23U && located.selection_length == 0U);
        UmiStudioGtkWorkbench *closing = *slot; *slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
        gtk_editable_set_text(GTK_EDITABLE(locationField), "1:1");
        g_signal_emit_by_name(locationField, "activate");
        CHECK(umi_ui_document_view_model_find(views, active.view_id, &located) == UMI_STATUS_OK);
        CHECK(located.cursor_offset == 23U && located.selection_length == 0U);
    } else if (strcmp(mode, "search-retired") == 0) {
        UmiStudioGtkWorkbench *closing = *slot; *slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
        g_signal_emit_by_name(entry, "activate");
        CHECK(!FirstUseKey(fieldKeys, GDK_KEY_Return, GDK_SHIFT_MASK));
        CHECK(!FirstUseKey(fieldKeys, GDK_KEY_Escape, 0));
        UmiUiDocumentViewSnapshot retained;
        CHECK(umi_ui_document_view_model_find(views, active.view_id, &retained) == UMI_STATUS_OK);
        CHECK(retained.cursor_offset == 0U && retained.selection_length == 5U);
    } else if (strcmp(mode, "search-feedback-close") == 0) {
        GtkWidget *foundNotice = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
        CHECK(GTK_IS_LABEL(foundNotice));
        notice = g_object_ref(foundNotice);
        closeHandler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), slot);
        g_signal_emit_by_name(entry, "activate");
        CHECK(*slot == NULL);
        UmiUiDocumentViewSnapshot selected;
        CHECK(umi_ui_document_view_model_find(views, active.view_id, &selected) == UMI_STATUS_OK);
        CHECK(selected.cursor_offset == 6U && selected.selection_length == 5U);
    } else {
        if (strcmp(mode, "search-options") == 0) {
            GtkWidget *matchCase = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.search-case");
            GtkWidget *word = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.search-whole-word");
            CHECK(GTK_IS_DROP_DOWN(matchCase) && GTK_IS_CHECK_BUTTON(word));
            gtk_drop_down_set_selected(GTK_DROP_DOWN(matchCase), 1U);
            gtk_check_button_set_active(GTK_CHECK_BUTTON(word), TRUE);
            g_signal_emit_by_name(entry, "activate");
            CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 21U));
        } else {
            g_signal_emit_by_name(entry, "activate");
            CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 6U));
        }
        CHECK(!FirstUseKey(fieldKeys, GDK_KEY_Return, GDK_SHIFT_MASK | GDK_CONTROL_MASK));
        CHECK(FirstUseKey(fieldKeys, GDK_KEY_KP_Enter, GDK_SHIFT_MASK));
        CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 0U));
        gtk_editable_set_text(GTK_EDITABLE(entry), "");
        g_signal_emit_by_name(entry, "activate");
        CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 0U));
        CHECK(FirstUseKey(fieldKeys, GDK_KEY_Escape, 0));
        CHECK(Reachable(editor, window));
    }
    size_t length = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &length) == UMI_STATUS_OK);
    CHECK(length == strlen(draft) && strcmp(text, draft) == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(g_file_get_contents(source, &saved, NULL, NULL) && strcmp(saved, "saved baseline\n") == 0);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(notice, closeHandler);
    g_clear_object(&notice); g_clear_object(&entry); g_clear_object(&locationField);
    g_clear_object(&capture); g_clear_object(&fieldKeys);
    g_free(source); g_free(saved); UmiUiDocumentViewModelFreeText(text);
    return failed;
}

/* A launch review must remain visible through the real command route, without
 * saving a dirty source or producing a job. These cases use inert files. */
static int ExerciseRunFeedback(UmiStudioGtkWorkbench **workbenchSlot,
    UmiStudioServices *services, UmiDocumentCoordinator *documents, UmiUiDocumentViewModel *views,
    const char *directory, const char *mode)
{
    int failed = 0;
    UmiBuildProfile *profile = NULL;
    char *source = g_build_filename(directory, "launch-review.c", NULL);
    char *program = g_build_filename(directory, "selected-program", NULL);
    char *text = NULL, *saved = NULL;
    GtkWidget *notice = NULL;
    gulong closeHandler = 0U;
    const char *draft = "#error preserve this unsaved draft\n";
    const char *expectedText = "selected program file";
    UmiStatus expected = UMI_STATUS_NOT_FOUND;
    UmiStudioGtkWorkbench *workbench = *workbenchSlot;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    UmiStudioBuildService *build = umi_studio_services_build(services);
    CHECK(g_file_set_contents(source, "saved baseline\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), draft, -1);
    CHECK(umi_studio_workspace_set_trusted(services, strcmp(mode, "run-untrusted") != 0) == UMI_STATUS_OK);
    profile = g_new(UmiBuildProfile, 1);
    *profile = *umi_studio_build_service_profile(build);
    strcpy(profile->run_program, "./missing-program");
    strcpy(profile->run_working_directory, ".");
    if (strcmp(mode, "run-untrusted") == 0) {
        expected = UMI_STATUS_PERMISSION_DENIED;
        expectedText = "enable its build tools";
    } else if (strcmp(mode, "run-unconfigured") == 0) {
        profile->run_program[0] = '\0';
        expected = UMI_STATUS_INVALID_STATE;
        expectedText = "Select Program to run";
    } else if (strcmp(mode, "run-working-folder") == 0) {
        CHECK(g_file_set_contents(program, "inert program fixture\n", -1, NULL));
        strcpy(profile->run_program, "./selected-program");
        strcpy(profile->run_working_directory, "missing-folder");
        expectedText = "Run working folder";
    }
    CHECK(umi_studio_build_service_set_profile(build, profile) == UMI_STATUS_OK);
    GtkWidget *label = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
    CHECK(GTK_IS_LABEL(label));
    /* The generic retained notification owns a GObject reference, so use its checked parent-type conversion. Retained for engineering review. */
#if 0
notice = g_object_ref(label);
#endif
/* This receiver is a GtkWidget; keep the retained reference at that type instead of assigning its GObject parent pointer. The previous statement is retained for review. */
#if 0
notice = g_object_ref(G_OBJECT(label));
#endif
    notice = GTK_WIDGET(g_object_ref(label));
    if (strcmp(mode, "run-feedback-close") == 0)
        closeHandler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), workbenchSlot);
    /* Closing the native owner during feedback changes the public completion
     * result to CANCELLED. Keep the former service-only expectation for review;
     * the draft and process checks below still prove that refusal is harmless. */
#if 0
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL) == expected);
#endif
    UmiStatus completed = umi_studio_gtk_workbench_execute_command(
        workbench, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL);
    UmiStatus expectedCompletion = strcmp(mode, "run-feedback-close") == 0
        ? UMI_STATUS_CANCELLED : expected;
    if (completed != expectedCompletion)
        fprintf(stderr, "Run completion: expected %s, received %s\n",
            umi_status_text(expectedCompletion), umi_status_text(completed));
    CHECK(completed == expectedCompletion);
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(notice)), expectedText) != NULL);
    CHECK(!UmiStudioBuildBusy(build));
    UmiBuildProjectSessionSnapshot progress;
    CHECK(UmiStudioBuildProgress(build, &progress) == UMI_STATUS_OK);
    CHECK(!progress.active && progress.completed_phase_count == 0U);
    CHECK(umi_build_profile_equal(profile, umi_studio_build_service_profile(build)));
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    size_t length = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &length) == UMI_STATUS_OK);
    CHECK(length == strlen(draft) && strcmp(text, draft) == 0);
    CHECK(g_file_get_contents(source, &saved, NULL, NULL) && strcmp(saved, "saved baseline\n") == 0);
    if (strcmp(mode, "run-feedback-close") == 0) CHECK(*workbenchSlot == NULL);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(notice, closeHandler);
    g_clear_object(&notice);
    g_free(source); g_free(program); g_free(profile); g_free(saved);
    UmiUiDocumentViewModelFreeText(text);
    return failed;
}

/* Replacement uses the complete unsaved draft and the same options as Find.
 * Retained fields lose authority when their workbench is destroyed. */
static int ExerciseReplaceKeyboard(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *root, const char *mode)
{
    int failed = 0;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    GtkWidget *replace = NULL, *notice = NULL;
    GtkEventController *capture = FirstUseKeyboard(window, "umicom-studio-document-navigation");
    char *source = g_build_filename(root, "replace source.c", NULL), *text = NULL, *saved = NULL;
    gulong closeHandler = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    const char *draft = "alpha ALPHA alphabet\nalpha\n";
    const char *first = "beta ALPHA alphabet\nalpha\n";
    const char *expected = first;
    CHECK(capture != NULL && g_file_set_contents(source, "saved baseline\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, draft, -1);
    GtkTextIter begin;
    gtk_text_buffer_get_start_iter(buffer, &begin);
    gtk_text_buffer_place_cursor(buffer, &begin);
    GtkWidget *find = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.find-text");
    GtkWidget *replaceField = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.replace-text");
    CHECK(GTK_IS_ENTRY(find) && GTK_IS_ENTRY(replaceField));
    replace = g_object_ref(replaceField);
    gtk_editable_set_text(GTK_EDITABLE(find), "alpha");
    gtk_editable_set_text(GTK_EDITABLE(replace), "beta");
    if (strcmp(mode, "replace-options") == 0) {
        GtkWidget *matchCase = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.search-case");
        GtkWidget *wholeWord = umi_gtk4_automation_find_tagged_widget(window, "studio.editor.search-whole-word");
        CHECK(GTK_IS_DROP_DOWN(matchCase) && GTK_IS_CHECK_BUTTON(wholeWord));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(matchCase), 1U);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(wholeWord), TRUE);
    }
    g_signal_emit_by_name(find, "activate");
    CHECK(FirstUseSearchSelection(views, active.view_id, buffer, 0U));
    CHECK(!FirstUseKey(capture, GDK_KEY_h, GDK_CONTROL_MASK | GDK_ALT_MASK));
    CHECK(FirstUseKey(capture, GDK_KEY_h, GDK_CONTROL_MASK));
    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(window));
    CHECK(focus == replace || (focus != NULL && gtk_widget_is_ancestor(focus, replace)));
    if (strcmp(mode, "replace-retired") == 0) {
        UmiStudioGtkWorkbench *closing = *slot; *slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
        g_signal_emit_by_name(replace, "activate");
        CHECK(!FirstUseKey(capture, GDK_KEY_h, GDK_CONTROL_MASK));
        expected = draft;
    } else {
        if (strcmp(mode, "replace-feedback-close") == 0) {
            GtkWidget *found = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
            CHECK(GTK_IS_LABEL(found));
            notice = g_object_ref(found);
            closeHandler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), slot);
        }
        g_signal_emit_by_name(replace, "activate");
        if (strcmp(mode, "replace-feedback-close") == 0) CHECK(*slot == NULL);
        else {
            size_t count = 0U;
            CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &count) == UMI_STATUS_OK);
            CHECK(strcmp(text, first) == 0);
            UmiUiDocumentViewModelFreeText(text); text = NULL;
            g_signal_emit_by_name(replace, "activate");
            CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &count) == UMI_STATUS_OK);
            CHECK(strcmp(text, strcmp(mode, "replace-options") == 0
                ? "beta ALPHA alphabet\nbeta\n" : "beta beta alphabet\nalpha\n") == 0);
            UmiUiDocumentViewModelFreeText(text); text = NULL;
            CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
            CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &count) == UMI_STATUS_OK);
            CHECK(strcmp(text, first) == 0);
            UmiUiDocumentViewModelFreeText(text); text = NULL;
            CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
            expected = draft;
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
            GtkTextIter start, end;
            gtk_text_buffer_get_bounds(buffer, &start, &end);
            gchar *visible = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
            int same = strcmp(visible, draft) == 0;
            g_free(visible);
            CHECK(same);
        }
    }
    size_t length = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &length) == UMI_STATUS_OK);
    CHECK(length == strlen(expected) && strcmp(text, expected) == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(g_file_get_contents(source, &saved, NULL, NULL) && strcmp(saved, "saved baseline\n") == 0);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(notice, closeHandler);
    g_clear_object(&replace); g_clear_object(&notice); g_clear_object(&capture);
    g_free(source); g_free(saved); UmiUiDocumentViewModelFreeText(text);
    return failed;
}

/* Use a real CTest catalogue and executable, with Studio launched from a
 * different directory. This catches a process-cwd dependency that a catalogue
 * mock cannot reveal. No compiler is required; CMake supplies the test body. */
/* The catalogue fixture now also exercises window destruction during completion. The original live-window fixture is retained for review. */
#if 0
static int ExerciseNativeDiscovery(UmiStudioGtkWorkbench *workbench,
    UmiStudioServices *services, const char *fixture, const char *scenario)
{
    int failed = 0;
    char *root = g_build_filename(fixture, "Selected project", NULL);
    char *buildRoot = g_build_filename(root, "build", "selected", NULL);
    char *metadata = g_build_filename(buildRoot, "CTestTestfile.cmake", NULL);
    char *cmake = g_find_program_in_path("cmake");
    char *escaped = NULL, *catalogue = NULL;
    CHECK(cmake != NULL);
    escaped = g_strescape(cmake, NULL);
    catalogue = g_strdup_printf("add_test(project.smoke \"%s\" \"-E\" \"true\")\n", escaped);
    CHECK(g_mkdir_with_parents(buildRoot, 0700) == 0);
    CHECK(g_file_set_contents(metadata, catalogue, -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiBuildProfile profile = *umi_studio_build_service_profile(build);
    CHECK(g_strlcpy(profile.build_directory,
        strcmp(scenario, "discovery-absolute") == 0 ? buildRoot : "build/selected",
        sizeof profile.build_directory) < sizeof profile.build_directory);
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
    /* Only this synthetic catalogue is trusted. The selected source root is
     * intentionally not the working directory selected by the outer fixture. */
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
    UmiStudioTestService *tests = umi_studio_services_tests(services);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_TESTS_DISCOVER, NULL) == UMI_STATUS_OK);
    gint64 deadline = g_get_monotonic_time() + 30000000;
    while (UmiStudioTestDiscoveryPending(tests) && g_get_monotonic_time() < deadline) {
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        while (g_main_context_pending(NULL)) (void)g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    CHECK(!UmiStudioTestDiscoveryPending(tests));
    UmiStudioTestExplorerState explorer;
    CHECK(umi_studio_test_service_explorer_state(tests, &explorer) == UMI_STATUS_OK);
    CHECK(umi_path_equal(explorer.workspace_root, root) && strcmp(explorer.active_project_id, "workspace") == 0);
    UmiTestPlatformOperationPlan plan;
    CHECK(umi_studio_test_service_plan_all(tests, 1U, 0, &plan) == UMI_STATUS_OK && plan.selection.count == 1U);
    CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(build)));
    if (strcmp(scenario, "discovery-run-relative") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_TESTS_RUN_ALL, NULL) == UMI_STATUS_OK);
        deadline = g_get_monotonic_time() + 30000000;
        while (UmiStudioTestRunPending(tests) && g_get_monotonic_time() < deadline) {
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            while (g_main_context_pending(NULL)) (void)g_main_context_iteration(NULL, FALSE);
            g_usleep(1000U);
        }
        CHECK(!UmiStudioTestRunPending(tests));
        UmiStudioTestSnapshot result;
        CHECK(umi_studio_test_service_snapshot(tests, &result) == UMI_STATUS_OK);
        CHECK(result.passed == 1U && result.failed == 0U && result.skipped == 0U);
    }
cleanup:
    g_free(catalogue); g_free(escaped); g_free(cmake);
    g_free(metadata); g_free(buildRoot); g_free(root);
    return failed;
}

#endif

/* Observe completion before submitting: small catalogues may finish during
 * the command's first refresh. Ignore acceptance/progress while work is pending. */
typedef struct DiscoveryFeedbackProbe {
    UmiStudioGtkWorkbench **slot;
    UmiStudioTestService *tests;
    int runningTests;
    unsigned completed;
} DiscoveryFeedbackProbe;
static void CloseWhenDiscoveryCompleted(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)object;
    (void)spec;
    DiscoveryFeedbackProbe *probe = data;
    int pending = probe->runningTests
        ? UmiStudioTestRunPending(probe->tests)
        : UmiStudioTestDiscoveryPending(probe->tests);
    if (pending || *probe->slot == NULL) return;
    ++probe->completed;
    UmiStudioGtkWorkbench *closing = *probe->slot;
    *probe->slot = NULL;
    umi_studio_gtk_workbench_destroy(closing);
}

static int ExerciseNativeDiscovery(UmiStudioGtkWorkbench **workbenchSlot,
    UmiStudioServices *services, const char *fixture, const char *scenario)
{
    int failed = 0;
    UmiStudioGtkWorkbench *workbench = *workbenchSlot;
    GObject *notice = NULL;
    gulong closeHandler = 0U;
    DiscoveryFeedbackProbe feedback = {workbenchSlot, NULL, 0, 0U};
    char *root = g_build_filename(fixture, "Selected project", NULL);
    char *buildRoot = g_build_filename(root, "build", "selected", NULL);
    char *metadata = g_build_filename(buildRoot, "CTestTestfile.cmake", NULL);
    char *cmake = g_find_program_in_path("cmake");
    char *escaped = NULL, *catalogue = NULL;
    CHECK(cmake != NULL);
    escaped = g_strescape(cmake, NULL);
    catalogue = g_strdup_printf("add_test(project.smoke \"%s\" \"-E\" \"true\")\n", escaped);
    CHECK(g_mkdir_with_parents(buildRoot, 0700) == 0);
    CHECK(g_file_set_contents(metadata, catalogue, -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", root) == UMI_STATUS_OK);
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiBuildProfile profile = *umi_studio_build_service_profile(build);
    CHECK(g_strlcpy(profile.build_directory,
        strcmp(scenario, "discovery-absolute") == 0 ? buildRoot : "build/selected",
        sizeof profile.build_directory) < sizeof profile.build_directory);
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
    /* Only this synthetic catalogue is trusted. The selected source root is
     * intentionally not the working directory selected by the outer fixture. */
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
    UmiStudioTestService *tests = umi_studio_services_tests(services);
    feedback.tests = tests;
    /* Retain the former submit-before-observe ordering for comparison. It
     * could miss completion from a small catalogue on a fast machine. */
#if 0
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_TESTS_DISCOVER, NULL) == UMI_STATUS_OK);
#endif
    if (strcmp(scenario, "discovery-feedback-close") == 0) {
        GtkWidget *label = umi_gtk4_automation_find_tagged_widget(
            GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.notification");
        CHECK(label != NULL);
        /* The generic retained notification owns a GObject reference, so use its checked parent-type conversion. Retained for engineering review. */
#if 0
notice = g_object_ref(label);
#endif
notice = g_object_ref(G_OBJECT(label));
        closeHandler = g_signal_connect(notice, "notify::label",
            G_CALLBACK(CloseWhenDiscoveryCompleted), &feedback);
    }
    UmiStatus submitted = umi_studio_gtk_workbench_execute_command(
        workbench, UMI_STUDIO_COMMAND_TESTS_DISCOVER, NULL);
    CHECK(submitted == (*workbenchSlot != NULL ? UMI_STATUS_OK : UMI_STATUS_CANCELLED));
    gint64 deadline = g_get_monotonic_time() + 30000000;
    while (UmiStudioTestDiscoveryPending(tests) && g_get_monotonic_time() < deadline) {
        UmiStatus pollStatus = umi_studio_gtk_workbench_workspace_synchronise(*workbenchSlot);
        if (*workbenchSlot == NULL) {
            CHECK(strcmp(scenario, "discovery-feedback-close") == 0 && pollStatus == UMI_STATUS_CANCELLED);
            break;
        }
        CHECK(pollStatus == UMI_STATUS_OK);
        while (g_main_context_pending(NULL)) (void)g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    CHECK(!UmiStudioTestDiscoveryPending(tests));
    if (strcmp(scenario, "discovery-feedback-close") == 0) {
        CHECK(*workbenchSlot == NULL && feedback.completed == 1U);
        goto cleanup;
    }
    UmiStudioTestExplorerState explorer;
    CHECK(umi_studio_test_service_explorer_state(tests, &explorer) == UMI_STATUS_OK);
    CHECK(umi_path_equal(explorer.workspace_root, root) && strcmp(explorer.active_project_id, "workspace") == 0);
    UmiTestPlatformOperationPlan plan;
    CHECK(umi_studio_test_service_plan_all(tests, 1U, 0, &plan) == UMI_STATUS_OK && plan.selection.count == 1U);
    CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(build)));
    if (strcmp(scenario, "discovery-run-relative") == 0 ||
        strcmp(scenario, "discovery-run-feedback-close") == 0) {
        /* Install completion observation before submission, as for discovery. */
#if 0
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_TESTS_RUN_ALL, NULL) == UMI_STATUS_OK);
#endif
        feedback.runningTests = 1;
        if (strcmp(scenario, "discovery-run-feedback-close") == 0) {
            GtkWidget *label = umi_gtk4_automation_find_tagged_widget(
                GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.notification");
            CHECK(label != NULL);
            /* The generic retained notification owns a GObject reference, so use its checked parent-type conversion. Retained for engineering review. */
#if 0
notice = g_object_ref(label);
#endif
notice = g_object_ref(G_OBJECT(label));
            closeHandler = g_signal_connect(notice, "notify::label",
                G_CALLBACK(CloseWhenDiscoveryCompleted), &feedback);
        }
        submitted = umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_TESTS_RUN_ALL, NULL);
        CHECK(submitted == (*workbenchSlot != NULL ? UMI_STATUS_OK : UMI_STATUS_CANCELLED));
        deadline = g_get_monotonic_time() + 30000000;
        while (UmiStudioTestRunPending(tests) && g_get_monotonic_time() < deadline) {
            UmiStatus pollStatus = umi_studio_gtk_workbench_workspace_synchronise(*workbenchSlot);
            if (*workbenchSlot == NULL) {
                CHECK(strcmp(scenario, "discovery-run-feedback-close") == 0 && pollStatus == UMI_STATUS_CANCELLED);
                break;
            }
            CHECK(pollStatus == UMI_STATUS_OK);
            while (g_main_context_pending(NULL)) (void)g_main_context_iteration(NULL, FALSE);
            g_usleep(1000U);
        }
        CHECK(!UmiStudioTestRunPending(tests));
        if (strcmp(scenario, "discovery-run-feedback-close") == 0)
            CHECK(*workbenchSlot == NULL && feedback.completed == 1U);
        UmiStudioTestSnapshot result;
        CHECK(umi_studio_test_service_snapshot(tests, &result) == UMI_STATUS_OK);
        CHECK(result.passed == 1U && result.failed == 0U && result.skipped == 0U);
    }
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(notice, closeHandler);
    g_clear_object(&notice);
    g_free(catalogue); g_free(escaped); g_free(cmake);
    g_free(metadata); g_free(buildRoot); g_free(root);
    return failed;
}

/* Keep real confirmation controls past cancellation and host retirement. Their
 * lifetime may exceed a dialog's visible lifetime, but their authority must not.
 * The draft is checked through the shared document service after rejected clicks. */
static int ExerciseCloseConfirmation(UmiStudioGtkWorkbench **slot,
    UmiDocumentCoordinator *documents, const char *mode)
{
    int failed = 0;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkWindow *prompt = NULL;
    GtkWidget *cancel = NULL, *discard = NULL, *foreign = NULL;
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.new", NULL) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(editor != NULL);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)),
        "keep this unsaved draft", -1);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
    gtk_window_present(window);
    gtk_window_close(window);
    prompt = FindProjectForm(window, "studio.close.discard");
    CHECK(prompt != NULL && gtk_widget_get_visible(GTK_WIDGET(window)));
    GtkWidget *foundCancel = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(prompt), "studio.close.cancel");
    GtkWidget *foundDiscard = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(prompt), "studio.close.discard");
    CHECK(GTK_IS_BUTTON(foundCancel) && GTK_IS_BUTTON(foundDiscard));
    cancel = g_object_ref(foundCancel);
    discard = g_object_ref(foundDiscard);
    if (strcmp(mode, "close-dialog-confirm") == 0) {
        g_signal_emit_by_name(discard, "clicked");
        CHECK(!gtk_widget_get_visible(GTK_WIDGET(window)));
        /* Keep the native window referenced until adapter teardown completes. */
        umi_studio_gtk_workbench_destroy(*slot);
        *slot = NULL;
    } else if (strcmp(mode, "close-dialog-retired-owner") == 0) {
        umi_studio_gtk_workbench_destroy(*slot);
        *slot = NULL;
        g_signal_emit_by_name(discard, "clicked");
        g_signal_emit_by_name(cancel, "clicked");
        CHECK(!gtk_widget_get_visible(GTK_WIDGET(window)));
    } else if (strcmp(mode, "close-dialog-reparented-cancel") == 0) {
        GtkWidget *parent = gtk_widget_get_parent(cancel);
        CHECK(GTK_IS_BOX(parent));
        gtk_box_remove(GTK_BOX(parent), cancel);
        foreign = g_object_ref(gtk_window_new());
        gtk_window_set_child(GTK_WINDOW(foreign), cancel);
        gtk_window_present(GTK_WINDOW(foreign));
        g_signal_emit_by_name(cancel, "clicked");
        CHECK(gtk_widget_get_visible(foreign) && gtk_widget_get_visible(GTK_WIDGET(prompt)));
        CHECK(gtk_widget_get_visible(GTK_WIDGET(window)));
    } else {
        g_signal_emit_by_name(cancel, "clicked");
        CHECK(!gtk_widget_get_visible(GTK_WIDGET(prompt)));
        if (strcmp(mode, "close-dialog-retained-button") == 0)
            g_clear_object(&prompt);
        if (strcmp(mode, "close-dialog-cancel") != 0) {
            g_signal_emit_by_name(discard, "clicked");
            g_signal_emit_by_name(cancel, "clicked");
        }
        CHECK(gtk_widget_get_visible(GTK_WIDGET(window)));
    }
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
cleanup:
    if (foreign != NULL) gtk_window_destroy(GTK_WINDOW(foreign));
    if (prompt != NULL) gtk_window_destroy(prompt);
    g_clear_object(&foreign);
    g_clear_object(&cancel);
    g_clear_object(&discard);
    g_clear_object(&prompt);
    g_clear_object(&window);
    return failed;
}

typedef struct {
    UmiStudioGtkWorkbench **slot;
    const char *prefix;
} FirstUseSequenceObserver;

static void CloseOnSequenceFeedback(GObject *object, GParamSpec *spec, gpointer data)
{
    FirstUseSequenceObserver *observer = data;
    (void)spec;
    if (*observer->slot != NULL &&
        g_str_has_prefix(gtk_label_get_text(GTK_LABEL(object)), observer->prefix)) {
        UmiStudioGtkWorkbench *closing = *observer->slot;
        *observer->slot = NULL;
        umi_studio_gtk_workbench_destroy(closing);
    }
}

/* Drive the normal idle-based Framework sequence through Studio's public command
 * route. Disk contents distinguish admission, completion and cancellation. */
static int ExerciseDocumentSequence(UmiStudioGtkWorkbench **slot,
    UmiDocumentCoordinator *documents, const char *root, const char *mode)
{
    int failed = 0;
    int saving = strncmp(mode, "batch-save", 10U) == 0;
    int startClose = strstr(mode, "-start-close") != NULL;
    int completionClose = strstr(mode, "-completion-close") != NULL;
    int cancelling = strstr(mode, "-cancel") != NULL;
    char *paths[2] = {NULL, NULL};
    char *disk = NULL;
    GtkWidget *notice = NULL;
    gulong handler = 0U;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    const char *finalPrefix = saving ? "Save All complete:" : "Close documents complete:";
    FirstUseSequenceObserver observer = {slot, startClose
        ? (saving ? "Save All started." : "Closing source documents.") : finalPrefix};
    for (size_t index = 0U; index < 2U; ++index) {
        paths[index] = g_build_filename(root, index == 0U ? "sequence one.c" : "sequence two.c", NULL);
        CHECK(g_file_set_contents(paths[index], "saved source\n", -1, NULL));
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", paths[index]) == UMI_STATUS_OK);
        if (saving) {
            UmiDocumentWorkingCopySnapshot active;
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
            GtkWidget *editor = FindEditor(window, active.view_id);
            CHECK(editor != NULL);
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), "new source\n", -1);
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
        }
    }
    GtkWidget *found = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
    CHECK(GTK_IS_LABEL(found));
    notice = g_object_ref(found);
    if (startClose || completionClose)
        handler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseOnSequenceFeedback), &observer);
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot,
        saving ? "file.save-all" : "file.close-all", NULL);
    CHECK(status == UMI_STATUS_OK || (startClose && status == UMI_STATUS_CANCELLED));
    if (cancelling) {
        CHECK(*slot != NULL);
        CHECK(umi_studio_gtk_workbench_execute_command(*slot,
            saving ? "file.save-all.cancel" : "file.close-all.cancel", NULL) == UMI_STATUS_OK);
        finalPrefix = saving ? "Save All cancelled:" : "Close documents cancelled:";
    }
    if (!startClose) {
        gint64 deadline = g_get_monotonic_time() + 10000000;
        while (!g_str_has_prefix(gtk_label_get_text(GTK_LABEL(notice)), finalPrefix) &&
               g_get_monotonic_time() < deadline) {
            (void)g_main_context_iteration(NULL, FALSE);
            g_usleep(1000U);
        }
        CHECK(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(notice)), finalPrefix));
    }
    CHECK((startClose || completionClose) ? *slot == NULL : *slot != NULL);
    CHECK(umi_document_coordinator_count(documents) ==
        ((!saving && !startClose && !cancelling) ? 0U : 2U));
    for (size_t index = 0U; index < 2U; ++index) {
        CHECK(g_file_get_contents(paths[index], &disk, NULL, NULL));
        CHECK(strcmp(disk, saving && !startClose && !cancelling ? "new source\n" : "saved source\n") == 0);
        g_clear_pointer(&disk, g_free);
    }
    if (saving && (startClose || cancelling)) {
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    }
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(notice, handler);
    g_clear_object(&notice);
    g_free(disk);
    g_free(paths[0]);
    g_free(paths[1]);
    return failed;
}

/* Native history commands must bring the destination into the user's layout,
 * not merely change the coordinator's active ID behind a hidden editor pane. */
static int ExerciseVisibleHistory(UmiStudioGtkWorkbench **slot,
    UmiStudioUi *ui, const char *root, const char *mode)
{
    int failed = 0;
    char *first = g_build_filename(root, "history first.c", NULL);
    char *second = g_build_filename(root, "history second.c", NULL);
    char *text = NULL;
    size_t length = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    gulong closeHandler = 0U;
    int reopen = strstr(mode, "reopen") != NULL;
    int missing = strcmp(mode, "history-reopen-missing") == 0;
    int closeDuringFocus = strcmp(mode, "history-reopen-focus-close") == 0;
    int forward = strcmp(mode, "history-forward-hidden") == 0;
    UmiDocumentWorkingCopySnapshot active;
    CHECK(g_file_set_contents(first, "first source\n", -1, NULL));
    CHECK(g_file_set_contents(second, "second source\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", first) == UMI_STATUS_OK);
    if (reopen) {
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.close", NULL) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_count(documents) == 0U);
        if (missing) CHECK(g_remove(first) == 0);
    } else {
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", second) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(editor != NULL);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), "unsaved second\n", -1);
        if (forward) CHECK(umi_studio_gtk_workbench_execute_command(*slot, "navigate.back", NULL) == UMI_STATUS_OK);
    }
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "history-hidden", "History") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.history-hidden", "History") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    if (closeDuringFocus)
        closeHandler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot,
        reopen ? "file.reopen-closed" : (forward ? "navigate.forward" : "navigate.back"), NULL);
    if (missing) {
        CHECK(status != UMI_STATUS_OK);
        CHECK(umi_document_coordinator_count(documents) == 0U);
    } else if (closeDuringFocus) {
        CHECK(*slot == NULL && status == UMI_STATUS_CANCELLED);
    } else {
        CHECK(status == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, forward ? second : first));
        GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
        if (!reopen) {
            /* Return to the second draft and prove history navigation did not
             * reload its disk copy or save its edits. */
            if (!forward) CHECK(umi_studio_gtk_workbench_execute_command(*slot, "navigate.forward", NULL) == UMI_STATUS_OK);
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
            CHECK(umi_path_equal(active.path, second));
            CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &length) == UMI_STATUS_OK);
            CHECK(length == strlen("unsaved second\n") && strcmp(text, "unsaved second\n") == 0);
        }
    }
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(window, closeHandler);
    g_clear_object(&window);
    UmiUiDocumentViewModelFreeText(text);
    g_free(first);
    g_free(second);
    return failed;
}

/* Retain actual toolbar/welcome controls across workbench teardown. A signal
 * emitted on a retired widget must not regain command or local-window authority. */
static int ExerciseCommandButtons(UmiStudioGtkWorkbench **slot,
    UmiDocumentCoordinator *documents, const char *root, const char *mode)
{
    int failed = 0;
    GtkWidget *button = NULL;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    char *source = g_build_filename(root, "button source.c", NULL);
    int opening = strcmp(mode, "button-open") == 0 || strcmp(mode, "button-retired-argument") == 0;
    int retired = strstr(mode, "retired") != NULL;
    const char *localTag = strcmp(mode, "button-retired-palette") == 0
        ? "studio.local.command-palette" : strcmp(mode, "button-retired-layout") == 0
        ? "studio.local.layout-browser" : NULL;
    GtkWidget *found = localTag != NULL
        ? umi_gtk4_automation_find_tagged_widget(window, localTag)
        : FindAction(window, opening ? "file.open" : "file.new");
    CHECK(GTK_IS_BUTTON(found));
    button = g_object_ref(found);
    CHECK(g_file_set_contents(source, "button source\n", -1, NULL));
    if (opening)
        g_object_set_data_full(G_OBJECT(button), "umicom-command-argument", g_strdup(source), g_free);
    if (retired) {
        umi_studio_gtk_workbench_destroy(*slot);
        *slot = NULL;
    }
    guint windowsBefore = g_list_model_get_n_items(gtk_window_get_toplevels());
    g_signal_emit_by_name(button, "clicked");
/* The explicit-path button does not create a multi-file open batch; waiting for that absent batch rejected successful opens. The previous implementation is retained for review. */
#if 0
    /* Native Open queues captured paths. Wait for its actual completion before
     * inspecting the document; new-file and retired controls remain immediate. */
    if (opening && !retired) {
        gint64 deadline = g_get_monotonic_time() + 10000000;
        UmiDocumentOpenBatchSummary summary;
        int running = 1;
        while (running && g_get_monotonic_time() < deadline) {
            for (unsigned step = 0U; step < 32U && g_main_context_pending(NULL); ++step)
                (void)g_main_context_iteration(NULL, FALSE);
            CHECK(umi_studio_gtk_workbench_open_files_snapshot(*slot, &summary, &running) == UMI_STATUS_OK);
            if (running) g_usleep(1000U);
        }
        CHECK(!running);
    }

#endif
    /* A button with an explicit path uses RuntimeOpenSourcePath synchronously.
     * The active document and reachable editor checks below are the observable
     * contract. Multi-file chooser completion has separate batch coverage. */
    if (retired) {
        CHECK(umi_document_coordinator_count(documents) == 0U);
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == windowsBefore);
    } else {
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(Reachable(FindEditor(window, active.view_id), window));
        CHECK(opening ? active.has_path && umi_path_equal(active.path, source) : !active.has_path);
    }
cleanup:
    g_clear_object(&button);
    g_free(source);
    return failed;
}

/* A refused tool command must leave the editor's dirty bytes and the saved file
 * unchanged. Exercise aliases, canonical commands and the scheduling argument
 * through the public native host, rather than just the command registry. */
static int ExerciseBuildDenied(UmiStudioGtkWorkbench **slot,
    UmiStudioServices *services, UmiDocumentCoordinator *documents,
    UmiUiDocumentViewModel *views, const char *directory, const char *mode)
{
    int failed = 0;
    char *source = g_build_filename(directory, "pending-source.c", NULL);
    char *other = g_build_filename(directory, "Other project", NULL);
    char *disk = NULL, *text = NULL;
    GtkWidget *notice = NULL;
    gulong closeHandler = 0U;
    const char *draft = "int pending_change = 42;\n";
    const int closed = strcmp(mode, "build-denied-closed") == 0;
    const int mismatch = strcmp(mode, "build-denied-other-project") == 0;
    const int closeOnFeedback = strcmp(mode, "build-denied-feedback-close") == 0;
    const UmiStatus expected = closed || mismatch ? UMI_STATUS_INVALID_STATE : UMI_STATUS_PERMISSION_DENIED;
    const char *reason = closed ? "Open a project" : mismatch ? "no longer matches" : "enable its build tools";
    UmiStudioBuildService *build = umi_studio_services_build(services);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(*slot));
    CHECK(g_file_set_contents(source, "int saved_change = 1;\n", -1, NULL));
    CHECK(g_mkdir_with_parents(other, 0700) == 0);
    if (!closed) {
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", directory) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, mismatch) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(Reachable(editor, window));
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), draft, -1);
    UmiBuildProfile profile = *umi_studio_build_service_profile(build);
    if (mismatch) {
        CHECK(g_strlcpy(profile.source_directory, other, sizeof profile.source_directory) < sizeof profile.source_directory);
        CHECK(umi_studio_build_service_set_profile(build, &profile) == UMI_STATUS_OK);
    }
    GtkWidget *label = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
    CHECK(GTK_IS_LABEL(label));
    /* The generic retained notification owns a GObject reference, so use its checked parent-type conversion. Retained for engineering review. */
#if 0
notice = g_object_ref(label);
#endif
/* This receiver is a GtkWidget; keep the retained reference at that type instead of assigning its GObject parent pointer. The previous statement is retained for review. */
#if 0
notice = g_object_ref(G_OBJECT(label));
#endif
    notice = GTK_WIDGET(g_object_ref(label));
    if (closeOnFeedback)
        closeHandler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), slot);
    UmiStudioBuildSnapshot before, after;
    CHECK(umi_studio_build_service_snapshot(build, &before) == UMI_STATUS_OK);
    static const struct { const char *menu; const char *canonical; } commands[] = {
        {"build.configure", UMI_STUDIO_COMMAND_BUILD_CONFIGURE},
        {"build.build", UMI_STUDIO_COMMAND_BUILD_COMPILE},
        {"build.test", UMI_STUDIO_COMMAND_BUILD_TEST},
        {"build.clean", UMI_STUDIO_COMMAND_BUILD_CLEAN},
        {"run.start", UMI_STUDIO_COMMAND_BUILD_RUN},
        {"build.install", UMI_STUDIO_COMMAND_BUILD_INSTALL},
        {"build.package", UMI_STUDIO_COMMAND_BUILD_PACKAGE},
        {"build.rebuild", UMI_STUDIO_COMMAND_BUILD_REBUILD},
        {"build.deploy", UMI_STUDIO_COMMAND_BUILD_DEPLOY},
        {"debug.start", UMI_STUDIO_COMMAND_DEBUG_START}
    };
    for (size_t index = 0U; index < sizeof commands / sizeof commands[0]; ++index) {
        for (unsigned route = 0U; route < 3U; ++route) {
            /* The wrapper rechecks its owner after publishing denied-command
             * feedback. A closed host reports cancellation while preserving
             * the original refusal text and unchanged source/build state. */
#if 0
            CHECK(umi_studio_gtk_workbench_execute_command(*slot,
                route == 0U ? commands[index].menu : commands[index].canonical,
                route == 2U ? "background" : NULL) == expected);
#endif
            UmiStatus completed = umi_studio_gtk_workbench_execute_command(*slot,
                route == 0U ? commands[index].menu : commands[index].canonical,
                route == 2U ? "background" : NULL);
            UmiStatus expectedCompletion = closeOnFeedback ? UMI_STATUS_CANCELLED : expected;
            if (completed != expectedCompletion)
                fprintf(stderr, "Build admission completion: expected %s, received %s\n",
                    umi_status_text(expectedCompletion), umi_status_text(completed));
            CHECK(completed == expectedCompletion);
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(notice)), reason) != NULL);
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
            size_t bytes = 0U;
            CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &bytes) == UMI_STATUS_OK);
            CHECK(bytes == strlen(draft) && strcmp(text, draft) == 0);
            UmiUiDocumentViewModelFreeText(text); text = NULL;
            CHECK(g_file_get_contents(source, &disk, NULL, NULL) && strcmp(disk, "int saved_change = 1;\n") == 0);
            g_clear_pointer(&disk, g_free);
            CHECK(!UmiStudioBuildBusy(build) && !UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)));
            CHECK(umi_studio_build_service_snapshot(build, &after) == UMI_STATUS_OK);
            CHECK(before.history_count == after.history_count && before.next_operation_id == after.next_operation_id);
            CHECK(before.graph.revision == after.graph.revision && before.graph.node_count == after.graph.node_count);
            CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(build)));
            if (closeOnFeedback) { CHECK(*slot == NULL); goto cleanup; }
        }
    }
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(notice, closeHandler);
    g_clear_object(&notice);
    g_free(source); g_free(other); g_free(disk);
    UmiUiDocumentViewModelFreeText(text);
    return failed;
}

/* Hidden layouts must not turn successful bookmark and tab navigation into an
 * invisible model change. Keep a third draft dirty throughout the operation. */
static int ExerciseVisibleNavigation(UmiStudioGtkWorkbench **slot,
    UmiStudioUi *ui, const char *root, const char *mode)
{
    int failed = 0;
    char *paths[3] = {NULL, NULL, NULL};
    char ids[3][UMI_UI_ID_CAPACITY] = {{0}};
    char *text = NULL, *disk = NULL;
    size_t bytes = 0U;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    gulong closeHandler = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    int bookmark = strstr(mode, "bookmark") != NULL;
    int previous = strstr(mode, "previous") != NULL;
    int empty = strcmp(mode, "navigation-bookmark-empty") == 0;
    int closeOnFocus = strcmp(mode, "navigation-bookmark-focus-close") == 0;
    UmiDocumentWorkingCopySnapshot active;
    for (unsigned index = 0U; index < 3U; ++index) {
        char name[32];
        (void)snprintf(name, sizeof name, "navigation-%u.c", index);
        paths[index] = g_build_filename(root, name, NULL);
        CHECK(g_file_set_contents(paths[index], "first line\nbookmark line\n", -1, NULL));
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", paths[index]) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        g_strlcpy(ids[index], active.view_id, sizeof ids[index]);
        GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
        if (bookmark && !empty && index < 2U) {
            CHECK(umi_studio_gtk_workbench_execute_command(*slot, "umicom.document.go-to-line", "2:1") == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_execute_command(*slot, "editor.bookmark.toggle", NULL) == UMI_STATUS_OK);
        }
        if (index == 2U)
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), "unsaved third draft\n", -1);
    }
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "navigation-hidden", "Navigation") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.navigation-hidden", "Navigation") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    if (closeOnFocus)
        closeHandler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
    const char *command = bookmark
        ? (previous ? "editor.bookmark.previous" : "editor.bookmark.next")
        : (previous ? "navigate.document.previous" : "navigate.document.next");
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot, command, NULL);
    if (closeOnFocus) CHECK(*slot == NULL && status == UMI_STATUS_CANCELLED);
    else if (empty) {
        /* The public host rejects an unavailable bookmark action before
         * dispatch. Retain the lower-level navigator expectation for review;
         * the unchanged active document below remains the safety assertion. */
#if 0
        CHECK(status == UMI_STATUS_NOT_FOUND);
#endif
        CHECK(status == UMI_STATUS_PERMISSION_DENIED);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, paths[2]));
    } else {
        CHECK(status == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, paths[previous ? 1U : 0U]));
        GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
        GtkWidget *focus = gtk_window_get_focus(window);
        CHECK(focus == editor || (focus != NULL && gtk_widget_is_ancestor(focus, editor)));
        if (bookmark) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
            GtkTextIter caret;
            gtk_text_buffer_get_iter_at_mark(buffer, &caret, gtk_text_buffer_get_insert(buffer));
            CHECK(gtk_text_iter_get_line(&caret) == 1);
        }
    }
    UmiUiDocumentViewSnapshot third;
    CHECK(umi_ui_document_view_model_find(views, ids[2], &third) == UMI_STATUS_OK && third.dirty);
    CHECK(UmiUiDocumentViewModelCopyText(views, ids[2], &text, &bytes) == UMI_STATUS_OK);
    CHECK(bytes == strlen("unsaved third draft\n") && strcmp(text, "unsaved third draft\n") == 0);
    CHECK(g_file_get_contents(paths[2], &disk, NULL, NULL) && strcmp(disk, "first line\nbookmark line\n") == 0);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(window, closeHandler);
    g_clear_object(&window);
    for (unsigned index = 0U; index < 3U; ++index) g_free(paths[index]);
    UmiUiDocumentViewModelFreeText(text);
    g_free(disk);
    return failed;
}

/* Go to Line must remain useful when the editor pane is hidden, including its
 * native prompt. Cancelling or rejecting input keeps the captured caret. */
static int ExerciseSourceLocation(UmiStudioGtkWorkbench **slot,
    UmiStudioUi *ui, const char *root, const char *mode)
{
    int failed = 0;
    char *source = g_build_filename(root, "location source.c", NULL);
    char *text = NULL, *disk = NULL;
    size_t bytes = 0U;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkWindow *form = NULL;
    gulong closeHandler = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    const int prompt = strstr(mode, "prompt") != NULL;
    const int cancel = strcmp(mode, "location-prompt-cancel") == 0;
    const int invalid = strcmp(mode, "location-invalid") == 0;
    const int closeOnFocus = strcmp(mode, "location-focus-close") == 0;
    CHECK(g_file_set_contents(source, "first\nsecond\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    UmiUiDocumentViewSnapshot before, after;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(Reachable(editor, GTK_WIDGET(window)));
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), "first\nsecond\nthird\n", -1);
    CHECK(umi_ui_document_view_model_find(views, active.view_id, &before) == UMI_STATUS_OK && before.dirty);
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "location-hidden", "Location") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.location-hidden", "Location") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    if (closeOnFocus)
        closeHandler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
    UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot, "umicom.document.go-to-line",
        prompt ? NULL : invalid ? "-1" : "2:3");
    if (closeOnFocus) {
        CHECK(status == UMI_STATUS_CANCELLED && *slot == NULL);
        goto check_draft;
    }
    CHECK(status == (invalid ? UMI_STATUS_INVALID_ARGUMENT : UMI_STATUS_OK));
    if (prompt) {
        form = FindProjectForm(window, "document.location.input");
        CHECK(form != NULL);
        GtkWidget *input = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "document.location.input");
        GtkWidget *action = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form),
            cancel ? "document.location.cancel" : "document.location.go");
        CHECK(GTK_IS_EDITABLE(input) && GTK_IS_BUTTON(action));
        gtk_editable_set_text(GTK_EDITABLE(input), "2:3");
        g_signal_emit_by_name(action, "clicked");
    }
    CHECK(umi_ui_document_view_model_find(views, active.view_id, &after) == UMI_STATUS_OK);
    if (invalid || cancel) CHECK(after.cursor_offset == before.cursor_offset && after.selection_length == before.selection_length);
    else CHECK(after.cursor_offset == 8U && after.selection_length == 0U);
    if (!invalid) {
        editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
    }
check_draft:
    CHECK(umi_ui_document_view_model_find(views, active.view_id, &after) == UMI_STATUS_OK && after.dirty);
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &bytes) == UMI_STATUS_OK);
    CHECK(strcmp(text, "first\nsecond\nthird\n") == 0);
    CHECK(g_file_get_contents(source, &disk, NULL, NULL) && strcmp(disk, "first\nsecond\n") == 0);
cleanup:
    if (closeHandler != 0U) g_signal_handler_disconnect(window, closeHandler);
    if (form != NULL) { gtk_window_destroy(form); g_object_unref(form); }
    g_clear_object(&window);
    UmiUiDocumentViewModelFreeText(text);
    g_free(disk); g_free(source);
    return failed;
}

/* These routes exercise the real Studio toolbar and native command boundary.
 * The shared service owns the comparison; application tests verify visibility,
 * actual editor buffers, saved bytes and the retained native host boundary. */
static int ExerciseReviewedReload(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *root, const char *mode)
{
    int failed = 0;
    char *source = g_build_filename(root, "reload source.c", NULL), *text = NULL, *disk = NULL;
    size_t length = 0U;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot)), *form = NULL;
    GtkWidget *notice = NULL;
    gulong handler = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    const int cancel = strcmp(mode, "reload-cancel") == 0;
    const int stale = strcmp(mode, "reload-disk-changed") == 0;
    const int focusClose = strcmp(mode, "reload-focus-close") == 0;
    const int external = strncmp(mode, "external-", 9U) == 0;
    const int feedbackClose = strcmp(mode, "external-feedback-close") == 0;
    const int missing = strcmp(mode, "external-missing") == 0;
    const char *expected = "unsaved draft\n", *diskExpected = "saved elsewhere\n";
    CHECK(g_file_set_contents(source, "original\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(Reachable(editor, GTK_WIDGET(window)));
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)), expected, -1);
    CHECK(g_file_set_contents(source, diskExpected, -1, NULL));
    if (external) {
        if (missing) { CHECK(g_remove(source) == 0); diskExpected = NULL; }
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.editor.check-external");
        CHECK(GTK_IS_BUTTON(button));
        if (feedbackClose) {
            GtkWidget *found = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.notification");
            CHECK(GTK_IS_LABEL(found)); notice = g_object_ref(found);
            handler = g_signal_connect(notice, "notify::label", G_CALLBACK(CloseWhenFeedbackDisplayed), slot);
        }
        g_signal_emit_by_name(button, "clicked");
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(active.conflict == (missing ? UMI_DOCUMENT_CONFLICT_DELETED_EXTERNALLY : UMI_DOCUMENT_CONFLICT_EXTERNAL_CHANGE));
        if (feedbackClose) CHECK(*slot == NULL);
        goto verify;
    }
    if (strcmp(mode, "reload-toolbar") == 0) {
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.editor.reload");
        CHECK(GTK_IS_BUTTON(button)); g_signal_emit_by_name(button, "clicked");
    } else {
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "reload-hidden", "Reload") == UMI_STATUS_OK);
#endif
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.reload-hidden", "Reload") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
        if (focusClose) handler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
        UmiStatus status;
        if (strcmp(mode, "reload-menu") == 0) {
            GtkWidget *action = FindAction(GTK_WIDGET(window), "file.reload");
            CHECK(GTK_IS_BUTTON(action) && gtk_widget_get_sensitive(action));
            g_signal_emit_by_name(action, "clicked"); status = UMI_STATUS_OK;
        } else status = umi_studio_gtk_workbench_execute_command(*slot,
            strcmp(mode, "reload-canonical") == 0 ? "umicom.document.revert" : "file.reload", NULL);
        if (focusClose) { CHECK(status == UMI_STATUS_CANCELLED && *slot == NULL); goto verify; }
        CHECK(status == UMI_STATUS_OK);
    }
    form = FindProjectForm(window, "document.reload.apply"); CHECK(form != NULL);
    editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(editor != NULL && gtk_widget_get_visible(editor));
    GtkWidget *approve = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form), "document.reload.approve");
    GtkWidget *action = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(form),
        cancel ? "document.reload.cancel" : "document.reload.apply");
    CHECK(GTK_IS_CHECK_BUTTON(approve) && GTK_IS_BUTTON(action));
    if (!cancel) gtk_check_button_set_active(GTK_CHECK_BUTTON(approve), TRUE);
    if (stale) { diskExpected = "new disk change\n"; CHECK(g_file_set_contents(source, diskExpected, -1, NULL)); }
    g_signal_emit_by_name(action, "clicked");
    if (!cancel && !stale) expected = "saved elsewhere\n";
    if (strcmp(mode, "reload-undo") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "edit.undo", NULL) == UMI_STATUS_OK);
        expected = "unsaved draft\n";
    }
verify:
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &text, &length) == UMI_STATUS_OK);
    CHECK(length == strlen(expected) && strcmp(text, expected) == 0);
    if (diskExpected != NULL) CHECK(g_file_get_contents(source, &disk, NULL, NULL) && strcmp(disk, diskExpected) == 0);
    else CHECK(!g_file_test(source, G_FILE_TEST_EXISTS));
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(feedbackClose ? G_OBJECT(notice) : G_OBJECT(window), handler);
    if (form != NULL) { gtk_window_destroy(form); g_object_unref(form); }
    g_clear_object(&notice); g_clear_object(&window);
    UmiUiDocumentViewModelFreeText(text); g_free(disk); g_free(source);
    return failed;
}

/* Source keyboard input and command discovery share Framework's draft edit
 * transaction. Verify the native buffer as well as the model after revealing a
 * hidden editor, and revoke the operation if focus presentation retires Studio. */
static int ExerciseSourceLineCommands(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *mode)
{
    int failed = 0;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkEventController *keys = NULL;
    GtkWidget *editor = NULL;
    gulong handler = 0U;
    char *modelText = NULL, *nativeText = NULL;
    size_t bytes = 0U;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    const int keyboard = strcmp(mode, "line-keyboard") == 0;
    const int invalid = strcmp(mode, "line-argument") == 0;
    const int focusClose = strcmp(mode, "line-focus-close") == 0;
    const char *source = "a\nbb\nccc", *expected = "a\nbb\nbb\nccc";
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "file.new", NULL) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    editor = FindEditor(GTK_WIDGET(window), active.view_id);
    CHECK(Reachable(editor, GTK_WIDGET(window)));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    GtkTextIter caret;
    gtk_text_buffer_set_text(buffer, source, -1);
    gtk_text_buffer_get_iter_at_offset(buffer, &caret, 2);
    gtk_text_buffer_place_cursor(buffer, &caret);
    if (strcmp(mode, "line-toolbar") == 0) {
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.editor.line-duplicate");
        CHECK(GTK_IS_BUTTON(button)); g_signal_emit_by_name(button, "clicked");
    } else if (keyboard) {
        keys = FirstUseKeyboard(editor, "umicom-source-editor-commands"); CHECK(keys != NULL);
        CHECK(FirstUseKey(keys, GDK_KEY_d, GDK_CONTROL_MASK | GDK_SHIFT_MASK));
    } else {
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "line-hidden", "Line editing") == UMI_STATUS_OK);
#endif
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.line-hidden", "Line editing") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
        if (focusClose) handler = g_signal_connect(window, "notify::focus-widget", G_CALLBACK(CloseWhenEditorFocused), slot);
        UmiStatus status = umi_studio_gtk_workbench_execute_command(*slot,
            "umicom.document.line-edit.duplicate-line", invalid ? "unexpected" : NULL);
        CHECK(status == (invalid ? UMI_STATUS_INVALID_ARGUMENT : focusClose ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
        if (invalid || focusClose) expected = source;
        if (focusClose) CHECK(*slot == NULL);
    }
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &modelText, &bytes) == UMI_STATUS_OK);
    CHECK(bytes == strlen(expected) && strcmp(modelText, expected) == 0);
    if (!invalid && !focusClose) {
        editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
        GtkTextIter begin, end;
        gtk_text_buffer_get_bounds(buffer, &begin, &end);
        nativeText = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
        CHECK(strcmp(nativeText, expected) == 0);
        CHECK(umi_studio_gtk_workbench_execute_command(*slot, "edit.undo", NULL) == UMI_STATUS_OK);
        UmiUiDocumentViewModelFreeText(modelText); modelText = NULL;
        CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &modelText, &bytes) == UMI_STATUS_OK);
        CHECK(bytes == strlen(source) && strcmp(modelText, source) == 0);
    }
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(window, handler);
    UmiUiDocumentViewModelFreeText(modelText); g_free(nativeText);
    g_clear_object(&keys); g_clear_object(&window);
    return failed;
}

/* Evidence callbacks deliberately exercise the same synchronous notification
 * boundaries as a user closing or selecting another test during a refresh. */
static GtkWidget *EvidenceTestSelect(GtkWidget *root, const char *itemId)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-test-id");
    if (GTK_IS_BUTTON(root) && g_strcmp0(id, itemId) == 0 &&
        g_strcmp0(gtk_button_get_label(GTK_BUTTON(root)), "Select") == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = EvidenceTestSelect(child, itemId); if (found != NULL) return found;
    }
    return NULL;
}
typedef struct EvidenceCallbackProbe {
    UmiStudioGtkWorkbench **slot;
    unsigned calls;
    int reenter, reselect, close;
    UmiStatus nested;
} EvidenceCallbackProbe;
static void EvidenceObserved(gpointer object, gpointer data)
{
    EvidenceCallbackProbe *probe = data;
    (void)object;
    if (probe->calls++ != 0U || *probe->slot == NULL) return;
    if (probe->close) {
        UmiStudioGtkWorkbench *owner = *probe->slot; *probe->slot = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (probe->reenter) probe->nested = umi_studio_gtk_workbench_workspace_synchronise(*probe->slot);
    else if (probe->reselect) {
        GtkWidget *select = EvidenceTestSelect(GTK_WIDGET(umi_studio_gtk_workbench_window(*probe->slot)), "evidence.a.more");
        if (select != NULL) g_signal_emit_by_name(select, "clicked");
    }
}
static void EvidenceNotified(GObject *object, GParamSpec *spec, gpointer data)
{ (void)spec; EvidenceObserved(object, data); }
static int ExerciseEvidenceRefresh(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *root, const char *mode)
{
    int failed = 0;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GObject *observed = NULL;
    gulong handler = 0U;
    char *text = NULL, *source = g_build_filename(root, "evidence source.c", NULL);
    UmiTestPlatformService *platform = umi_studio_test_service_platform(umi_studio_services_tests(umi_studio_ui_services(ui)));
    CHECK(platform != NULL && g_file_set_contents(source, "first\nsecond\n", -1, NULL));
    UmiTestPlatformItemRegistry *items = umi_test_platform_service_item(platform);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    UmiTestPlatformOutputRegistry *outputs = umi_test_platform_service_output(platform);
    const char *ids[] = {"evidence.a", "evidence.a.more"};
    for (size_t i = 0U; i < 2U; ++i) {
        UmiTestPlatformItemSnapshot item = {0};
        g_strlcpy(item.id, ids[i], sizeof item.id); g_strlcpy(item.name, ids[i], sizeof item.name);
        g_strlcpy(item.framework, "ctest", sizeof item.framework); g_strlcpy(item.kind, "test", sizeof item.kind);
        g_strlcpy(item.working_directory, root, sizeof item.working_directory);
        item.enabled = 1; item.discovered = 1;
        CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
        UmiTestPlatformResultSnapshot row = {0};
        g_strlcpy(row.id, ids[i], sizeof row.id); g_strlcpy(row.item_id, ids[i], sizeof row.item_id);
        g_strlcpy(row.session_id, "native-evidence-run", sizeof row.session_id);
        g_strlcpy(row.message, i == 0U ? "selected original" : "other test only", sizeof row.message);
        snprintf(row.failure_details, sizeof row.failure_details, "%s:2:1: original", source);
        row.outcome = UMI_TEST_PLATFORM_OUTCOME_FAILED; row.sequence = 1U; row.exit_code = 1;
        CHECK(umi_test_platform_result_registry_upsert(results, &row) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(*slot, UMI_STUDIO_SURFACE_TEST_EXPLORER, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
    GtkWidget *select = EvidenceTestSelect(GTK_WIDGET(window), ids[0]); CHECK(GTK_IS_BUTTON(select));
    g_signal_emit_by_name(select, "clicked");
    GtkWidget *selected = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.tests.selected-evidence");
    CHECK(GTK_IS_TEXT_VIEW(selected) && !gtk_text_view_get_editable(GTK_TEXT_VIEW(selected)));
    EvidenceCallbackProbe probe = {slot, 0U, strcmp(mode, "evidence-reenter") == 0,
        strcmp(mode, "evidence-reselect") == 0, strstr(mode, "-close") != NULL, UMI_STATUS_INTERNAL_ERROR};
    const int preserve = strcmp(mode, "evidence-selection") == 0;
    const int raw = strcmp(mode, "evidence-invalid-bytes") == 0;
    const char *tag = "studio.tests.selected-evidence";
    if (strcmp(mode, "evidence-results-close") == 0) tag = "studio.tests.results-evidence";
    if (strcmp(mode, "evidence-coverage-close") == 0) tag = "studio.tests.coverage-evidence";
    if (strcmp(mode, "evidence-output-close") == 0) tag = "studio.tests.output-evidence";
    if (strcmp(mode, "evidence-locations-close") == 0 || probe.reselect) tag = "studio.tests.source-location-summary";
    if (strcmp(mode, "evidence-row-close") == 0) tag = "studio.tests.source-location.0";
    GtkWidget *target = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), tag); CHECK(target != NULL);
    if (strcmp(mode, "evidence-clipboard-close") == 0) {
        observed = g_object_ref(G_OBJECT(gtk_widget_get_clipboard(selected)));
        handler = g_signal_connect(observed, "changed", G_CALLBACK(EvidenceObserved), &probe);
        GtkWidget *copy = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.tests.copy-evidence-csv");
        CHECK(GTK_IS_BUTTON(copy)); g_signal_emit_by_name(copy, "clicked");
        CHECK(probe.calls != 0U && *slot == NULL); goto cleanup;
    }
    if (GTK_IS_TEXT_VIEW(target)) {
        observed = g_object_ref(G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(target))));
        handler = g_signal_connect(observed, "changed", G_CALLBACK(EvidenceObserved), &probe);
    } else {
        observed = g_object_ref(G_OBJECT(target));
        handler = g_signal_connect(observed, GTK_IS_LABEL(target) ? "notify::label" : "notify::parent", G_CALLBACK(EvidenceNotified), &probe);
    }
    if (preserve) {
        GtkTextBuffer *buffer = GTK_TEXT_BUFFER(observed); GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_offset(buffer, &start, 2); gtk_text_buffer_get_iter_at_offset(buffer, &end, 8);
        gtk_text_buffer_select_range(buffer, &start, &end);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
        CHECK(probe.calls == 0U && gtk_text_buffer_get_selection_bounds(buffer, &start, &end));
        CHECK(gtk_text_iter_get_offset(&start) == 2 && gtk_text_iter_get_offset(&end) == 8); goto cleanup;
    }
    UmiTestPlatformResultSnapshot changed;
    CHECK(umi_test_platform_result_registry_find(results, ids[0], &changed) == UMI_STATUS_OK);
    g_strlcpy(changed.message, "selected changed", sizeof changed.message); changed.sequence = 2U;
    snprintf(changed.failure_details, sizeof changed.failure_details, "%s:1:1: changed", source);
    CHECK(umi_test_platform_result_registry_upsert(results, &changed) == UMI_STATUS_OK);
    g_strlcpy(changed.id, "additional.result", sizeof changed.id); changed.sequence = 3U;
    CHECK(umi_test_platform_result_registry_upsert(results, &changed) == UMI_STATUS_OK);
    UmiTestPlatformOutputSnapshot output = {0};
    strcpy(output.id, "output.a"); strcpy(output.item_id, ids[0]); strcpy(output.stream, "stderr");
    strcpy(output.text, "new output"); if (raw) { output.text[0] = (char)0xff; output.text[1] = 'x'; output.text[2] = '\0'; }
    CHECK(umi_test_platform_output_registry_upsert(outputs, &output) == UMI_STATUS_OK);
    UmiTestPlatformCoverageSnapshot coverage = {0};
    strcpy(coverage.id, "coverage.a"); g_strlcpy(coverage.uri, source, sizeof coverage.uri);
    coverage.lines_covered = 1U; coverage.lines_total = 2U;
    CHECK(umi_test_platform_coverage_registry_upsert(umi_test_platform_service_coverage(platform), &coverage) == UMI_STATUS_OK);
    UmiStatus status = umi_studio_gtk_workbench_workspace_synchronise(*slot);
    if (probe.close) { CHECK(status == UMI_STATUS_CANCELLED && probe.calls != 0U && *slot == NULL); goto cleanup; }
    CHECK(status == UMI_STATUS_OK && *slot != NULL);
    if (probe.reenter) CHECK(probe.calls != 0U && probe.nested == UMI_STATUS_OK);
    if (probe.reselect) {
        CHECK(probe.calls != 0U);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
    }
    selected = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.tests.selected-evidence");
    CHECK(GTK_IS_TEXT_VIEW(selected));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(selected));
    GtkTextIter begin, end; gtk_text_buffer_get_bounds(buffer, &begin, &end);
    text = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
    CHECK(g_utf8_validate(text, -1, NULL));
    CHECK(strstr(text, probe.reselect ? "other test only" : "selected changed") != NULL);
    CHECK(strstr(text, probe.reselect ? "selected changed" : "other test only") == NULL);
    if (raw) CHECK(strstr(text, "\xef\xbf\xbd" "x") != NULL);
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(observed, handler);
    g_clear_object(&observed); g_clear_object(&window); g_free(text); g_free(source);
    return failed;
}

/* Exercise the actual indexed Explorer list; no replacement row renderer is
 * supplied by the test. Observers change host state at native publication. */
static GtkWidget *ExplorerFileButton(GtkWidget *root, const char *path)
{
    const char *candidate = g_object_get_data(G_OBJECT(root), "umicom-file-path");
    if (GTK_IS_BUTTON(root) && candidate != NULL && umi_path_equal(candidate, path)) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = ExplorerFileButton(child, path);
        if (found != NULL) return found;
    }
    return NULL;
}
typedef struct ExplorerRefreshProbe {
    UmiStudioGtkWorkbench **slot;
    UmiStudioServices *services;
    GtkWidget *filter;
    const char *mode;
    unsigned calls;
    UmiStatus nested;
} ExplorerRefreshProbe;
static void ExplorerRefreshObserved(GObject *object, GParamSpec *property, gpointer data)
{
    (void)object; (void)property;
    ExplorerRefreshProbe *probe = data;
    if (probe->calls++ != 0U) return;
    if (strstr(probe->mode, "-close") != NULL) {
        UmiStudioGtkWorkbench *owner = *probe->slot; *probe->slot = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (strcmp(probe->mode, "explorer-filter-during-refresh") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(probe->filter), "beta");
        g_signal_emit_by_name(probe->filter, "search-changed");
    } else if (strcmp(probe->mode, "explorer-workspace-during-refresh") == 0) {
        probe->nested = umi_studio_workspace_close(probe->services);
    } else if (strcmp(probe->mode, "explorer-reenter") == 0) {
        probe->nested = umi_studio_gtk_workbench_workspace_synchronise(*probe->slot);
    }
}
static int ExerciseExplorerRefresh(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *fixture, const char *mode)
{
    int failed = 0;
    UmiStudioServices *services = umi_studio_ui_services(ui);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(*slot));
    GtkWidget *oldButton = NULL, *filter = NULL;
    GObject *observed = NULL;
    gulong handler = 0U;
    char *root = g_build_filename(fixture, "Explorer project", NULL);
    char *alpha = g_build_filename(root, "alpha.c", NULL), *beta = g_build_filename(root, "beta.c", NULL);
    char *gamma = g_build_filename(root, "gamma.c", NULL);
    ExplorerRefreshProbe probe = {slot, services, NULL, mode, 0U, UMI_STATUS_OK};
    CHECK(g_mkdir_with_parents(root, 0700) == 0);
    CHECK(g_file_set_contents(alpha, "alpha source\n", -1, NULL) && g_file_set_contents(beta, "beta source\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(*slot, "workspace.open-folder", root) == UMI_STATUS_OK);
    UmiFileIndex *index = umi_studio_services_file_index(services);
    CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "explorer-only", "Explorer") == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(*slot, "umicom.studio.layout.explorer-only", "Explorer") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(*slot) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(*slot, UMI_STUDIO_SURFACE_EXPLORER, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
    GtkWidget *list = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-list");
    CHECK(GTK_IS_LIST_BOX(list));
    GtkWidget *old = ExplorerFileButton(list, alpha); CHECK(GTK_IS_BUTTON(old)); oldButton = g_object_ref(old);
    GtkWidget *filterCandidate = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-filter");
    CHECK(GTK_IS_SEARCH_ENTRY(filterCandidate)); filter = g_object_ref(filterCandidate); probe.filter = filter;
    const int unchanged = strcmp(mode, "explorer-unchanged") == 0;
    const int sameRoot = strcmp(mode, "explorer-same-root-reopen") == 0;
    const int close = strstr(mode, "-close") != NULL;
    GtkWidget *target = gtk_widget_get_ancestor(oldButton, GTK_TYPE_LIST_BOX_ROW);
    if (strcmp(mode, "explorer-summary-close") == 0)
        target = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-summary");
    if (strcmp(mode, "explorer-status-close") == 0) {
        target = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-refresh-status");
        CHECK(GTK_IS_LABEL(target)); gtk_label_set_text(GTK_LABEL(target), "Awaiting next refresh");
    }
    CHECK(target != NULL); observed = g_object_ref(G_OBJECT(target));
    handler = g_signal_connect(observed, GTK_IS_LABEL(target) ? "notify::label" : "notify::parent", G_CALLBACK(ExplorerRefreshObserved), &probe);
    if (sameRoot) {
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    } else if (!unchanged) {
        CHECK(g_file_set_contents(gamma, "gamma source\n", -1, NULL));
        CHECK(umi_file_index_update(index, gamma) == UMI_STATUS_OK);
    }
    UmiStatus status = umi_studio_gtk_workbench_workspace_synchronise(*slot);
    if (close) { CHECK(status == UMI_STATUS_CANCELLED && *slot == NULL && probe.calls != 0U); goto cleanup; }
    CHECK(status == UMI_STATUS_OK && *slot != NULL);
    if (unchanged) {
        CHECK(probe.calls == 0U && ExplorerFileButton(list, alpha) == oldButton); goto cleanup;
    }
    CHECK(probe.calls != 0U && probe.nested == UMI_STATUS_OK);
    /* A retained button from a removed or retired page must not open a source. */
    CHECK(umi_document_coordinator_count(documents) == 0U);
    g_signal_emit_by_name(oldButton, "clicked");
    CHECK(umi_document_coordinator_count(documents) == 0U);
    if (strcmp(mode, "explorer-workspace-during-refresh") == 0) {
        UmiStudioWorkspaceSnapshot workspace;
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.open);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
        CHECK(ExplorerFileButton(list, alpha) == NULL && ExplorerFileButton(list, beta) == NULL &&
            ExplorerFileButton(list, gamma) == NULL);
        g_signal_emit_by_name(oldButton, "clicked");
        CHECK(umi_document_coordinator_count(documents) == 0U);
        goto cleanup;
    }
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(*slot) == UMI_STATUS_OK);
    if (strcmp(mode, "explorer-filter-during-refresh") == 0) {
        CHECK(ExplorerFileButton(list, alpha) == NULL && ExplorerFileButton(list, gamma) == NULL);
        CHECK(ExplorerFileButton(list, beta) != NULL);
    } else {
        GtkWidget *fresh = ExplorerFileButton(list, sameRoot ? alpha : gamma);
        CHECK(GTK_IS_BUTTON(fresh)); g_signal_emit_by_name(fresh, "clicked");
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, sameRoot ? alpha : gamma));
        CHECK(Reachable(FindEditor(GTK_WIDGET(window), active.view_id), GTK_WIDGET(window)));
    }
cleanup:
    if (handler != 0U) g_signal_handler_disconnect(observed, handler);
    g_clear_object(&observed); g_clear_object(&oldButton); g_clear_object(&filter); g_clear_object(&window);
    g_free(root); g_free(alpha); g_free(beta); g_free(gamma);
    return failed;
}

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *name = argv[1];
    const char *cases[] = {"new-file", "hidden-new", "open-edit-save", "hidden-open",
        "unicode-path", "missing-file", "reopen-dirty", "project-folder", "tab-switch", "build-guidance", "close-during-open", "build-unsaved", "real-build-run", "real-build-repair", "repository-switch", "close-during-feedback", "debug-unsaved", "debug-canonical-unsaved", "project-create-hidden", "project-create-existing", "close-during-create", "workspace-refresh-edit", "project-prompt-retired", "project-prompt-retained-window", "wizard-edit-settings", "wizard-hidden-editor", "wizard-close-during-adoption", "project-open-failure-draft", "settings-present-complete", "settings-present-close", "settings-present-replace", "problem-next-stale", "problem-previous-stale", "keyboard-new", "keyboard-save", "keyboard-modifiers", "keyboard-retired", "search-keyboard", "search-options", "search-retired", "search-feedback-close", "search-location", "run-untrusted", "run-unconfigured", "run-missing", "run-working-folder", "run-feedback-close", "replace-keyboard", "replace-options", "replace-retired", "replace-feedback-close", "discovery-relative", "discovery-absolute", "discovery-run-relative", "discovery-feedback-close", "discovery-run-feedback-close", "close-dialog-cancel", "close-dialog-confirm", "close-dialog-retained-dialog", "close-dialog-retained-button", "close-dialog-retired-owner", "close-dialog-reparented-cancel", "batch-save", "batch-save-start-close", "batch-save-completion-close", "batch-save-cancel", "batch-close", "batch-close-start-close", "batch-close-completion-close", "batch-close-cancel", "history-reopen-hidden", "history-reopen-missing", "history-reopen-focus-close", "history-back-hidden", "history-forward-hidden", "button-new", "button-open", "button-retired-command", "button-retired-argument", "button-retired-palette", "button-retired-layout", "build-denied-closed", "build-denied-untrusted", "build-denied-other-project", "build-denied-feedback-close", "navigation-document-next", "navigation-document-previous", "navigation-bookmark-next", "navigation-bookmark-previous", "navigation-bookmark-empty", "navigation-bookmark-focus-close", "location-direct-hidden", "location-invalid", "location-prompt-hidden", "location-prompt-cancel", "location-focus-close", "reload-menu", "reload-toolbar", "reload-canonical", "reload-cancel", "reload-disk-changed", "reload-focus-close", "reload-undo", "external-changed", "external-missing", "external-feedback-close", "form-project-open", "form-application-open", "form-project-create-close", "form-application-create-close", "form-project-create-reenter", "form-application-create-reenter", "form-project-present-close", "form-application-present-close", "form-project-present-reenter", "form-application-present-reenter", "form-project-replace-close", "form-application-replace-close", "line-hidden", "line-keyboard", "line-argument", "line-focus-close", "line-toolbar", "evidence-selected-close", "evidence-results-close", "evidence-coverage-close", "evidence-output-close", "evidence-locations-close", "evidence-row-close", "evidence-selection", "evidence-invalid-bytes", "evidence-clipboard-close", "evidence-reenter", "evidence-reselect", "explorer-row-close", "explorer-summary-close", "explorer-status-close", "explorer-filter-during-refresh", "explorer-workspace-during-refresh", "explorer-reenter", "explorer-unchanged", "explorer-same-root-reopen", "explorer-retained-old-row"};
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
    /* These workflows deliberately exercise an empty editor. Close only the
     * known, pristine sample through its normal non-discard close operation;
     * never change Studio's real startup or hide an unexpected user draft.
     * Startup ownership is covered separately by test_editor_settings.c. */
    UmiDocumentWorkingCopySnapshot welcome;
    CHECK(umi_document_coordinator_count(documents) == 1U);
    CHECK(umi_document_coordinator_active_snapshot(documents, &welcome) == UMI_STATUS_OK);
    CHECK(!strcmp(welcome.view_id, "studio.editor.welcome") && !welcome.dirty && !welcome.has_path);
    CHECK(UmiDocumentCoordinatorClose(documents, welcome.document_id, 0) == UMI_STATUS_OK);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.first-use-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(umi_document_coordinator_count(documents) == 0U);
    if (strncmp(name, "button-", 7U) == 0) {
        failed = ExerciseCommandButtons(&workbench, documents, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "history-", 8U) == 0) {
        failed = ExerciseVisibleHistory(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "batch-", 6U) == 0) {
        failed = ExerciseDocumentSequence(&workbench, documents, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "close-dialog-", 13U) == 0) {
        failed = ExerciseCloseConfirmation(&workbench, documents, name);
        goto cleanup;
    }
    if (strncmp(name, "discovery-", 10U) == 0) {
        failed = ExerciseNativeDiscovery(&workbench, services, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "replace-", 8U) == 0) {
        failed = ExerciseReplaceKeyboard(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "explorer-", 9U) == 0) {
        failed = ExerciseExplorerRefresh(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "evidence-", 9U) == 0) {
        failed = ExerciseEvidenceRefresh(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "line-", 5U) == 0) {
        failed = ExerciseSourceLineCommands(&workbench, ui, name);
        goto cleanup;
    }
    if (strncmp(name, "reload-", 7U) == 0 || strncmp(name, "external-", 9U) == 0) {
        failed = ExerciseReviewedReload(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "location-", 9U) == 0) {
        failed = ExerciseSourceLocation(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "navigation-", 11U) == 0) {
        failed = ExerciseVisibleNavigation(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "build-denied-", 13U) == 0) {
        failed = ExerciseBuildDenied(&workbench, services, documents, views, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "run-", 4U) == 0) {
        failed = ExerciseRunFeedback(&workbench, services, documents, views, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "search-", 7U) == 0) {
        failed = ExerciseSearchKeyboard(&workbench, ui, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "keyboard-", 9U) == 0) {
        failed = ExerciseFileKeyboard(&workbench, documents, directory, name);
        goto cleanup;
    }
    if (strcmp(name, "problem-next-stale") == 0 || strcmp(name, "problem-previous-stale") == 0) {
        failed = ExerciseAvailableProblem(workbench, services, documents, directory,
            strcmp(name, "problem-previous-stale") == 0);
        goto cleanup;
    }
    if (strncmp(name, "form-", 5U) == 0) {
        failed = ExerciseProjectPresentation(&workbench, documents, name);
        goto cleanup;
    }
    if (strncmp(name, "settings-present-", 17U) == 0) {
        failed = ExerciseSettingsPresentation(&workbench, services, documents, directory, name);
        goto cleanup;
    }
    if (strncmp(name, "wizard-", 7U) == 0) {
        failed = ExerciseProjectWizard(&workbench, services, documents, directory, name);
        goto cleanup;
    }
    if (strcmp(name, "real-build-run") == 0 || strcmp(name, "real-build-repair") == 0) {
        failed = ExerciseNativeBuild(workbench, services, documents, directory,
            strcmp(name, "real-build-repair") == 0);
        goto cleanup;
    }
    if (strcmp(name, "project-create-hidden") == 0 || strcmp(name, "project-create-existing") == 0 ||
        strcmp(name, "close-during-create") == 0 || strcmp(name, "project-prompt-retired") == 0 ||
        strcmp(name, "project-prompt-retained-window") == 0) {
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
/* Verify the displayed root using platform path equality. The previous implementation is retained for review. */
#if 0
        CHECK(ContainsLabel(review, directory));
#endif
        CHECK(RepositoryRootMatches(review, directory));
        source = g_build_filename(directory, "Second project", NULL);
        CHECK(g_mkdir_with_parents(source, 0700) == 0);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", source) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
/* A changed workspace must replace the old displayed root despite path separator normalization. The previous implementation is retained for review. */
#if 0
        CHECK(ContainsLabel(review, source) && !ContainsLabel(review, directory));
#endif
        CHECK(RepositoryRootMatches(review, source) && !RepositoryRootMatches(review, directory));
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
/* Closing a workspace must retire its tagged repository root. The previous implementation is retained for review. */
#if 0
        CHECK(ContainsLabel(review, "Open a project to inspect its repository.") && !ContainsLabel(review, source));
#endif
        CHECK(ContainsLabel(review, "Open a project to inspect its repository.") && !RepositoryRootMatches(review, source));
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
/* Canvas creation requires a qualified Studio layout identity; the earlier short-ID fixture is retained. Retained for engineering review. */
#if 0
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "first-use-empty", "Empty layout") == UMI_STATUS_OK);
#endif
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "umicom.studio.layout.first-use-empty", "Empty layout") == UMI_STATUS_OK);
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
    if (strcmp(name, "project-open-failure-draft") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        char next[UMI_PATH_CAPACITY], child[UMI_PATH_CAPACITY], marker[UMI_PATH_CAPACITY];
        CHECK(umi_fs_join(next, sizeof next, directory, "too-many-projects") == UMI_STATUS_OK);
        CHECK(umi_fs_make_directories(next) == UMI_STATUS_OK);
        for (size_t item = 0U; item <= UMI_WORKSPACE_PROJECT_MAX; ++item) {
            char childName[40];
            (void)snprintf(childName, sizeof childName, "project-%zu", item);
            CHECK(umi_fs_join(child, sizeof child, next, childName) == UMI_STATUS_OK);
            CHECK(umi_fs_make_directories(child) == UMI_STATUS_OK);
            CHECK(umi_fs_join(marker, sizeof marker, child, "CMakeLists.txt") == UMI_STATUS_OK);
            CHECK(umi_fs_write_text(marker, "# bounded discovery fixture\n") == UMI_STATUS_OK);
        }
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", next) == UMI_STATUS_CAPACITY_EXCEEDED);
        UmiStudioWorkspaceSnapshot rejected;
        CHECK(umi_studio_workspace_snapshot(services, &rejected) == UMI_STATUS_OK && !rejected.graph.open);
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
        CHECK(FindEditor(window, active.view_id) == editor && gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer);
        char *kept = NULL;
        size_t keptSize = 0U;
        CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &kept, &keptSize) == UMI_STATUS_OK);
        int unchanged = keptSize == draftLength && memcmp(kept, draft, draftLength) == 0;
        UmiUiDocumentViewModelFreeText(kept);
        CHECK(unchanged);
        GtkWidget *notice = umi_gtk4_automation_find_tagged_widget(window, "studio.notification");
        CHECK(GTK_IS_LABEL(notice) && strstr(gtk_label_get_text(GTK_LABEL(notice)), "open documents are retained") != NULL);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.save", NULL) == UMI_STATUS_OK);
        CHECK(g_file_get_contents(source, &disk, NULL, NULL) && strcmp(disk, draft) == 0);
        CHECK(!UmiStudioBuildBusy(umi_studio_services_build(services)));
        goto cleanup;
    }
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
