/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/debug.inc
 *
 * PURPOSE:
 *   Build the Framework-backed debug presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_debug(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Run and Debug");
    char value[96];

    if (panel == NULL || runtime == NULL) return panel;

    runtime_panel_add_stat(
        panel,
        "Controller",
        runtime->debugger_snapshot.controller_state);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->debugger_snapshot.breakpoint_count);
    runtime_panel_add_stat(panel, "Breakpoints", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->debugger_snapshot.thread_count);
    runtime_panel_add_stat(panel, "Threads", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->debugger_snapshot.stack_frame_count);
    runtime_panel_add_stat(panel, "Stack frames", value);

    {
        static const char *const labels[] = {"Start", "Continue", "Stop"};
        static const char *const commands[] = {
            "debug.start", "debug.continue", "debug.stop"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }

    return runtime_scroll(panel);
}

