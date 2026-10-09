/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_modules.inc
 * PURPOSE: Bind module pagination to the captured native debugger and trusted workspace.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_modules.h"
static UmiStatus RuntimeDebugModulesQuery(void *context, const UmiDebugModuleSession *expected,
                                          uint32_t first, uint32_t count, UmiDebugModulePage *out)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    if (status == UMI_STATUS_OK && expected != NULL &&
        (binding->workspace_revision != workspace.graph.revision ||
         !umi_path_equal(binding->root, workspace.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    UmiDebugModuleSession session;
    UmiDebugRuntimePlatform *platform =
        status == UMI_STATUS_OK
            ? UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services))
            : NULL;
    if (status == UMI_STATUS_OK)
    {
        if (expected != NULL)
            session = *expected;
        else
            status = UmiDebugRuntimeModuleSessionRead(platform, &session);
    }
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeModulePageRead(platform, &session, first, count, 1500U, out);
    if (status == UMI_STATUS_OK)
    {
        binding->workspace_revision = workspace.graph.revision;
        g_strlcpy(binding->root, workspace.graph.root, sizeof binding->root);
    }
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugModulesCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    StudioDebugWorkspaceBinding *binding = g_new0(StudioDebugWorkspaceBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Module browsing is unavailable.");
    }
    GtkWidget *panel = UmiGtk4DebugModulesCreate(RuntimeDebugModulesQuery, binding,
                                                 RuntimeDebugWorkspaceBindingFree);
    if (panel == NULL)
    {
        RuntimeDebugWorkspaceBindingFree(binding);
        return gtk_label_new("Module browsing is unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Loaded modules");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
