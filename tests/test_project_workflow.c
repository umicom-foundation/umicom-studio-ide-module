/*-----------------------------------------------------------------------------
 * Umicom Framework / Studio qualification
 * File: applications/studio/tests/test_project_workflow.c
 * PURPOSE: Exercise real generate/edit/save/build/run/test/install through Studio and Framework.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #test); return EXIT_FAILURE; } } while (0)

#include "umicom/document/edit.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/diagnostics.h"
#include "umicom/diagnostic_ui/navigation.h"
#include "umicom/developer_productivity/text_comparison.h"
#include "umicom/developer_project/new_project.h"
#include "umicom/platform/threading.h"
static int Wait(UmiStudioBuildService *build, UmiStatus expected, UmiBuildResult *result)
{
    for (unsigned attempt = 0U; attempt < 6000U; ++attempt) {
        UmiStatus status;
        while ((status = UmiStudioBuildCollect(build, result)) == UMI_STATUS_OK) {
            printf("%s: %s (exit %d)\n%s\n", umi_build_phase_text(result->phase),
                umi_status_text(result->status), result->exit_code, result->output);
        }
        CHECK(status == UMI_STATUS_NOT_FOUND);
        if (!UmiStudioBuildBusy(build)) {
            UmiBuildProjectSessionSnapshot progress;
            CHECK(UmiStudioBuildProgress(build, &progress) == UMI_STATUS_OK);
            CHECK(progress.status == expected);
            return EXIT_SUCCESS;
        }
        umi_thread_sleep_ms(10U);
    }
    fputs("Build timed out.\n", stderr);
    return EXIT_FAILURE;
}
static int Edit(UmiUiWorkbench *workbench, const char *viewId, const char *text)
{
    UmiUiDocumentViewSnapshot *view = calloc(1U, sizeof(*view));
    CHECK(view != NULL);
    CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(workbench), viewId, view) == UMI_STATUS_OK);
    view->dirty = 1;
    UmiStatus status = UmiUiDocumentViewModelUpsertText(
        umi_ui_workbench_documents(workbench), view, text, strlen(text));
    free(view);
    CHECK(status == UMI_STATUS_OK);
    return EXIT_SUCCESS;
}
/* The same workflow also compiles a source file whose meaningful code is
 * beyond the former inline preview limit. No dummy compiler is substituted. */
static int EditSource(UmiUiWorkbench *workbench, const char *viewId,
    const char *tail, int largeSource)
{
    if (!largeSource) return Edit(workbench, viewId, tail);
    const size_t prefix = 131072U;
    char *source = malloc(prefix + strlen(tail) + 1U);
    CHECK(source != NULL);
    memset(source, ' ', prefix);
    for (size_t n = 63U; n < prefix; n += 64U) source[n] = '\n';
    strcpy(source + prefix, tail);
    int result = Edit(workbench, viewId, source);
    free(source);
    return result;
}

int main(int argc, char **argv)
{
    int largeSource = argc == 2 && strcmp(argv[1], "large-source") == 0;
    int diagnosticsFlow = argc == 2 && strcmp(argv[1], "diagnostics") == 0;
    int projectFiles = argc == 2 && strcmp(argv[1], "project-files") == 0;
    int backgroundRefresh = argc == 2 && strcmp(argv[1], "background-refresh") == 0;
    int compareFlow = argc == 2 && strcmp(argv[1], "compare-saved") == 0;
    int reloadFlow = (argc == 2 && strcmp(argv[1], "reload") == 0) || compareFlow;
    int editCommands = argc == 2 && strcmp(argv[1], "edit-commands") == 0;
    int fileSearch = argc == 2 && strcmp(argv[1], "find-in-files") == 0;
    CHECK(argc == 1 || largeSource || diagnosticsFlow || projectFiles || backgroundRefresh || fileSearch || reloadFlow || editCommands);
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServicesOptions options = {0};
    UmiDeveloperProjectService *projects = NULL;
    UmiDeveloperProjectGenerationRequest request;
    UmiDeveloperProjectGeneratorReport report;
    UmiDeveloperProjectModel model;
    UmiBuildProfile profile;
    UmiBuildResult *result = NULL;
    UmiStudioServices *services; UmiStudioBuildService *build;
    UmiUiWorkbench *workbench; UmiDocumentCoordinator *documents; UmiCommandRegistry *commands;
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY];
    char viewId[UMI_UI_ID_CAPACITY], message[512], installed[UMI_PATH_CAPACITY];
    CHECK(umi_fs_current_directory(cwd, sizeof(cwd)) == UMI_STATUS_OK);
    CHECK(umi_fs_join(root, sizeof(root), cwd, "Umicom Notes") == UMI_STATUS_OK);
    CHECK(!umi_fs_exists(root)); /* Never delete a pre-existing developer folder. */
    CHECK(umi_developer_project_service_create(&projects) == UMI_STATUS_OK);
    umi_developer_project_generation_request_init(&request);
    strcpy(request.template_id, "developer.template.c23-console");
    strcpy(request.application_name, "Umicom Notes"); strcpy(request.application_id, "org.umicom.notes");
    strcpy(request.repository_name, "umicom-notes"); strcpy(request.target_name, "umicom_notes");
    strcpy(request.project_root, root);
    CHECK(UmiDeveloperProjectCreateNew(projects, &request, &report, &model, &profile) == UMI_STATUS_OK);
    CHECK(report.files_created >= 6U && report.files_skipped == 0U);
    CHECK(UmiDeveloperProjectCreateNew(projects, &request, &report, &model, &profile) == UMI_STATUS_ALREADY_EXISTS);
    CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    services = umi_studio_bootstrap_services(bootstrap); build = umi_studio_services_build(services);
    workbench = umi_studio_ui_workbench(umi_studio_bootstrap_ui(bootstrap));
    documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
    commands = umi_ui_workbench_commands(workbench);
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    CHECK(umi_path_equal(umi_studio_build_service_profile(build)->source_directory, root));
    CHECK(umi_studio_build_service_profile(build)->run_program[0] == '\0');
    CHECK(umi_studio_build_service_set_profile(build, &profile) == UMI_STATUS_OK);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_COMPILE,
        "background", message, sizeof(message)) == UMI_STATUS_PERMISSION_DENIED);
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
    CHECK(umi_fs_join(source, sizeof(source), root, "src/main.c") == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_open(documents, source, viewId, sizeof(viewId)) == UMI_STATUS_OK);
    CHECK(EditSource(workbench, viewId, "#include <stdio.h>\nint main(void){puts(\"Umicom Notes: saved and built\");return 0;}\n", largeSource) == EXIT_SUCCESS);
    if (editCommands) {
        UmiDocumentWorkingCopySnapshot target, active;
        UmiDocumentEditPlan *plan = NULL;
        UmiUiDocumentViewModel *views = umi_ui_workbench_documents(workbench);
        UmiUiDocumentViewSnapshot view;
        char sideView[UMI_UI_ID_CAPACITY];
        char *text = NULL;
        size_t bytes = 0U;
        CHECK(umi_document_coordinator_active_snapshot(documents, &target) == UMI_STATUS_OK);
        CHECK(umi_ui_document_view_model_find(views, viewId, &view) == UMI_STATUS_OK);
        view.cursor_offset = 0U;
        view.selection_length = 0U;
        CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorPrepareEdit(documents, target.document_id, &plan) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_new(documents, "Build review notes", sideView, sizeof sideView) == UMI_STATUS_OK);
        CHECK(Edit(workbench, sideView, "Keep this independent draft.\n") == EXIT_SUCCESS);
        const char *comment = "/* Edited through the shared document contract. */\n";
        CHECK(UmiDocumentCoordinatorApplyEdit(documents, plan, comment, strlen(comment)) == UMI_STATUS_OK);
        UmiDocumentEditPlanDestroy(plan); plan = NULL;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(strcmp(active.view_id, sideView) == 0 && active.dirty);
        CHECK(UmiUiDocumentViewModelCopyText(views, sideView, &text, &bytes) == UMI_STATUS_OK);
        CHECK(strcmp(text, "Keep this independent draft.\n") == 0);
        UmiUiDocumentViewModelFreeText(text); text = NULL;
        CHECK(UmiDocumentCoordinatorUndo(documents, target.document_id) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorRedo(documents, target.document_id) == UMI_STATUS_OK);
        CHECK(UmiUiDocumentViewModelCopyText(views, viewId, &text, &bytes) == UMI_STATUS_OK);
        CHECK(strncmp(text, comment, strlen(comment)) == 0);
        UmiUiDocumentViewModelFreeText(text); text = NULL;
        /* A changed selection rejects a delayed edit, without erasing the
         * comment or affecting the second document. */
        CHECK(UmiDocumentCoordinatorPrepareEdit(documents, target.document_id, &plan) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorSelectAll(documents, target.document_id) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorApplyEdit(documents, plan, "wrong", 5U) == UMI_STATUS_INVALID_STATE);
        UmiDocumentEditPlanDestroy(plan);
        char sidePath[UMI_PATH_CAPACITY];
        CHECK(umi_fs_join(sidePath, sizeof sidePath, root, "review.txt") == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorSaveAs(documents, active.document_id, sidePath) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_open(documents, source, viewId, sizeof viewId) == UMI_STATUS_OK);
    }
    if (reloadFlow) {
        const char *diskSource = "/* saved by another editor */\n#include <stdio.h>\nint main(void){puts(\"Umicom Notes: saved and built\");return 0;}\n";
        UmiDocumentWorkingCopySnapshot target, active;
        UmiDocumentReloadPlan *plan = NULL;
        char sidePath[UMI_PATH_CAPACITY], sideView[UMI_UI_ID_CAPACITY];
        CHECK(umi_document_coordinator_active_snapshot(documents, &target) == UMI_STATUS_OK && target.dirty);
        CHECK(umi_fs_write_text(source, diskSource) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorPrepareReload(documents, target.document_id, &plan) == UMI_STATUS_OK);
        UmiTextComparison *review = NULL;
        if (compareFlow) {
            const char *previous = NULL, *incoming = NULL;
            size_t previousLength = 0U, incomingLength = 0U, firstChange = SIZE_MAX;
            UmiTextComparisonSummary summary;
            CHECK(UmiDocumentReloadPlanTexts(plan, &previous, &previousLength, &incoming, &incomingLength) == UMI_STATUS_OK);
            CHECK(strcmp(incoming, diskSource) == 0);
            CHECK(UmiTextComparisonCreate(previous, previousLength, incoming, incomingLength, &review) == UMI_STATUS_OK);
            CHECK(UmiTextComparisonGetSummary(review, &summary) == UMI_STATUS_OK);
            CHECK(!summary.identicalBytes && summary.alignmentStatus == UMI_STATUS_OK);
            CHECK(UmiTextComparisonNavigate(review, SIZE_MAX, 1, &firstChange) == UMI_STATUS_OK);
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
            CHECK(active.document_id == target.document_id && active.dirty);
        }
        CHECK(UmiDocumentCoordinatorApplyReload(documents, plan, 0) == UMI_STATUS_INVALID_STATE);
        CHECK(umi_fs_join(sidePath, sizeof sidePath, root, "notes.txt") == UMI_STATUS_OK);
        CHECK(umi_fs_write_text(sidePath, "saved side note\n") == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_open(documents, sidePath, sideView, sizeof sideView) == UMI_STATUS_OK);
        CHECK(Edit(workbench, sideView, "unsaved side note\n") == EXIT_SUCCESS);
        CHECK(UmiDocumentCoordinatorApplyReload(documents, plan, 1) == UMI_STATUS_OK);
        UmiDocumentReloadPlanDestroy(plan);
        if (review != NULL) {
            const char *captured = NULL; size_t capturedLength = 0U;
            CHECK(UmiTextComparisonText(review, 1, &captured, &capturedLength) == UMI_STATUS_OK);
            CHECK(capturedLength == strlen(diskSource) && strcmp(captured, diskSource) == 0);
            UmiTextComparisonDestroy(review);
        }
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(strcmp(active.view_id, sideView) == 0 && active.dirty);
        char *text = NULL; size_t length = 0U;
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), sideView, &text, &length) == UMI_STATUS_OK);
        CHECK(strcmp(text, "unsaved side note\n") == 0); UmiUiDocumentViewModelFreeText(text);
        CHECK(umi_ui_workbench_activate_document(workbench, viewId) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_redo(documents) == UMI_STATUS_OK);
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), viewId, &text, &length) == UMI_STATUS_OK);
        CHECK(strcmp(text, diskSource) == 0); UmiUiDocumentViewModelFreeText(text);
    }
    if (backgroundRefresh) {
        UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
        UmiFileIndex *index = umi_studio_services_file_index(services);
        UmiFileIndexRefreshSnapshot refresh;
        UmiFileIndexStats before = umi_file_index_stats(index);
        char added[UMI_PATH_CAPACITY];
        char *draft = NULL, *disk = NULL;
        size_t length = 0U;
        CHECK(umi_path_join(root, "notes-created-externally.txt", added, sizeof(added)) == UMI_STATUS_OK);
        CHECK(umi_fs_write_text(added, "A file created outside Studio.\n") == UMI_STATUS_OK);
        CHECK(UmiStudioUiRefreshProjectFiles(ui) == UMI_STATUS_OK);
        /* This exercises the UI facade while the real worker scans; no wait or
         * disk scan is hidden inside the normal UI refresh operation. */
        for (unsigned attempt = 0U; attempt < 3000U; ++attempt) {
            CHECK(umi_studio_ui_refresh(ui) == UMI_STATUS_OK);
            CHECK(UmiStudioUiProjectFileRefreshState(ui, &refresh) == UMI_STATUS_OK);
            if (!refresh.active) break;
            umi_thread_sleep_ms(1U);
        }
        CHECK(!refresh.active && refresh.status == UMI_STATUS_OK && refresh.requestId == 1U);
        CHECK(umi_file_index_stats(index).files == before.files + 1U);
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench),
            viewId, &draft, &length) == UMI_STATUS_OK);
        CHECK(strstr(draft, "Umicom Notes: saved and built") != NULL);
        UmiUiDocumentViewModelFreeText(draft);
        CHECK(umi_fs_read_text(source, &disk, NULL) == UMI_STATUS_OK);
        CHECK(strstr(disk, "Umicom Notes: saved and built") == NULL);
        umi_fs_free_text(disk);
        CHECK(UmiStudioUiCancelProjectFileRefresh(ui) == UMI_STATUS_OK);
        CHECK(UmiStudioUiRefreshProjectFiles(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioUiProjectFileRefreshState(ui, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    }
    if (projectFiles) {
        UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
        UmiWorkspaceGraphSnapshot workspace;
        UmiStudioProjectEntryResult created;
        CHECK(umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &workspace) == UMI_STATUS_OK);
        CHECK(UmiStudioUiCreateProjectEntry(ui, workspace.revision, "include",
            UMI_WORKSPACE_ENTRY_DIRECTORY, &created) == UMI_STATUS_OK);
        CHECK(created.entry.created);
        CHECK(UmiStudioUiCreateProjectEntry(ui, workspace.revision, "include/notes.h",
            UMI_WORKSPACE_ENTRY_FILE, &created) == UMI_STATUS_OK);
        CHECK(created.entry.created && created.index_status == UMI_STATUS_OK &&
            created.document_status == UMI_STATUS_OK);
        CHECK(Edit(workbench, created.view_id,
            "#ifndef UMICOM_NOTES_H\n#define UMICOM_NOTES_H\n#define UMICOM_NOTES_MESSAGE \"Umicom Notes: saved and built\"\n#endif\n") == EXIT_SUCCESS);
        char *draft = NULL;
        size_t draftLength = 0U;
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench),
            viewId, &draft, &draftLength) == UMI_STATUS_OK);
        CHECK(strstr(draft, "saved and built") != NULL);
        UmiUiDocumentViewModelFreeText(draft);
        CHECK(UmiStudioUiCreateProjectEntry(ui, workspace.revision, "include/notes.h",
            UMI_WORKSPACE_ENTRY_FILE, &created) == UMI_STATUS_ALREADY_EXISTS);
        CHECK(!created.entry.created);
        CHECK(Edit(workbench, viewId,
            "#include <stdio.h>\n#include \"../include/notes.h\"\nint main(void){puts(UMICOM_NOTES_MESSAGE);return 0;}\n") == EXIT_SUCCESS);
        CHECK(umi_ui_workbench_activate_document(workbench, viewId) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
        CHECK(UmiStudioUiCreateProjectEntry(ui, 0U, "denied.c", UMI_WORKSPACE_ENTRY_FILE,
            &created) == UMI_STATUS_PERMISSION_DENIED);
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        CHECK(UmiStudioUiCreateProjectEntry(ui, workspace.revision, "stale.c", UMI_WORKSPACE_ENTRY_FILE,
            &created) == UMI_STATUS_BUSY);
    }
    if (fileSearch) {
        UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
        UmiFileSearchSnapshot search;
        char notes[UMI_PATH_CAPACITY], notesView[UMI_UI_ID_CAPACITY];
        char *draft = NULL;
        size_t length = 0U;
        CHECK(umi_path_join(root, "notes-search.txt", notes, sizeof notes) == UMI_STATUS_OK);
        CHECK(umi_fs_write_text(notes, "Umicom Notes\n  UMICOM_SEARCH_MARKER\n") == UMI_STATUS_OK);
        CHECK(umi_file_index_update(umi_studio_services_file_index(services), notes) == UMI_STATUS_OK);
        CHECK(UmiStudioUiSearchStart(ui, "UMICOM_SEARCH_MARKER", 1) == UMI_STATUS_OK);
        for (unsigned n = 0U; n < 10000U; ++n) {
            CHECK(UmiStudioUiSearchRead(ui, &search) == UMI_STATUS_OK);
            if (!search.active) break;
            umi_thread_sleep_ms(1U);
        }
        CHECK(search.ready && search.stats.matches == 1U);
        CHECK(UmiStudioUiSearchOpen(ui, search.requestId, 0U) == UMI_STATUS_OK);
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, notes)); strcpy(notesView, active.view_id);
        UmiUiDocumentViewSnapshot *view = calloc(1U, sizeof(*view)); CHECK(view != NULL);
        CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(workbench), notesView, view) == UMI_STATUS_OK);
        CHECK(view->cursor_offset == strlen("Umicom Notes\n  ")); free(view);
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), viewId, &draft, &length) == UMI_STATUS_OK);
        CHECK(strstr(draft, "Umicom Notes: saved and built") != NULL); UmiUiDocumentViewModelFreeText(draft);
        CHECK(Edit(workbench, notesView, "New unsaved first line\nUmicom Notes\n  UMICOM_SEARCH_MARKER\n") == EXIT_SUCCESS);
        CHECK(UmiStudioUiSearchOpen(ui, search.requestId, 0U) == UMI_STATUS_NOT_FOUND);
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), notesView, &draft, &length) == UMI_STATUS_OK);
        CHECK(strncmp(draft, "New unsaved first line", strlen("New unsaved first line")) == 0); UmiUiDocumentViewModelFreeText(draft);
        /* Changing workspace identity invalidates old results even before a
         * new search is started; no old row can open a previous project. */
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(UmiStudioUiSearchRead(ui, &search) == UMI_STATUS_OK && search.stale);
        CHECK(UmiStudioUiSearchOpen(ui, search.requestId, 0U) == UMI_STATUS_BUSY);
        CHECK(UmiStudioUiSearchStart(ui, "note", 0) == UMI_STATUS_INVALID_STATE);
        CHECK(umi_studio_workspace_open(services, root, 1, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_build_service_set_profile(build, &profile) == UMI_STATUS_OK);
        CHECK(umi_ui_workbench_activate_document(workbench, viewId) == UMI_STATUS_OK);
    }
    CHECK(UmiDocumentCoordinatorSaveAll(documents, NULL) == UMI_STATUS_OK);
    CHECK(umi_build_result_create(&result) == UMI_STATUS_OK);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_RUN,
        "background", message, sizeof(message)) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_close(services) == UMI_STATUS_BUSY);
    if (backgroundRefresh) {
        UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
        CHECK(UmiStudioUiRefreshProjectFiles(ui) == UMI_STATUS_OK);
        CHECK(UmiStudioUiCancelProjectFileRefresh(ui) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    }
    CHECK(Wait(build, UMI_STATUS_OK, result) == EXIT_SUCCESS);
    CHECK(result->phase == UMI_BUILD_PHASE_RUN);
    CHECK(strstr(result->output, "Umicom Notes: saved and built") != NULL);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_TEST,
        "background", message, sizeof(message)) == UMI_STATUS_OK);
    CHECK(Wait(build, UMI_STATUS_OK, result) == EXIT_SUCCESS);
    CHECK(result->phase == UMI_BUILD_PHASE_TEST);
    {
        char testBuild[UMI_PATH_CAPACITY];
        size_t discovered = 0U;
        UmiStudioTestService *tests = umi_studio_services_tests(services);
        CHECK(umi_path_absolute(profile.build_directory, root, testBuild, sizeof(testBuild)) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_discover(tests, testBuild, &discovered) == UMI_STATUS_OK);
        CHECK(discovered == 1U);
        CHECK(umi_studio_test_service_discover(tests, testBuild, &discovered) == UMI_STATUS_OK);
        CHECK(discovered == 1U);
    }
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_INSTALL,
        "background", message, sizeof(message)) == UMI_STATUS_OK);
    CHECK(Wait(build, UMI_STATUS_OK, result) == EXIT_SUCCESS);
#ifdef _WIN32
    CHECK(umi_fs_join(installed, sizeof(installed), root, "install/windows-ucrt64-debug/bin/umicom_notes.exe") == UMI_STATUS_OK);
#else
    CHECK(umi_fs_join(installed, sizeof(installed), root, "install/linux-debug/bin/umicom_notes") == UMI_STATUS_OK);
#endif
    CHECK(umi_fs_is_file(installed));
    CHECK(EditSource(workbench, viewId, "#error Umicom_expected_compile_failure\n", largeSource) == EXIT_SUCCESS);
    CHECK(UmiDocumentCoordinatorSaveAll(documents, NULL) == UMI_STATUS_OK);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_RUN,
        "background", message, sizeof(message)) == UMI_STATUS_OK);
    CHECK(Wait(build, UMI_STATUS_INTERNAL_ERROR, result) == EXIT_SUCCESS);
    CHECK(result->phase == UMI_BUILD_PHASE_BUILD);
    CHECK(strstr(result->output, "Umicom_expected_compile_failure") != NULL);
    if (diagnosticsFlow) {
        UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
        UmiDiagnosticModel *problems = umi_diagnostic_pipeline_model(umi_studio_services_diagnostic_pipeline(services));
        UmiDiagnosticSnapshot problem;
        UmiDocumentWorkingCopySnapshot active;
        char scratch[UMI_UI_ID_CAPACITY], targetId[UMI_DIAGNOSTIC_ID_CAPACITY] = "";
        CHECK(result->diagnostics.count != 0U);
        CHECK(umi_studio_diagnostics_ingest_build_result(services,result)==UMI_STATUS_OK);
        for (size_t i=0U;i<umi_diagnostic_model_count(problems);++i) {
            CHECK(umi_diagnostic_model_at(problems,i,&problem)==UMI_STATUS_OK);
            if (strstr(problem.message,"Umicom_expected_compile_failure") != NULL && problem.uri[0] != '\0') {
                strcpy(targetId,problem.id); break;
            }
        }
        CHECK(targetId[0]!='\0' && problem.line==1U && umi_path_equal(problem.uri,source));
        CHECK(umi_document_coordinator_new(documents,"Unsaved notes",scratch,sizeof scratch)==UMI_STATUS_OK);
        CHECK(Edit(workbench,scratch,"Important unsaved notes\n")==EXIT_SUCCESS);
        CHECK(UmiStudioUiCanNavigateProblems(ui));
        CHECK(UmiStudioUiOpenProblem(ui,targetId)==UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK);
        CHECK(strcmp(active.view_id,viewId)==0);
        char *draft=NULL; size_t length=0U;
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench),scratch,&draft,&length)==UMI_STATUS_OK);
        CHECK(strcmp(draft,"Important unsaved notes\n")==0); UmiUiDocumentViewModelFreeText(draft);
        CHECK(Edit(workbench,viewId,"#include <stdio.h>\nint main(void){puts(\"Umicom Notes: corrected\");return 0;}\n")==EXIT_SUCCESS);
        CHECK(umi_document_coordinator_save_active(documents)==UMI_STATUS_OK);
        /* Submit directly because Save All must correctly reject the separate
         * unnamed draft. Navigation is not permission to discard that draft. */
        CHECK(UmiStudioBuildSubmit(build,UMI_BUILD_PHASE_RUN,1)==UMI_STATUS_OK);
        for(unsigned attempt=0U;attempt<6000U && UmiStudioBuildBusy(build);++attempt) {
            CHECK(umi_studio_ui_refresh(ui)==UMI_STATUS_OK);
            umi_thread_sleep_ms(10U);
        }
        CHECK(!UmiStudioBuildBusy(build));
        UmiBuildProjectSessionSnapshot progress;
        CHECK(UmiStudioBuildProgress(build,&progress)==UMI_STATUS_OK && progress.status==UMI_STATUS_OK);
        CHECK(umi_diagnostic_model_find(problems,targetId,&problem)==UMI_STATUS_OK && problem.resolved);
        CHECK(!UmiStudioUiCanNavigateProblems(ui));
        CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench),scratch,&draft,&length)==UMI_STATUS_OK);
        CHECK(strcmp(draft,"Important unsaved notes\n")==0); UmiUiDocumentViewModelFreeText(draft);
        /* The source-location operation keeps history and file bytes unchanged. */
        CHECK(UmiDocumentCoordinatorGoToPosition(documents,2U,5U,NULL)==UMI_STATUS_OK);
        UmiUiDocumentViewSnapshot view;
        CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(workbench),viewId,&view)==UMI_STATUS_OK);
        CHECK(view.cursor_offset==23U); /* first line 19 bytes, then byte column 5 */
    }
    if (backgroundRefresh) {
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(UmiStudioUiRefreshProjectFiles(umi_studio_bootstrap_ui(bootstrap)) == UMI_STATUS_INVALID_STATE);
    }
    CHECK(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap); umi_build_result_destroy(result);
    umi_developer_project_service_destroy(projects);
    CHECK(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    puts("Studio project workflow passed: generation, editing, trust, run, tests, install and compiler failure.");
    return EXIT_SUCCESS;
}
