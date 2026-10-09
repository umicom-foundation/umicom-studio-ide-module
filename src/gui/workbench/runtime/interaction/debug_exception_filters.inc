/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_exception_filters.inc
 * PURPOSE: Bind shared exception controls to the current trusted Studio workspace.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_exception_filters.h"
/* Share the native debugger workspace binding across configuration panels. The earlier exception-specific shape is retained for review. The previous implementation is retained for engineering review. */
#if 0
typedef struct StudioExceptionBinding
{
    UmiStudioFileDialogRequest *owner;
    uint64_t workspace_revision;
    char root[UMI_PATH_CAPACITY];
} StudioExceptionBinding;
#endif
typedef StudioDebugWorkspaceBinding StudioExceptionBinding;
/* The shared debug-workspace owner now releases the weak window token. Preserve this private adapter and its previous implementation for review. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeExceptionBindingFree(gpointer data)
{
    StudioExceptionBinding *binding = data;
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
#endif
static void RuntimeExceptionBindingFree(gpointer data)
{
    RuntimeDebugWorkspaceBindingFree(data);
}
/* Use one workspace and window-lifetime check for native debugger configuration panels. The previous exception-specific implementation is retained for review. The previous implementation is retained for engineering review. */
#if 0
static UmiStatus RuntimeExceptionOwner(StudioExceptionBinding *binding, GObject **window,
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
#endif
static UmiStatus RuntimeExceptionOwner(StudioExceptionBinding *binding, GObject **window,
    UmiStudioServices **services, UmiStudioWorkspaceSnapshot *workspace)
{
    return RuntimeDebugWorkspaceOwner(binding, window, services, workspace);
}
static UmiStatus RuntimeExceptionRead(void *context, UmiDebugExceptionSnapshot *out)
{
    StudioExceptionBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeExceptionOwner(binding, &window, &services, &workspace);
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeExceptionFiltersRead(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), out);
    if (status == UMI_STATUS_OK)
    {
        binding->workspace_revision = workspace.graph.revision;
        g_strlcpy(binding->root, workspace.graph.root, sizeof binding->root);
    }
    g_clear_object(&window);
    return status;
}
static UmiStatus RuntimeExceptionApply(void *context, const UmiDebugExceptionSelection *selection)
{
    StudioExceptionBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeExceptionOwner(binding, &window, &services, &workspace);
    /* Checking the captured workspace as well as the debugger's session keeps a
     * late control activation from crossing a project or trust transition. */
    if (status == UMI_STATUS_OK && (workspace.graph.revision != binding->workspace_revision ||
                                    !umi_path_equal(workspace.graph.root, binding->root)))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeExceptionFiltersApply(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), selection,
            1500U);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugExceptionFiltersCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    StudioExceptionBinding *binding = g_new0(StudioExceptionBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Exception controls are unavailable.");
    }
    GtkWidget *panel = UmiGtk4DebugExceptionFiltersCreate(
        RuntimeExceptionRead, RuntimeExceptionApply, binding, RuntimeExceptionBindingFree);
    if (panel == NULL)
    {
        RuntimeExceptionBindingFree(binding);
        return gtk_label_new("Exception controls are unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Pause on exceptions");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
