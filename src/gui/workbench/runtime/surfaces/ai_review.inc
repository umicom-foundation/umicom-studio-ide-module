/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_review.inc
 *
 * PURPOSE:
 *   Build the Framework-backed ai review presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_review(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Patch Review");
    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Governed review",
            "Complete-file AI changes are reviewed before apply. Existing "
            "comments and unrelated functionality remain protected by policy."));

    {
        static const char *const labels[] = {"Open Review", "Apply", "Reject"};
        static const char *const commands[] = {
            "ide.ai.open-review",
            "ai.developer.patch-apply",
            "ai.developer.patch-reject"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }
    return runtime_scroll(panel);
}

