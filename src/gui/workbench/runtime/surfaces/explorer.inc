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
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(
            (UmiStudioGtkRuntimeChrome *)user_data,
            umi_status_text(status));
    }
}

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
    char value[64];

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

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    new_file = gtk_button_new_with_label("New File");
    open_file = runtime_make_command_button(
        runtime,
        "Open",
        "file.open",
        "Open a file into the Framework DocumentCoordinator");
    open_folder = runtime_make_command_button(
        runtime,
        "Open Folder",
        "workspace.open-folder",
        "Open and index a workspace folder");
    save = runtime_make_command_button(
        runtime,
        "Save",
        "file.save",
        "Save the active Framework document");
    refresh = gtk_button_new_with_label("Refresh");

    if (runtime->explorer_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->explorer_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->explorer_summary_label);
    }

    if (actions != NULL && new_file != NULL &&
        open_file != NULL && open_folder != NULL &&
        save != NULL && refresh != NULL) {
        g_signal_connect(
            new_file,
            "clicked",
            G_CALLBACK(runtime_on_explorer_new_file),
            runtime);
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

    if (runtime->explorer_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->explorer_list),
            GTK_SELECTION_NONE);
        scroll = runtime_scroll(runtime->explorer_list);
        if (scroll != NULL) {
            gtk_box_append(GTK_BOX(panel), scroll);
        }
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Framework Explorer",
            "The Framework File Index owns workspace discovery and the "
            "DocumentCoordinator owns file loading. This GTK surface only "
            "renders and dispatches those services."));

    runtime_append_context_actions(runtime, panel, "workspace");

    runtime_explorer_refresh_list(runtime);
    return panel;
}
