/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_editor_workflow.c
 * PURPOSE: Follow real Studio documents from typing through navigation and save.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/close_guard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

/* The fixture owns only a new subdirectory in its isolated CTest directory. */
static int Run(UmiStudioBootstrap *bootstrap, const char *root)
{
    char path[UMI_PATH_CAPACITY], viewId[UMI_UI_ID_CAPACITY], reopened[UMI_UI_ID_CAPACITY];
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiUiWorkbench *workbench = umi_studio_ui_workbench(ui);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewSnapshot view;
    UmiDocumentWorkingCopySnapshot edited, other, current;
    UmiIdeActiveContext *context = NULL;
    UmiStudioRuntimeCloseReport report;
    size_t offset = 0U, replaced = 0U, saved = 0U, length = 0U;
    char *disk = NULL;
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    CHECK(umi_fs_join(path, sizeof path, root, "notes.c") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(path, "int noteCount = 1;\n") == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_open(documents, path, viewId, sizeof viewId) == UMI_STATUS_OK);
    CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(workbench), viewId, &view) == UMI_STATUS_OK);
    strcpy(view.source_text, "int noteCount = 3;\nint total = noteCount;\n"); view.dirty = 1;
    CHECK(umi_ui_document_view_model_upsert(umi_ui_workbench_documents(workbench), &view) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &edited) == UMI_STATUS_OK && edited.dirty);
    context = calloc(1U, sizeof *context);
    CHECK(context != NULL);
    UmiStatus status = UmiStudioCloseGuardEvaluateWithActivity(documents, context, 0, &report);
    free(context);
    CHECK(status == UMI_STATUS_OK && report.dirty_document_count >= 1U);
    CHECK(UmiDocumentCoordinatorFindNext(documents, "noteCount", 0, &offset, NULL) == UMI_STATUS_OK && offset == 4U);
    CHECK(UmiDocumentCoordinatorFindNext(documents, "noteCount", 0, &offset, NULL) == UMI_STATUS_OK && offset == 31U);
    CHECK(UmiDocumentCoordinatorReplaceAll(documents, "noteCount", "savedCount", &replaced) == UMI_STATUS_OK && replaced == 2U);
    CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
    CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(workbench), viewId, &view) == UMI_STATUS_OK);
    CHECK(strstr(view.source_text, "int noteCount = 3;") != NULL);
    CHECK(umi_document_coordinator_redo(documents) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorSaveAll(documents, &saved) == UMI_STATUS_OK && saved == 1U);
    CHECK(umi_fs_read_text(path, &disk, &length) == UMI_STATUS_OK);
    int expected = strcmp(disk, "int savedCount = 3;\nint total = savedCount;\n") == 0;
    umi_fs_free_text(disk);
    CHECK(expected);
    CHECK(umi_document_coordinator_new(documents, "other.c", NULL, 0U) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &other) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_open(documents, path, reopened, sizeof reopened) == UMI_STATUS_OK);
    CHECK(strcmp(reopened, viewId) == 0);
    CHECK(umi_ui_workbench_activate_document(workbench, other.view_id) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorClose(documents, edited.document_id, 0) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &current) == UMI_STATUS_OK && current.document_id == other.document_id);
    CHECK(UmiDocumentCoordinatorClose(documents, edited.document_id, 1) == UMI_STATUS_NOT_FOUND);
    CHECK(UmiDocumentCoordinatorClose(documents, other.document_id, 1) == UMI_STATUS_OK);
    puts("Studio editor workflow: draft detection, navigation, replacement, undo/redo, save and captured close passed.");
    return 0;
}

/* Always tear down the actual service container, including failed runs. */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServicesOptions options = {0};
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY];
    int result = 1;
    CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
    CHECK(umi_fs_join(root, sizeof root, cwd, "editor-workspace") == UMI_STATUS_OK);
    CHECK(!umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    if (umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK &&
        umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK) result = Run(bootstrap, root);
    if (bootstrap != NULL) {
        (void)umi_studio_bootstrap_stop(bootstrap);
        umi_studio_bootstrap_destroy(bootstrap);
    }
    if (umi_fs_remove_tree(root) != UMI_STATUS_OK) result = 1;
    return result;
}
