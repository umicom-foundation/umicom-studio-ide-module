/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/problems.inc
 *
 * PURPOSE:
 *   Build the Framework-backed problems presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_problems(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Problems");
    char value[64];

    if (panel == NULL || runtime == NULL) return panel;

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.retained_diagnostics);
    runtime_panel_add_stat(panel, "Retained diagnostics", value);

    {
        static const char *const labels[] = {"Open", "Next", "Previous"};
        static const char *const commands[] = {
            "view.problems", "navigate.problem.next", "navigate.problem.previous"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Cross-navigation",
            "Problem navigation is routed through the Framework command and "
            "document integration layers; successful navigation focuses Editor."));
    return runtime_scroll(panel);
}

