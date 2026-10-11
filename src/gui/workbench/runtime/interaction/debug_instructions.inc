/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_instructions.inc
 * PURPOSE: Keep paused instruction browsing bound to its trusted Studio project.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_instructions.h"
typedef struct StudioInstructionBinding
{
    StudioDebugWorkspaceBinding workspace;
    UmiDebugInstructionTarget *target;
} StudioInstructionBinding;
static void RuntimeInstructionsFree(gpointer data)
{
    StudioInstructionBinding *binding = data;
    UmiDebugInstructionTargetDestroy(binding->target);
    runtime_file_dialog_request_free(binding->workspace.owner);
    g_free(binding);
}
static UmiStatus RuntimeInstructionsCapture(void *context, UmiDebugInstructionContext *out)
{
    StudioInstructionBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    /* A new Refresh retires the previous frame even if the new request fails.
     * The Framework target owns the stopped-frame evidence; Studio owns only
     * the project identity and the weak relationship to its window. */
    UmiDebugInstructionTargetDestroy(binding->target);
    binding->target = NULL;
    UmiStatus status =
        RuntimeDebugWorkspaceOwner(&binding->workspace, &window, &services, &workspace);
    if (status == UMI_STATUS_OK)
    {
        UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
        status = UmiDebugInstructionTargetCapture(UmiStudioDebuggerNativePlatform(debugger),
                                                  umi_studio_debugger_service_workspace(debugger),
                                                  1500U, &binding->target);
    }
    if (status == UMI_STATUS_OK)
        status = UmiDebugInstructionTargetRead(binding->target, out);
    if (status == UMI_STATUS_OK)
    {
        binding->workspace.workspace_revision = workspace.graph.revision;
        g_strlcpy(binding->workspace.root, workspace.graph.root, sizeof binding->workspace.root);
    }
    g_clear_object(&window);
    return status;
}
static UmiStatus RuntimeInstructionsRead(void *context, const UmiDebugInstructionContext *expected,
                                         int64_t bytes, int64_t offset, uint32_t count, int symbols,
                                         UmiDebugInstructionPage *out)
{
    StudioInstructionBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiDebugInstructionContext captured;
    UmiStatus status =
        RuntimeDebugWorkspaceOwner(&binding->workspace, &window, &services, &workspace);
    if (status == UMI_STATUS_OK &&
        (binding->workspace.workspace_revision != workspace.graph.revision ||
         !umi_path_equal(binding->workspace.root, workspace.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
        status = UmiDebugInstructionTargetRead(binding->target, &captured);
    if (status == UMI_STATUS_OK && !UmiDebugInstructionContextEqual(expected, &captured))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
    {
        UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
        status =
            UmiDebugInstructionPageRead(UmiStudioDebuggerNativePlatform(debugger),
                                        umi_studio_debugger_service_workspace(debugger),
                                        binding->target, bytes, offset, count, symbols, 1500U, out);
    }
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugInstructionsCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    StudioInstructionBinding *binding = g_new0(StudioInstructionBinding, 1);
    binding->workspace.owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->workspace.owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Instruction browsing is unavailable.");
    }
    GtkWidget *panel = UmiGtk4DebugInstructionsCreate(
        RuntimeInstructionsCapture, RuntimeInstructionsRead, binding, RuntimeInstructionsFree);
    if (panel == NULL)
    {
        RuntimeInstructionsFree(binding);
        return gtk_label_new("Instruction browsing is unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Machine instructions");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
