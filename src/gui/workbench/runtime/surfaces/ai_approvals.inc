/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_approvals.inc
 *
 * PURPOSE:
 *   Build the Framework-backed ai approvals presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_approvals(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Approvals");
    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Human approval boundary",
            "Sensitive tool calls and governed patches require explicit "
            "Framework approval before mutation."));

    {
        static const char *const labels[] = {"Approvals", "Approve", "Reject"};
        static const char *const commands[] = {
            "ai.developer.open-approvals",
            "ai.developer.approval-approve",
            "ai.developer.approval-reject"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 3U);
    }
    return runtime_scroll(panel);
}

