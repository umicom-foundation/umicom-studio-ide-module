/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_run_current_workflow.inc
 * PURPOSE: Verify launch-only execution preserves a saved source and a different unsaved draft.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Called after the surrounding workflow builds its real executable. A broken
 * unsaved draft must neither be saved nor trigger a compile in this action. */
static int VerifyRunCurrentWorkflow(UmiStudioServices *services, UmiCommandRegistry *commands,
                                    UmiUiWorkbench *workbench, UmiDocumentCoordinator *documents,
                                    const char *view_id, const char *source, UmiBuildResult *result)
{
    UmiStudioBuildService *build = umi_studio_services_build(services);
    const char *unsaved = "#error unsaved source must not be compiled by launch-only\n";
    char *saved = NULL, *after = NULL, *draft = NULL;
    size_t saved_length = 0U, after_length = 0U, draft_length = 0U;
    char message[512];
    CHECK(umi_fs_read_text(source, &saved, &saved_length) == UMI_STATUS_OK);
    CHECK(Edit(workbench, view_id, unsaved) == EXIT_SUCCESS);
    CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL,
                                       message, sizeof message) == UMI_STATUS_PERMISSION_DENIED);
    CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL,
                                       message, sizeof message) == UMI_STATUS_OK);
    CHECK(Wait(build, UMI_STATUS_OK, result) == EXIT_SUCCESS);
    UmiBuildProjectSessionSnapshot progress;
    CHECK(UmiStudioBuildProgress(build, &progress) == UMI_STATUS_OK);
    CHECK(progress.completed_phase_count == 1U && result->phase == UMI_BUILD_PHASE_RUN);
    CHECK(strstr(result->output, "Umicom Notes: saved and built") != NULL);
    CHECK(umi_fs_read_text(source, &after, &after_length) == UMI_STATUS_OK);
    CHECK(after_length == saved_length && memcmp(saved, after, saved_length) == 0);
    CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), view_id, &draft,
                                         &draft_length) == UMI_STATUS_OK);
    CHECK(strcmp(draft, unsaved) == 0);
    UmiUiDocumentViewModelFreeText(draft);
    umi_fs_free_text(after);
    /* Restore the test's editor text explicitly so later build/test/install
     * stages exercise the same program as the original workflow. */
    CHECK(Edit(workbench, view_id, saved) == EXIT_SUCCESS);
    CHECK(UmiDocumentCoordinatorSaveAll(documents, NULL) == UMI_STATUS_OK);
    umi_fs_free_text(saved);
    return EXIT_SUCCESS;
}
