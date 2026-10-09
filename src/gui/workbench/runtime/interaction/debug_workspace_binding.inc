/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_workspace_binding.inc
 * PURPOSE: Share weak native-debugger workspace ownership across configuration panels.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
typedef struct StudioDebugWorkspaceBinding
{
    UmiStudioFileDialogRequest *owner;
    uint64_t workspace_revision;
    char root[UMI_PATH_CAPACITY];
} StudioDebugWorkspaceBinding;
static void RuntimeDebugWorkspaceBindingFree(gpointer data)
{
    StudioDebugWorkspaceBinding *binding = data;
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
static UmiStatus RuntimeDebugWorkspaceOwner(StudioDebugWorkspaceBinding *binding, GObject **window,
                                            UmiStudioServices **services,
                                            UmiStudioWorkspaceSnapshot *workspace)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(*window)))
        return UMI_STATUS_CANCELLED;
    *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = umi_studio_workspace_snapshot(*services, workspace);
    if (status == UMI_STATUS_OK && (!workspace->graph.open || !workspace->graph.trusted))
        status = UMI_STATUS_PERMISSION_DENIED;
    return status;
}
