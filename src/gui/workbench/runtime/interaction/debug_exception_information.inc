/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_exception_information.inc
 * PURPOSE: Request exception details through the current trusted Studio debugger owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_exception_information.h"
static UmiStatus RuntimeDebugExceptionInformationQuery(void *context,
                                                       UmiDebugExceptionTarget *target,
                                                       UmiDebugExceptionInformation *information)
{
    StudioDebugWorkspaceBinding *binding = context;
    GObject *window = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
    UmiDebugRuntimePlatform *platform =
        status == UMI_STATUS_OK
            ? UmiStudioDebuggerNativePlatform(umi_studio_services_debugger(services))
            : NULL;
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeExceptionTargetRead(platform, target);
    if (status == UMI_STATUS_OK)
        status = UmiDebugRuntimeExceptionInformationRead(platform, target, 1500U, information);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeDebugExceptionInformationCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    StudioDebugWorkspaceBinding *binding = g_new0(StudioDebugWorkspaceBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        return gtk_label_new("Exception inspection is unavailable.");
    }
    GtkWidget *panel = UmiGtk4DebugExceptionInformationCreate(
        RuntimeDebugExceptionInformationQuery, binding, RuntimeDebugWorkspaceBindingFree);
    if (panel == NULL)
    {
        RuntimeDebugWorkspaceBindingFree(binding);
        return gtk_label_new("Exception inspection is unavailable.");
    }
    GtkWidget *expander = gtk_expander_new("Stopped exception");
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    return expander;
}
