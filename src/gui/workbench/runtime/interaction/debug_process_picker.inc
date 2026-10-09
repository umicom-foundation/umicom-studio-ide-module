/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_process_picker.inc
 * PURPOSE: Connect shared process discovery to the explicit Studio attach form without retaining a workbench.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/process_picker.h"
typedef struct StudioProcessSelection
{
    UmiDesktopSystemProcess process;
    uint64_t workspace_revision;
    char root[UMI_PATH_CAPACITY];
} StudioProcessSelection;
typedef struct StudioProcessPickerBinding
{
    UmiStudioFileDialogRequest *owner;
    GWeakRef pid;
} StudioProcessPickerBinding;
static void RuntimeProcessPickerBindingFree(gpointer data)
{
    StudioProcessPickerBinding *binding = data;
    runtime_file_dialog_request_free(binding->owner);
    g_weak_ref_clear(&binding->pid);
    g_free(binding);
}
static void RuntimeProcessManualEdit(GtkEditable *editable, gpointer data)
{
    (void)data;
    /* Editing the field means the user is supplying a fresh manual PID. Drop
     * the old observation rather than associating its creation time with new text. */
    g_object_set_data(G_OBJECT(editable), "umicom-selected-process", NULL);
}
static UmiStatus RuntimeProcessChoose(void *context, const UmiDesktopSystemProcess *process)
{
    StudioProcessPickerBinding *binding = context;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    GtkWidget *pid = g_weak_ref_get(&binding->pid);
    if (runtime != NULL && pid != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
    {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        UmiStudioWorkspaceSnapshot workspace;
        status = umi_studio_workspace_snapshot(services, &workspace);
        if (status == UMI_STATUS_OK && (!workspace.graph.open || !workspace.graph.trusted))
            status = UMI_STATUS_PERMISSION_DENIED;
        if (status == UMI_STATUS_OK)
        {
            StudioProcessSelection *selection = g_new0(StudioProcessSelection, 1);
            selection->process = *process;
            selection->workspace_revision = workspace.graph.revision;
            g_strlcpy(selection->root, workspace.graph.root, sizeof selection->root);
            char text[32];
            g_snprintf(text, sizeof text, "%llu", (unsigned long long)process->pid);
            gtk_editable_set_text(GTK_EDITABLE(pid), text);
            g_object_set_data_full(G_OBJECT(pid), "umicom-selected-process", selection, g_free);
        }
    }
    g_clear_object(&pid);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeProcessPickerCreate(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *pid)
{
    StudioProcessPickerBinding *binding = g_new0(StudioProcessPickerBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    g_weak_ref_init(&binding->pid, pid);
    if (binding->owner == NULL)
    {
        RuntimeProcessPickerBindingFree(binding);
        return NULL;
    }
    GtkWidget *picker = UmiGtk4ProcessPickerCreate(NULL, RuntimeProcessChoose, binding,
                                                   RuntimeProcessPickerBindingFree);
    if (picker == NULL)
        RuntimeProcessPickerBindingFree(binding);
    return picker;
}
static UmiStatus RuntimeProcessSelectionValidate(GtkWidget *pid,
                                                 const UmiStudioWorkspaceSnapshot *workspace,
                                                 uint64_t process_id)
{
    const StudioProcessSelection *selection =
        g_object_get_data(G_OBJECT(pid), "umicom-selected-process");
    if (selection == NULL)
        return UMI_STATUS_OK;
    if (selection->workspace_revision != workspace->graph.revision ||
        !umi_path_equal(selection->root, workspace->graph.root) ||
        selection->process.pid != process_id)
        return UMI_STATUS_INVALID_STATE;
    /* PID numbers can be reused. Re-read the captured creation identity just
     * before the explicit attach request; the OS can still change after this check. */
    return UmiDesktopProcessValidateCurrent(&selection->process);
}
