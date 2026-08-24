/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/source_control.inc
 *
 * PURPOSE:
 *   Build the Framework-backed source control presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_source_control(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Source Control");
    char value[128];

    if (panel == NULL || runtime == NULL) return panel;

    runtime_panel_add_stat(
        panel,
        "Provider",
        runtime->source_control_snapshot.available
            ? runtime->source_control_snapshot.provider_id
            : "Unavailable");
    runtime_panel_add_stat(
        panel,
        "Branch",
        runtime->source_control_snapshot.branch);

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->source_control_snapshot.changes);
    runtime_panel_add_stat(panel, "Changes", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->source_control_snapshot.staged);
    runtime_panel_add_stat(panel, "Staged", value);
    (void)snprintf(value, sizeof(value), "%d / %d",
                   runtime->source_control_snapshot.ahead,
                   runtime->source_control_snapshot.behind);
    runtime_panel_add_stat(panel, "Ahead / behind", value);

    {
        static const char *const labels[] = {"Refresh", "Pull", "Push"};
        static const char *const commands[] = {
            "source-control.refresh",
            "source-control.pull",
            "source-control.push"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }

    return runtime_scroll(panel);
}

