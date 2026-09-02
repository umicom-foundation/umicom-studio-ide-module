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
    GtkWidget *scroll;
    GtkWidget *about;
    char value[64];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    runtime->explorer_summary_label = runtime_label("", 0.0F);
    runtime->explorer_search_entry = gtk_search_entry_new();

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
        save != NULL && refresh != NULL) {
        g_signal_connect(
            new_file,
            "clicked",
            G_CALLBACK(runtime_on_explorer_new_file),
            runtime);
        gtk_widget_set_tooltip_text(new_file, "Create a new source file");
        gtk_widget_set_tooltip_text(refresh, "Refresh the workspace index");
        gtk_widget_add_css_class(actions, "umicom-runtime-panel-actions");
        g_signal_connect(
            refresh,
            "clicked",
            G_CALLBACK(runtime_on_explorer_refresh),
            runtime);

        gtk_box_append(GTK_BOX(actions), new_file);
        gtk_box_append(GTK_BOX(actions), open_file);
        gtk_box_append(GTK_BOX(actions), open_folder);
        gtk_box_append(GTK_BOX(actions), save);
        gtk_box_append(GTK_BOX(actions), refresh);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->explorer_search_entry != NULL) {
        g_object_set(G_OBJECT(runtime->explorer_search_entry), "placeholder-text", "Filter indexed files", NULL);
        g_signal_connect(
            runtime->explorer_search_entry,
            "search-changed",
            G_CALLBACK(runtime_on_explorer_search_changed),
            runtime);
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
