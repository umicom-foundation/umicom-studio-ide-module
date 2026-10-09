/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_sources.inc
 * PURPOSE: Bind debugger source references to the trusted Studio workspace that requested them.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_sources.h"
static UmiStatus RuntimeDebugSourcesRefresh(void *context, UmiDebugSourceCatalog *out)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeSourceCatalogRead(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), 1500U, out);
    if (status == UMI_STATUS_OK)
    {
        binding->workspace_revision = workspace.graph.revision;
        g_strlcpy(binding->root, workspace.graph.root, sizeof binding->root);
    }
    g_clear_object(&window);
    return status;
}
static UmiStatus RuntimeDebugSourcesRead(void *context,
                                         const UmiDebugConnectionIdentity *connection,
                                         uint32_t reference, UmiDebugSourceContent *out)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    /* The native connection and the Studio workspace must both still match the
     * captured catalogue. Neither a reused session name nor a new project is enough. */
    if (status == UMI_STATUS_OK && (binding->workspace_revision != workspace.graph.revision ||
                                    !umi_path_equal(binding->root, workspace.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeSourceContentRead(
            UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services)), connection,
            reference, 1500U, out);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugSourcesCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    StudioDebugWorkspaceBinding *binding = g_new0(StudioDebugWorkspaceBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Source browsing is unavailable.");
    }
    GtkWidget *panel =
        UmiGtk4DebugSourcesCreate(RuntimeDebugSourcesRefresh, RuntimeDebugSourcesRead, binding,
                                  RuntimeDebugWorkspaceBindingFree);
    if (panel == NULL)
    {
        RuntimeDebugWorkspaceBindingFree(binding);
        return gtk_label_new("Source browsing is unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Debugger sources");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
