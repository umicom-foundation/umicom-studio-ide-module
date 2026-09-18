/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/explorer.inc
 *
 * PURPOSE:
 *   Build the operational Framework-backed Explorer surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_explorer_new_file(
    GtkButton *button,
    gpointer user_data)
{
    UmiStatus status;
    (void)button;

    status = runtime_document_new(
        (UmiStudioGtkRuntimeChrome *)user_data);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(
            (UmiStudioGtkRuntimeChrome *)user_data,
            umi_status_text(status));
    }
}

static void RuntimeExplorerUntitledInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ runtime_on_explorer_new_file(GTK_BUTTON(widget), runtime); }

/*
 * Provide the runtime build surface explorer operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_build_surface_explorer(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Explorer");
    GtkWidget *actions;
    GtkWidget *new_file;
    GtkWidget *open_file;
    GtkWidget *open_folder;
    GtkWidget *save;
    GtkWidget *refresh;
    GtkWidget *cancelRefresh;
    GtkWidget *scroll;
    GtkWidget *about;
    char value[64];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    runtime->explorer_rendered = 0;
    runtime->explorer_offset = 0U;
    runtime->explorer_summary_label = runtime_label("", 0.0F);
    runtime->explorer_search_entry = gtk_search_entry_new();
    runtime->explorer_refresh_status = NULL;
    runtime->explorer_refresh_button = NULL;
    runtime->explorer_cancel_refresh = NULL;

    (void)snprintf(
        value,
        sizeof(value),
        "%zu",
        runtime->view_model_snapshot.open_documents);
    runtime_panel_add_stat(panel, "Open documents", value);
    (void)snprintf(
        value,
        sizeof(value),
        "%zu",
        runtime->view_model_snapshot.dirty_documents);
    runtime_panel_add_stat(panel, "Dirty documents", value);
    runtime->explorer_list = gtk_list_box_new();
    RuntimeExplorerListOwner *listOwner = g_new0(RuntimeExplorerListOwner, 1);
    listOwner->owner = runtime_file_dialog_request_new(runtime, 0U);
    listOwner->list = runtime->explorer_list;
    if (listOwner->owner == NULL) {
        g_free(listOwner);
        g_object_ref_sink(runtime->explorer_list); g_object_unref(runtime->explorer_list);
        g_object_ref_sink(runtime->explorer_search_entry); g_object_unref(runtime->explorer_search_entry);
        g_object_ref_sink(runtime->explorer_summary_label); g_object_unref(runtime->explorer_summary_label);
        runtime->explorer_list = NULL;
        runtime->explorer_search_entry = NULL;
        runtime->explorer_summary_label = NULL;
        gtk_box_append(GTK_BOX(panel), runtime_label("Explorer controls could not be allocated. Close this panel and reopen it.", 0.0F));
        return panel;
    }
    g_object_set_data_full(G_OBJECT(runtime->explorer_list), "umicom-explorer-list-owner",
        listOwner, RuntimeExplorerListReleased);
    g_object_set_data(G_OBJECT(runtime->explorer_list), "umicom-automation-id", "umicom-explorer-list");
    g_object_set_data(G_OBJECT(runtime->explorer_summary_label), "umicom-automation-id", "umicom-explorer-summary");

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    new_file = gtk_button_new_from_icon_name("document-new-symbolic");
    open_file = runtime_make_icon_command_button(
        runtime,
        "document-open-symbolic",
        "file.open",
        "Open a file into the Framework DocumentCoordinator");
    open_folder = runtime_make_icon_command_button(
        runtime,
        "folder-open-symbolic",
        "workspace.open-folder",
        "Open and index a workspace folder");
    save = runtime_make_icon_command_button(
        runtime,
        "document-save-symbolic",
        "file.save",
        "Save the active Framework document");
    refresh = gtk_button_new_from_icon_name("view-refresh-symbolic");
    cancelRefresh = gtk_button_new_from_icon_name("process-stop-symbolic");
    runtime->explorer_refresh_button = refresh;
    runtime->explorer_cancel_refresh = cancelRefresh;
    runtime->explorer_refresh_status = runtime_label("", 0.0F);
    gtk_label_set_wrap(GTK_LABEL(runtime->explorer_refresh_status), TRUE);
    g_object_set_data(G_OBJECT(runtime->explorer_refresh_status), "umicom-automation-id",
        "umicom-explorer-refresh-status");
    gtk_box_append(GTK_BOX(panel), runtime->explorer_refresh_status);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->explorer_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->explorer_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->explorer_summary_label);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (actions != NULL && new_file != NULL &&
        open_file != NULL && open_folder != NULL &&
        save != NULL && refresh != NULL && cancelRefresh != NULL) {
        RuntimeExplorerBindControl(runtime, new_file, "clicked", "umicom-explorer-untitled",
            RuntimeExplorerUntitledInvoked);
        gtk_widget_set_tooltip_text(new_file, "Create a new source file");
        gtk_widget_set_tooltip_text(refresh, "Refresh project files in the background; keep editing while the scan runs");
        gtk_widget_set_tooltip_text(cancelRefresh, "Cancel file-list refresh without cancelling a build or changing your documents");
        gtk_widget_set_sensitive(cancelRefresh, FALSE);
        gtk_widget_add_css_class(actions, "umicom-runtime-panel-actions");
        RuntimeExplorerBindControl(runtime, refresh, "clicked", "umicom-explorer-refresh",
            RuntimeExplorerRefreshInvoked);
        RuntimeExplorerBindControl(runtime, cancelRefresh, "clicked", "umicom-explorer-cancel-refresh",
            RuntimeExplorerCancelRefreshInvoked);

        gtk_box_append(GTK_BOX(actions), new_file);
        gtk_box_append(GTK_BOX(actions), open_file);
        gtk_box_append(GTK_BOX(actions), open_folder);
        gtk_box_append(GTK_BOX(actions), save);
        gtk_box_append(GTK_BOX(actions), refresh);
        gtk_box_append(GTK_BOX(actions), cancelRefresh);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->explorer_search_entry != NULL) {
        g_object_set(G_OBJECT(runtime->explorer_search_entry), "placeholder-text", "Filter indexed files", NULL);
        gtk_editable_set_text(GTK_EDITABLE(runtime->explorer_search_entry), runtime->explorer_query);
        RuntimeExplorerBindControl(runtime, runtime->explorer_search_entry,
            "search-changed", "umicom-explorer-filter", RuntimeExplorerSearchInvoked);
        gtk_box_append(
            GTK_BOX(panel),
            runtime->explorer_search_entry);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->explorer_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->explorer_list),
            GTK_SELECTION_NONE);
        scroll = runtime_scroll(runtime->explorer_list);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (scroll != NULL) {
            gtk_box_append(GTK_BOX(panel), scroll);
        }
    }

    /* Project entries are separate from File > New's unsaved document. */
    GtkWidget *projectActions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *projectFile = gtk_button_new_with_label("New Project File");
    GtkWidget *projectFolder = gtk_button_new_with_label("New Folder");
    gtk_widget_set_name(projectFile, "umicom-explorer-create-file");
    gtk_widget_set_name(projectFolder, "umicom-explorer-create-folder");
    gtk_widget_set_tooltip_text(projectFile, "Create an empty file in the trusted workspace without overwriting existing work");
    gtk_widget_set_tooltip_text(projectFolder, "Create one folder in the trusted workspace; the parent must already exist");
    RuntimeExplorerBindControl(runtime, projectFile, "clicked", "umicom-explorer-create-file", RuntimeExplorerNewFileInvoked);
    RuntimeExplorerBindControl(runtime, projectFolder, "clicked", "umicom-explorer-create-folder", RuntimeExplorerNewFolderInvoked);
    gtk_box_append(GTK_BOX(projectActions), projectFile);
    gtk_box_append(GTK_BOX(projectActions), projectFolder);
    GtkWidget *projectScroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(projectScroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(projectScroll), projectActions);
    gtk_box_append(GTK_BOX(panel), projectScroll);
    GtkWidget *pages = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    runtime->explorer_previous = gtk_button_new_with_label("Previous files");
    runtime->explorer_next = gtk_button_new_with_label("Next files");
    gtk_widget_set_name(runtime->explorer_previous, "umicom-explorer-previous");
    gtk_widget_set_name(runtime->explorer_next, "umicom-explorer-next");
    gtk_widget_set_tooltip_text(runtime->explorer_previous, "Show the previous page of matching files");
    gtk_widget_set_tooltip_text(runtime->explorer_next, "Show the next page of matching files");
    RuntimeExplorerBindControl(runtime, runtime->explorer_previous, "clicked", "umicom-explorer-previous", RuntimeExplorerPreviousInvoked);
    RuntimeExplorerBindControl(runtime, runtime->explorer_next, "clicked", "umicom-explorer-next", RuntimeExplorerNextInvoked);
    gtk_box_append(GTK_BOX(pages), runtime->explorer_previous);
    gtk_box_append(GTK_BOX(pages), runtime->explorer_next);
    GtkWidget *pageScroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pageScroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(pageScroll), pages);
    gtk_box_append(GTK_BOX(panel), pageScroll);

    about = gtk_expander_new("About Explorer");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (about != NULL) {
        gtk_expander_set_child(
            GTK_EXPANDER(about),
            runtime_card_new(
                "Framework Explorer",
                "Framework File Index discovers files and DocumentCoordinator "
                "loads them. Studio only presents those reusable services."));
        gtk_expander_set_expanded(GTK_EXPANDER(about), FALSE);
        gtk_box_append(GTK_BOX(panel), about);
    }

    runtime_append_context_actions(runtime, panel, "workspace");

    runtime_explorer_refresh_list(runtime);
    return panel;
}
