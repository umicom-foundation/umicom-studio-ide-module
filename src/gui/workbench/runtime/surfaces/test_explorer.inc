/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/test_explorer.inc
 *
 * PURPOSE:
 *   Build the Framework-backed test explorer presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_test_explorer(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Test Explorer");
    char value[128];

    if (panel == NULL || runtime == NULL) return panel;

    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->test_snapshot.test_count);
    runtime_panel_add_stat(panel, "Tests", value);
    (void)snprintf(value, sizeof(value), "%zu / %zu",
                   runtime->test_snapshot.passed,
                   runtime->test_snapshot.failed);
    runtime_panel_add_stat(panel, "Passed / failed", value);
    runtime_panel_add_stat(
        panel,
        "Operation",
        runtime->test_snapshot.operation_running ? "Running" : "Idle");

    {
        static const char *const labels[] = {"Run All", "Failed", "Stop"};
        static const char *const commands[] = {
            "test.run-all", "test.rerun-failed", "test.stop"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }

    return runtime_scroll(panel);
}

