/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_function_breakpoints.inc
 * PURPOSE: Expose function-breakpoint drafts through the native debugger and current workspace authority.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_function_breakpoints.h"
static UmiStatus RuntimeFunctionBreakpointRead(void *context, UmiDebugFunctionSnapshot *out)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeFunctionBreakpointsRead(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), out);
    if (status == UMI_STATUS_OK)
    {
        binding->workspace_revision = workspace.graph.revision;
        g_strlcpy(binding->root, workspace.graph.root, sizeof binding->root);
    }
    g_clear_object(&window);
    return status;
}
static UmiStatus RuntimeFunctionBreakpointApply(void *context, const UmiDebugFunctionDraft *draft)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    if (status == UMI_STATUS_OK && (workspace.graph.revision != binding->workspace_revision ||
                                    !umi_path_equal(workspace.graph.root, binding->root)))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeFunctionBreakpointsApply(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), draft, 1500U);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugFunctionBreakpointsCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    /* This binding has its own captured workspace state. Sharing the lifecycle
     * helpers does not share another panel's selection or review authority. */
    StudioDebugWorkspaceBinding *binding = g_new0(StudioDebugWorkspaceBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Function breakpoints are unavailable.");
    }
    GtkWidget *panel = UmiGtk4DebugFunctionBreakpointsCreate(RuntimeFunctionBreakpointRead,
                                                             RuntimeFunctionBreakpointApply,
                                                             binding, RuntimeDebugWorkspaceBindingFree);
    if (panel == NULL)
    {
        RuntimeDebugWorkspaceBindingFree(binding);
        return gtk_label_new("Function breakpoints are unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Function breakpoints");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
