/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_source_navigation.inc
 * PURPOSE: Exercise source navigation through the real compiler workflow without replacing drafts.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Reuse the failed real build from the project workflow. The retained page is
 * checked by identity again when navigating, and edits remain owned by the
 * document coordinator rather than by the diagnostics presenter. */
static int VerifyBuildSourceNavigation(UmiStudioUi *ui, UmiStudioBuildService *build,
                                       UmiDocumentCoordinator *documents, UmiUiWorkbench *workbench,
                                       const char *source_view)
{
    UmiBuildDiagnosticProgress progress;
    CHECK(UmiStudioBuildReadDiagnostics(build, &progress) == UMI_STATUS_OK);
    CHECK(progress.operation_id != 0U);
    UmiBuildDiagnosticPage *page = calloc(1U, sizeof *page);
    CHECK(page != NULL);
    size_t selected = SIZE_MAX;
    for (size_t first = 0U; first < UMI_BUILD_MAX_DIAGNOSTICS;
         first += UMI_BUILD_DIAGNOSTIC_PAGE_CAPACITY)
    {
        UmiStatus status = UmiStudioBuildReadDiagnosticPage(build, progress.operation_id,
                                                            progress.phase_index, first, page);
        if (status == UMI_STATUS_NOT_FOUND)
            break;
        CHECK(status == UMI_STATUS_OK);
        for (size_t row = 0U; row < page->count; ++row)
            if (strstr(page->items[row].message, "Umicom_expected_compile_failure") != NULL &&
                page->items[row].line != 0U && page->items[row].file[0] != '\0')
                selected = first + row;
        if (selected != SIZE_MAX || page->count == 0U)
            break;
    }
    free(page);
    CHECK(selected != SIZE_MAX);
    CHECK(Edit(workbench, source_view, "/* Unsaved source stays here. */\n") == EXIT_SUCCESS);
    char side[UMI_UI_ID_CAPACITY];
    CHECK(umi_document_coordinator_new(documents, "Unsaved navigation note", side, sizeof side) ==
          UMI_STATUS_OK);
    CHECK(Edit(workbench, side, "Do not replace this note.\n") == EXIT_SUCCESS);
    CHECK(UmiStudioUiOpenBuildDiagnostic(ui, progress.operation_id, progress.phase_index + 1U,
                                         selected) == UMI_STATUS_INVALID_STATE);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(strcmp(active.view_id, side) == 0);
    CHECK(UmiStudioUiOpenBuildDiagnostic(ui, progress.operation_id, progress.phase_index,
                                         selected) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(strcmp(active.view_id, source_view) == 0 && active.dirty);
    char *draft = NULL;
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), source_view, &draft,
                                         &bytes) == UMI_STATUS_OK);
    CHECK(strcmp(draft, "/* Unsaved source stays here. */\n") == 0);
    UmiUiDocumentViewModelFreeText(draft);
    CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(workbench), side, &draft,
                                         &bytes) == UMI_STATUS_OK);
    CHECK(strcmp(draft, "Do not replace this note.\n") == 0);
    UmiUiDocumentViewModelFreeText(draft);
    CHECK(UmiStudioUiOpenBuildDiagnostic(ui, progress.operation_id, progress.phase_index,
                                         UMI_BUILD_MAX_DIAGNOSTICS) == UMI_STATUS_INVALID_ARGUMENT);
    /* Retire only this temporary unsaved note; the source remains open for the
     * caller's original successful repair and rebuild portion of the workflow. */
    CHECK(umi_ui_workbench_activate_document(workbench, side) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_close_active(documents, 1) == UMI_STATUS_OK);
    CHECK(umi_ui_workbench_activate_document(workbench, source_view) == UMI_STATUS_OK);
    return EXIT_SUCCESS;
}
