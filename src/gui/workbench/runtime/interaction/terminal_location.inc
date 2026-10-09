/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/terminal_location.inc
 * PURPOSE: Collect explicit terminal folders and reject stale chooser completions.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/terminal/location.h"

typedef struct UmiStudioTerminalFolderRequest
{
    UmiStudioFileDialogRequest *owner;
    GCancellable *cancel;
    char session_id[UMI_TERMINAL_ID_CAPACITY];
    uint64_t revision;
} UmiStudioTerminalFolderRequest;

/* Entry revisions reject a chooser answer even if a draft changes and later
 * returns to the same text. Session selection has a separate identity check. */
static void RuntimeTerminalFolderChanged(GtkEditable *entry, gpointer context)
{
    (void)entry;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL && runtime->terminal_directory_revision != UINT64_MAX)
        ++runtime->terminal_directory_revision;
    g_clear_object(&window);
}
static void RuntimeTerminalFolderApply(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL && runtime->terminal_directory_entry != NULL)
    {
        char *path =
            g_strdup(gtk_editable_get_text(GTK_EDITABLE(runtime->terminal_directory_entry)));
        (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_TERMINAL_DIRECTORY,
                                                 path);
        g_free(path);
    }
    g_clear_object(&window);
}
static void RuntimeTerminalProjectFolder(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL && runtime->terminal_directory_entry != NULL)
    {
        UmiStudioWorkspaceSnapshot workspace;
        if (umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &workspace) ==
                UMI_STATUS_OK &&
            workspace.graph.open)
        {
            /* This fills the field only. Apply is the explicit session change. */
            gtk_editable_set_text(GTK_EDITABLE(runtime->terminal_directory_entry),
                                  workspace.graph.root);
        }
        else
            runtime_show_notification(runtime, "Open a project before choosing its folder.");
    }
    g_clear_object(&window);
}
static void RuntimeTerminalFolderUnmap(GtkWidget *window, gpointer context)
{
    (void)window;
    g_cancellable_cancel(G_CANCELLABLE(context));
}
static void RuntimeTerminalFolderComplete(GObject *source, GAsyncResult *result, gpointer context)
{
    UmiStudioTerminalFolderRequest *request = context;
    GError *error = NULL;
    GFile *folder = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source), result, &error);
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
    if (runtime != NULL)
    {
        runtime->terminal_directory_choosing = 0;
        UmiTerminalSession *session =
            umi_terminal_controller_active_session(RuntimeTerminalController(runtime));
        UmiTerminalSessionSnapshot current;
        int applicable = !g_cancellable_is_cancelled(request->cancel) &&
                         runtime->terminal_directory_revision == request->revision &&
                         runtime->terminal_directory_revision != UINT64_MAX &&
                         umi_terminal_session_snapshot(session, &current) == UMI_STATUS_OK &&
                         strcmp(current.session_id, request->session_id) == 0;
        if (applicable && folder != NULL && runtime->terminal_directory_entry != NULL)
        {
            char *path = g_file_get_path(folder);
            if (path != NULL)
                gtk_editable_set_text(GTK_EDITABLE(runtime->terminal_directory_entry), path);
            g_free(path);
        }
        else if (applicable && error != NULL && !runtime_file_dialog_cancelled(error))
            runtime_show_notification(runtime, error->message);
    }
    g_clear_object(&window);
    g_clear_object(&folder);
    g_clear_error(&error);
    g_clear_object(&request->cancel);
    runtime_file_dialog_request_free(request->owner);
    g_free(request);
}
static void RuntimeTerminalFolderChoose(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime == NULL || runtime->terminal_directory_choosing ||
        runtime->terminal_directory_revision == UINT64_MAX)
    {
        g_clear_object(&window);
        return;
    }
    UmiTerminalSessionSnapshot current;
    UmiTerminalSession *session =
        umi_terminal_controller_active_session(RuntimeTerminalController(runtime));
    if (umi_terminal_session_snapshot(session, &current) != UMI_STATUS_OK)
    {
        g_clear_object(&window);
        return;
    }
    UmiStudioTerminalFolderRequest *request = g_try_new0(UmiStudioTerminalFolderRequest, 1);
    if (request == NULL)
    {
        g_clear_object(&window);
        return;
    }
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (request->owner == NULL)
    {
        g_free(request);
        g_clear_object(&window);
        return;
    }
    request->cancel = g_cancellable_new();
    memcpy(request->session_id, current.session_id, strlen(current.session_id) + 1U);
    request->revision = runtime->terminal_directory_revision;
    runtime->terminal_directory_choosing = 1;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Choose the terminal folder");
    /* Cancellation owns no runtime pointer. Closing the window releases or
     * invalidates its weak binding before a late native response can publish. */
    g_signal_connect_object(window, "unmap", G_CALLBACK(RuntimeTerminalFolderUnmap),
                            request->cancel, 0);
    gtk_file_dialog_select_folder(dialog, GTK_WINDOW(window), request->cancel,
                                  RuntimeTerminalFolderComplete, request);
    g_object_unref(dialog);
    g_clear_object(&window);
}
static void RuntimeTerminalFolderControls(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *label = gtk_label_new("Folder");
    runtime->terminal_directory_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->terminal_directory_entry),
                                   "Choose an existing absolute folder");
    gtk_widget_set_hexpand(runtime->terminal_directory_entry, TRUE);
    (void)umi_gtk4_automation_tag_widget(runtime->terminal_directory_entry,
                                         "studio.terminal.folder");
    RuntimeTerminalBind(runtime->terminal_directory_entry, "changed",
                        G_CALLBACK(RuntimeTerminalFolderChanged), runtime);
    gtk_box_append(GTK_BOX(row), label);
    gtk_box_append(GTK_BOX(row), runtime->terminal_directory_entry);
    const char *labels[] = {"Browse…", "Project folder", "Apply folder"};
    const char *tags[] = {"studio.terminal.browse", "studio.terminal.project-folder",
                          "studio.terminal.apply-folder"};
    GCallback actions[] = {G_CALLBACK(RuntimeTerminalFolderChoose),
                           G_CALLBACK(RuntimeTerminalProjectFolder),
                           G_CALLBACK(RuntimeTerminalFolderApply)};
    for (size_t index = 0U; index < 3U; ++index)
    {
        GtkWidget *button = gtk_button_new_with_label(labels[index]);
        (void)umi_gtk4_automation_tag_widget(button, tags[index]);
        RuntimeTerminalBind(button, "clicked", actions[index], runtime);
        gtk_box_append(GTK_BOX(row), button);
    }
    gtk_box_append(GTK_BOX(panel), row);
}
