/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/explorer.inc
 *
 * PURPOSE:
 *   Build the Framework-backed explorer presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_explorer(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Explorer");
    char value[64];

    if (panel == NULL || runtime == NULL) return panel;

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.workspace_files);
    runtime_panel_add_stat(panel, "Workspace files", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.open_documents);
    runtime_panel_add_stat(panel, "Open documents", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.dirty_documents);
    runtime_panel_add_stat(panel, "Dirty documents", value);

    {
        static const char *const labels[] = {"New", "Open", "Save"};
        static const char *const commands[] = {
            "project.new", "file.open", "file.save"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }

    gtk_box_append(GTK_BOX(panel),
                   runtime_card_new(
                       "Framework Explorer",
                       "The existing Framework GTK4 adapter remains the detailed "
                       "file-tree/editor renderer in the centre workspace."));
    return runtime_scroll(panel);
}

