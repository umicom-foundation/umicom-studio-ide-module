/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/terminal.inc
 *
 * PURPOSE:
 *   Build the Framework-backed terminal presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_terminal(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Terminal");
    char value[64];

    if (panel == NULL || runtime == NULL) return panel;

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->terminal_snapshot.session_count);
    runtime_panel_add_stat(panel, "Prepared sessions", value);

    {
        static const char *const labels[] = {"Terminal", "New"};
        static const char *const commands[] = {
            "view.terminal", "terminal.new"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 2U);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Framework Terminal",
            "Terminal process execution and transcript ownership remain in "
            "the reusable Framework terminal services."));
    return runtime_scroll(panel);
}

