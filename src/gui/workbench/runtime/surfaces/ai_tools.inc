/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_tools.inc
 *
 * PURPOSE:
 *   Build the Framework-backed ai tools presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_tools(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Tool Activity");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Controlled developer tools",
            "Build, test, filesystem and source-control tools are surfaced "
            "through Framework policy and audit boundaries."));

    {
        static const char *const labels[] = {"Tool Activity"};
        static const char *const commands[] = {
            "ai.developer.open-tools"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 1U);
    }
    return runtime_scroll(panel);
}

