/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_validation.inc
 *
 * PURPOSE:
 *   Build the Framework-backed ai validation presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_validation(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Validation");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Validation evidence",
            "AI-generated changes are expected to pass the same build/test "
            "workflow gates as human-authored changes."));

    {
        static const char *const labels[] = {"Validation", "Build", "Tests"};
        static const char *const commands[] = {
            "ai.developer.open-validation",
            "build.build",
            "test.run-all"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }
    return runtime_scroll(panel);
}

