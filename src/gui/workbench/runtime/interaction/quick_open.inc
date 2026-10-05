/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/quick_open.inc
 * PURPOSE: Connect Framework indexed-file selection to the current Studio workspace and document owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/quick_open.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "umicom/developer_workbench/quick_open_selection.h"
typedef struct RuntimeQuickOpenRequest
{
    UmiStudioFileDialogRequest *owner;
    UmiWorkspaceGraphSnapshot workspace;
    UmiFileIndex *index; /* Identity only; reacquire through the live host before use. */
} RuntimeQuickOpenRequest;
static void RuntimeQuickOpenFree(gpointer data)
{
    RuntimeQuickOpenRequest *request = data;
    runtime_file_dialog_request_free(request->owner);
    g_free(request);
}
static UmiStudioGtkRuntimeChrome *RuntimeQuickOpenOwner(RuntimeQuickOpenRequest *request, GObject **window)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(*window)))
        return NULL;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiWorkspaceGraphSnapshot current;
    /* Reject changing away and back as well as a different current root.
     * File-index revision alone cannot describe a replaced workspace owner. */
    if (umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &current) != UMI_STATUS_OK ||
        !current.open || current.revision != request->workspace.revision ||
        strcmp(current.root, request->workspace.root) != 0 ||
        umi_studio_services_file_index(services) != request->index)
        return NULL;
    return runtime;
}
static UmiStatus RuntimeQuickOpenRead(gpointer data, const char *query, size_t offset, uint64_t revision,
                                      UmiFileIndexEntry *entries, size_t capacity, UmiFileIndexPage *page)
{
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status =
        runtime == NULL
            ? UMI_STATUS_INVALID_STATE
            : UmiFileIndexReadPage(umi_studio_services_file_index(umi_studio_ui_services(runtime->owner->ui)),
                                   query, 0, offset, revision, entries, capacity, page);
    if (status == UMI_STATUS_OK && strcmp(page->stats.root, request->workspace.root) != 0)
        status = UMI_STATUS_BUSY;
    g_clear_object(&window);
    return status;
}
static UmiStatus RuntimeQuickOpenSelected(gpointer data, const char *query, const UmiFileIndexPage *page,
                                          size_t position, const UmiFileIndexEntry *entry)
{
    RuntimeQuickOpenRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeQuickOpenOwner(request, &window);
    UmiStatus status = runtime == NULL
                           ? UMI_STATUS_INVALID_STATE
                           : UmiQuickOpenCheckSelection(
                                 umi_studio_services_file_index(umi_studio_ui_services(runtime->owner->ui)),
                                 query, page, position, entry);
    char view[UMI_UI_ID_CAPACITY];
    if (status == UMI_STATUS_OK)
        status = umi_document_coordinator_open(umi_studio_ui_documents(runtime->owner->ui), entry->path, view,
                                               sizeof(view));
    /* Opening may notify observers which close or replace the host. Resolve
     * its weak binding again before activating a view or refreshing chrome. */
    g_clear_object(&window);
    runtime = RuntimeQuickOpenOwner(request, &window);
    if (runtime == NULL)
        status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK)
    {
        status = umi_ui_workbench_activate_document(umi_studio_ui_workbench(runtime->owner->ui), view);
        g_clear_object(&window);
        runtime = RuntimeQuickOpenOwner(request, &window);
        if (runtime != NULL)
            (void)runtime_refresh_services(runtime);
    }
    g_clear_object(&window);
    return status;
}
static void RuntimeQuickOpenWindowFree(gpointer data)
{
    GtkWindow *window = data;
    gtk_window_destroy(window);
    g_object_unref(window);
}
static void RuntimeShowQuickOpen(UmiStudioGtkRuntimeChrome *runtime)
{
    RuntimeQuickOpenRequest *request = g_new0(RuntimeQuickOpenRequest, 1);
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &request->workspace) !=
            UMI_STATUS_OK ||
        !request->workspace.open || !umi_path_is_absolute(request->workspace.root))
    {
        g_free(request);
        runtime_show_notification(
            runtime, "Open a workspace before using Quick Open. Explorer Refresh discovers its files.");
        return;
    }
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    request->index = umi_studio_services_file_index(services);
    if (request->owner == NULL)
    {
        RuntimeQuickOpenFree(request);
        return;
    }
    GtkWindow *dialog = NULL;
    GObject *window = G_OBJECT(g_object_ref(runtime->owner->window));
    UmiStatus status =
        UmiGtk4QuickOpenCreate(GTK_WINDOW(window), RuntimeQuickOpenRead, RuntimeQuickOpenSelected, request,
                               RuntimeQuickOpenFree, &dialog);
    if (status != UMI_STATUS_OK)
        RuntimeQuickOpenFree(request);
    else
    {
        /* One picker per owner; opening again captures the current workspace.
         * The parent holds the returned reference, not a raw transient pointer. */
        g_object_set_data_full(window, "umicom-studio-quick-open-window", g_object_ref(dialog),
                               RuntimeQuickOpenWindowFree);
        if (UmiGtk4WindowIsOpen(GTK_WINDOW(window)) && UmiGtk4WindowIsOpen(dialog))
            gtk_window_present(dialog);
        g_object_unref(dialog);
    }
    g_object_unref(window);
}
static void RuntimeQuickOpenClicked(GtkButton *button, gpointer data)
{
    (void)button;
    RuntimeShowQuickOpen(data);
}
