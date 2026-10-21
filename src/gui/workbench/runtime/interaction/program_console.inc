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
/* Console authority must belong to the current build profile as well as the captured workspace. The previous workspace-only check is retained for review. */
#if 0
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

#endif

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
                  umi_path_equal(workspace.graph.root, binding->root) &&
                   UmiBuildProfileCheckProject(umi_studio_build_service_profile(
                       umi_studio_services_build(services)), binding->root) == UMI_STATUS_OK;
    }
    g_clear_object(&window);
    return allowed;
}
/* Console execution now shares canonical project admission and excludes pending document decisions. Retain the former local guard for review. */
#if 0
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

#endif

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
        status = UmiStudioBuildCheckAdmission(services, NULL, 0U);
        if (status != UMI_STATUS_OK) { g_clear_object(&window); return status; }
        if (UmiGtk4AdapterReplacementReviewBusy(runtime->owner->adapter) ||
            UmiGtk4AdapterDocumentProposalBusy(runtime->owner->adapter) ||
            UmiGtk4AdapterCompletionReviewBusy(runtime->owner->adapter) ||
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
/* The captured request and creation reservation below prevent orphan consoles and duplicate native launches. The original ownership sequence remains for review. */
#if 0
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

#endif

/* Native creation and presentation can retire or reenter the host. Keep a
 * separate weak request and native references until publication is complete;
 * the Framework panel owns its binding only after successful creation. */
static void RuntimeOpenProgramConsole(GtkButton *button, gpointer data)
{
    (void)button;
    GObject *ownerWindow = NULL, *checkedWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &ownerWindow);
    UmiStudioFileDialogRequest *request = NULL;
    GtkWidget *panel = NULL;
    GtkWindow *window = NULL;
    int reserved = 0, published = 0;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(ownerWindow))) goto done;
    if (g_object_get_data(ownerWindow, "studio-program-console-creating") != NULL) goto done;
    GWeakRef *existing = g_object_get_data(ownerWindow, "umicom-studio-program-console-window");
    GtkWindow *existingWindow = existing != NULL ? g_weak_ref_get(existing) : NULL;
    if (existingWindow != NULL && UmiGtk4WindowIsOpen(existingWindow)) {
        gtk_window_present(existingWindow);
        g_object_unref(existingWindow);
        goto done;
    }
    g_clear_object(&existingWindow);
    if (RuntimeProgramConsoleBusy(runtime)) {
        runtime_show_notification(runtime, "The previous program console is still stopping.");
        goto done;
    }
    UmiStudioWorkspaceSnapshot workspace;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    char message[512];
    UmiStatus status = UmiStudioBuildCheckAdmission(services, message, sizeof message);
    if (status != UMI_STATUS_OK) { runtime_show_notification(runtime, message); goto done; }
    if (umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK) goto done;
    request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) goto done;
    StudioConsoleBinding *binding = g_try_new0(StudioConsoleBinding, 1U);
    if (binding == NULL) goto done;
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL) { g_free(binding); goto done; }
    binding->workspace_revision = workspace.graph.revision;
    memcpy(binding->root, workspace.graph.root, strlen(workspace.graph.root) + 1U);
    g_object_set_data(ownerWindow, "studio-program-console-creating", GINT_TO_POINTER(1));
    reserved = 1;
    UmiGtk4ProgramConsoleCallbacks callbacks = {RuntimeProgramConsolePrepare,
        RuntimeProgramConsoleAuthorized, binding, RuntimeProgramConsoleBindingFree};
    status = UmiGtk4ProgramConsolePanelCreate(&callbacks, &panel);
    if (status != UMI_STATUS_OK) { RuntimeProgramConsoleBindingFree(binding); goto done; }
    g_object_ref_sink(panel);
    window = GTK_WINDOW(gtk_window_new()); g_object_ref(window);
    runtime = runtime_file_dialog_owner(request, &checkedWindow);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(checkedWindow))) goto done;
    gtk_window_set_title(window, "Studio — Program Console");
    gtk_window_set_default_size(window, 860, 540);
    gtk_window_set_transient_for(window, GTK_WINDOW(ownerWindow));
    gtk_window_set_destroy_with_parent(window, TRUE);
    gtk_window_set_child(window, panel);
    g_clear_object(&checkedWindow);
    runtime = runtime_file_dialog_owner(request, &checkedWindow);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(checkedWindow)) || !UmiGtk4WindowIsOpen(window) ||
        !RuntimeProgramConsoleAuthorized(binding)) goto done;
    g_object_set_data_full(ownerWindow, "umicom-studio-program-console-panel", g_object_ref(panel), g_object_unref);
    g_clear_object(&checkedWindow);
    runtime = runtime_file_dialog_owner(request, &checkedWindow);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(checkedWindow))) goto done;
    GWeakRef *weak = g_new0(GWeakRef, 1U);
    g_weak_ref_init(weak, window);
    g_object_set_data_full(ownerWindow, "umicom-studio-program-console-window", weak, RuntimeProgramConsoleWeakFree);
    gtk_window_present(window);
    /* After presentation, a retired host cannot leave an orphan console open. */
    g_clear_object(&checkedWindow);
    runtime = runtime_file_dialog_owner(request, &checkedWindow);
    published = runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(checkedWindow)) && UmiGtk4WindowIsOpen(window);
done:
    if (!published && window != NULL) gtk_window_destroy(window);
    if (reserved) g_object_set_data(ownerWindow, "studio-program-console-creating", NULL);
    g_clear_object(&window);
    g_clear_object(&panel);
    g_clear_object(&checkedWindow);
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(request);
}

static void RuntimeProgramConsoleButton(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *button = gtk_button_new_with_label("Open project program console");
    (void)umi_gtk4_automation_tag_widget(button, "studio.terminal.program-console");
    RuntimeTerminalBind(button, "clicked", G_CALLBACK(RuntimeOpenProgramConsole), runtime);
    gtk_box_append(GTK_BOX(panel), button);
}
