/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/program_console.inc
 * PURPOSE: Connect Studio project identity and trust to the shared program console.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/build/program_console.h"
#include "umicom/ui/gtk4/program_console.h"
#include "umicom/ui/gtk4/window_lifecycle.h"

typedef struct StudioConsoleBinding
{
    UmiStudioFileDialogRequest *owner;
    uint64_t workspace_revision;
    char root[UMI_PATH_CAPACITY];
} StudioConsoleBinding;
static void RuntimeProgramConsoleBindingFree(gpointer data)
{
    StudioConsoleBinding *binding = data;
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
static bool RuntimeProgramConsoleAuthorized(void *data)
{
    StudioConsoleBinding *binding = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    bool allowed = false;
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
    {
        UmiStudioWorkspaceSnapshot workspace;
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        allowed = umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
                  workspace.graph.open && workspace.graph.trusted &&
                  workspace.graph.revision == binding->workspace_revision &&
                  umi_path_equal(workspace.graph.root, binding->root);
    }
    g_clear_object(&window);
    return allowed;
}
static UmiStatus RuntimeProgramConsolePrepare(UmiProgramConsole **out, void *data)
{
    StudioConsoleBinding *binding = data;
    *out = NULL;
    if (!RuntimeProgramConsoleAuthorized(data))
        return UMI_STATUS_PERMISSION_DENIED;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    UmiStatus status = UMI_STATUS_INVALID_STATE;
    if (runtime != NULL)
    {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        if (UmiStudioBuildBusy(umi_studio_services_build(services)) ||
            UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)) ||
            UmiGtk4AdapterDocumentSaveAllBusy(runtime->owner->adapter) ||
            UmiGtk4AdapterCloseDocumentsBusy(runtime->owner->adapter))
            status = UMI_STATUS_BUSY;
        else
            status = UmiBuildProgramConsoleCreate(
                umi_studio_build_service_profile(umi_studio_services_build(services)), true, out);
    }
    g_clear_object(&window);
    return status;
}
static void RuntimeProgramConsoleWeakFree(gpointer data)
{
    GWeakRef *weak = data;
    g_weak_ref_clear(weak);
    g_free(weak);
}
/* Keep the panel reference until the workbench closes or a later console replaces
 * it. A closed dialog may still be stopping its worker; it must continue to block
 * a conflicting build while native cleanup finishes. No worker owns Studio. */
static bool RuntimeProgramConsoleBusy(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL)
        return false;
    GtkWidget *panel =
        g_object_get_data(G_OBJECT(runtime->owner->window), "umicom-studio-program-console-panel");
    return panel != NULL && UmiGtk4ProgramConsolePanelPending(panel);
}
static void RuntimeOpenProgramConsole(GtkButton *button, gpointer data)
{
    (void)button;
    GObject *owner_window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &owner_window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(owner_window)))
        goto done;
    GWeakRef *existing = g_object_get_data(owner_window, "umicom-studio-program-console-window");
    GtkWindow *existing_window = existing != NULL ? g_weak_ref_get(existing) : NULL;
    if (existing_window != NULL && UmiGtk4WindowIsOpen(existing_window))
    {
        gtk_window_present(existing_window);
        g_object_unref(existing_window);
        goto done;
    }
    g_clear_object(&existing_window);
    if (RuntimeProgramConsoleBusy(runtime))
    {
        runtime_show_notification(runtime, "The previous program console is still stopping.");
        goto done;
    }
    UmiStudioWorkspaceSnapshot workspace;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
        !workspace.graph.open || !workspace.graph.trusted)
    {
        runtime_show_notification(runtime,
                                  "Open and trust the project, then review its Run settings.");
        goto done;
    }
    StudioConsoleBinding *binding = g_try_new0(StudioConsoleBinding, 1U);
    if (binding == NULL)
        goto done;
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        goto done;
    }
    binding->workspace_revision = workspace.graph.revision;
    memcpy(binding->root, workspace.graph.root, strlen(workspace.graph.root) + 1U);
    UmiGtk4ProgramConsoleCallbacks callbacks = {RuntimeProgramConsolePrepare,
                                                RuntimeProgramConsoleAuthorized, binding,
                                                RuntimeProgramConsoleBindingFree};
    GtkWidget *panel = NULL;
    UmiStatus status = UmiGtk4ProgramConsolePanelCreate(&callbacks, &panel);
    if (status != UMI_STATUS_OK)
    {
        RuntimeProgramConsoleBindingFree(binding);
        goto done;
    }
    GtkWindow *window = GTK_WINDOW(gtk_window_new());
    gtk_window_set_title(window, "Studio — Program Console");
    gtk_window_set_default_size(window, 860, 540);
    gtk_window_set_transient_for(window, GTK_WINDOW(owner_window));
    gtk_window_set_destroy_with_parent(window, TRUE);
    gtk_window_set_child(window, panel);
    g_object_set_data_full(owner_window, "umicom-studio-program-console-panel", g_object_ref(panel),
                           g_object_unref);
    GWeakRef *weak = g_new0(GWeakRef, 1U);
    g_weak_ref_init(weak, window);
    g_object_set_data_full(owner_window, "umicom-studio-program-console-window", weak,
                           RuntimeProgramConsoleWeakFree);
    gtk_window_present(window);
done:
    g_clear_object(&owner_window);
}
static void RuntimeProgramConsoleButton(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *button = gtk_button_new_with_label("Open project program console");
    (void)umi_gtk4_automation_tag_widget(button, "studio.terminal.program-console");
    RuntimeTerminalBind(button, "clicked", G_CALLBACK(RuntimeOpenProgramConsole), runtime);
    gtk_box_append(GTK_BOX(panel), button);
}
