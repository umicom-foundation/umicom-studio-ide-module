/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/output.inc
 *
 * PURPOSE:
 *   Build the Framework-backed output presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_output(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Output");
    char value[64];

    if (panel == NULL || runtime == NULL) return panel;

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.queued_tasks);
    runtime_panel_add_stat(panel, "Queued tasks", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.running_tasks);
    runtime_panel_add_stat(panel, "Running tasks", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.retained_diagnostics);
    runtime_panel_add_stat(panel, "Diagnostics", value);

    return runtime_scroll(panel);
}

