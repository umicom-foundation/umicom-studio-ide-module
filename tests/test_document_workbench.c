/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_document_workbench.c
 *
 * PURPOSE:
 *   Verify Studio composes Framework document actions and commands without
 *   duplicating document I/O or editor behaviour in the application layer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/workbench_commands.h"
#include "umicom/studio/workspace.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioUi *ui;
    UmiUiActionSnapshot action;
    UmiUiMenuSnapshot menu;
    UmiUiContextSnapshot context;
    UmiStudioWorkspaceSnapshot workspace;
    UmiDocumentWorkingCopySnapshot document;
    UmiCommandRegistry *commands;
    char temporary[UMI_PATH_CAPACITY];
    char workspace_root[UMI_PATH_CAPACITY];
    char source_path[UMI_PATH_CAPACITY];
    char message[512];

    (void)umi_fs_remove_tree(".umicom");
    /* Build a small real workspace so this test verifies folder selection all
     * the way through indexing without scanning or changing the source tree. */
    assert(umi_fs_temp_directory(temporary, sizeof(temporary)) == UMI_STATUS_OK);
    assert(umi_path_join(temporary, "umicom-studio-open-folder-test",
                         workspace_root, sizeof(workspace_root)) ==
           UMI_STATUS_OK);
    assert(umi_fs_remove_tree(workspace_root) == UMI_STATUS_OK);
    assert(umi_fs_make_directories(workspace_root) == UMI_STATUS_OK);
    assert(umi_path_join(workspace_root, "main.c", source_path,
                         sizeof(source_path)) == UMI_STATUS_OK);
    assert(umi_fs_write_text(source_path, "int main(void){return 0;}\n") ==
           UMI_STATUS_OK);
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    ui = umi_studio_bootstrap_ui(bootstrap);
    commands = umi_studio_bootstrap_command_registry(bootstrap);
    assert(ui != NULL && commands != NULL);
    assert(umi_document_coordinator_count(umi_studio_ui_documents(ui)) >= 1U);
    assert(umi_ui_action_model_find(
        umi_ui_workbench_actions(umi_studio_ui_workbench(ui)),
        "studio.action.file.open", &action) == UMI_STATUS_OK);
    assert(strcmp(action.command_id, UMI_DOCUMENT_COMMAND_OPEN) == 0);
    assert(action.argument_kind == UMI_UI_ACTION_ARGUMENT_OPEN_PATH);
    assert(umi_ui_action_model_find(
        umi_ui_workbench_actions(umi_studio_ui_workbench(ui)),
        "studio.action.workspace.open-folder", &action) == UMI_STATUS_OK);
    assert(strcmp(action.command_id,
                  UMI_STUDIO_COMMAND_WORKSPACE_OPEN_FOLDER) == 0);
    assert(action.argument_kind == UMI_UI_ACTION_ARGUMENT_FOLDER_PATH);
    assert(umi_ui_menu_model_find(
        umi_ui_workbench_menus(umi_studio_ui_workbench(ui)),
        "menu.file.open-folder", &menu) == UMI_STATUS_OK);
    assert(strcmp(menu.action_id, action.action_id) == 0);
    assert(umi_command_registry_execute(
        commands, UMI_STUDIO_COMMAND_WORKSPACE_OPEN_FOLDER,
        workspace_root, message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "Workspace opened:") != NULL);
    assert(umi_studio_workspace_snapshot(
        umi_studio_ui_services(ui), &workspace) == UMI_STATUS_OK);
    assert(workspace.graph.open && !workspace.graph.trusted);
    assert(workspace.files.files == 1U && workspace.watcher.running);
    assert(umi_ui_context_get(
        umi_ui_workbench_context(umi_studio_ui_workbench(ui)),
        "studio.workspace.open", &context) == UMI_STATUS_OK);
    assert(context.kind == UMI_UI_CONTEXT_BOOLEAN && context.boolean_value);
    assert(umi_ui_context_get(
        umi_ui_workbench_context(umi_studio_ui_workbench(ui)),
        "studio.workspace.root", &context) == UMI_STATUS_OK);
    assert(context.kind == UMI_UI_CONTEXT_STRING &&
           context.string_value[0] != '\0');
    assert(umi_command_registry_execute(commands, UMI_DOCUMENT_COMMAND_NEW,
                                        "Workbench.c", message,
                                        sizeof(message)) == UMI_STATUS_OK);
    assert(umi_document_coordinator_active_snapshot(
        umi_studio_ui_documents(ui), &document) == UMI_STATUS_OK);
    assert(strcmp(document.display_name, "Workbench.c") == 0);
    assert(document.dirty && !document.has_path);
    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
    assert(umi_fs_remove_tree(workspace_root) == UMI_STATUS_OK);
    (void)printf("Studio document workbench passed\n");
    return 0;
}
