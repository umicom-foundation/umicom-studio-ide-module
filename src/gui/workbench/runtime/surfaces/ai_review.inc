/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_review.inc
 *
 * PURPOSE:
 *   Build the governed AI patch review/action surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_review(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Patch Review");
    GtkWidget *actions;
    GtkWidget *approve;
    GtkWidget *apply;
    GtkWidget *revert;

    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Human review boundary",
            "Framework retains patch creation, conflict detection, approval, "
            "application and rollback state. Studio only asks for a stable patch "
            "identifier and dispatches the governed command."));

    {
        static const char *const labels[] = {
            "Open Review",
            "Reject"
        };
        static const char *const commands[] = {
            "ide.ai.open-review",
            "ai.developer.patch-reject"
        };
        runtime_panel_add_commands(
            runtime,
            panel,
            labels,
            commands,
            2U);
    }

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    approve = gtk_button_new_with_label("Approve Patch");
    apply = gtk_button_new_with_label("Apply Patch");
    revert = gtk_button_new_with_label("Revert Patch");

    if (actions != NULL && approve != NULL &&
        apply != NULL && revert != NULL) {
        g_signal_connect(
            approve, "clicked",
            G_CALLBACK(runtime_on_ai_patch_approve), runtime);
        g_signal_connect(
            apply, "clicked",
            G_CALLBACK(runtime_on_ai_patch_apply), runtime);
        g_signal_connect(
            revert, "clicked",
            G_CALLBACK(runtime_on_ai_patch_revert), runtime);

        gtk_box_append(GTK_BOX(actions), approve);
        gtk_box_append(GTK_BOX(actions), apply);
        gtk_box_append(GTK_BOX(actions), revert);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    runtime_append_context_actions(runtime, panel, "ai");

    return runtime_scroll(panel);
}
