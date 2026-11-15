/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/source_navigation_commands.inc
 *
 * PURPOSE:
 *   Compose shared source inspection commands for menus and editor controls.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Studio contributes only its settings namespace and current project folder.
 * Framework owns source capture, review state, protocol work and result opening.
 * Publication can synchronously retire this workbench, so no owner is borrowed
 * again until its weak binding has been checked. */
static UmiStatus RuntimeReviewSourceCommand(
    UmiStudioGtkRuntimeChrome *runtime, const char *command_id, const char *argument)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        !UmiGtk4AdapterSourceNavigationCommandKnown(command_id) ||
        (argument != NULL && argument[0] != '\0'))
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder = NULL;
    if (services != NULL &&
        umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
        workspace.graph.open)
        folder = workspace.graph.root;
    UmiStatus status = UmiGtk4AdapterSourceNavigationCommand(
        runtime->owner->adapter, command_id, folder, "Studio", NULL);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL && status == UMI_STATUS_OK) status = UMI_STATUS_CANCELLED;
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Toolbar buttons report errors here; menu dispatch already has its own result
 * presentation. Both enter exactly the same shared command above. */
static void RuntimeReviewSourceButton(
    UmiStudioGtkRuntimeChrome *runtime, const char *command_id)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return;
    UmiStatus status = RuntimeReviewSourceCommand(runtime, command_id, NULL);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK && status != UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime, status == UMI_STATUS_BUSY
            ? "Finish the current document operation before opening source navigation."
            : "Open a source draft within the language-server message limit. "
              "Request results explicitly in the review window.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
