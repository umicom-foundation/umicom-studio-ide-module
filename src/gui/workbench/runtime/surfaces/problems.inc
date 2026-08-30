/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/problems.inc
 *
 * PURPOSE:
 *   Build the operational Problems surface over retained Framework diagnostics
 *   and existing next/previous problem navigation commands.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_problems(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Problems");
    GtkWidget *actions;
    GtkWidget *open;
    GtkWidget *previous;
    GtkWidget *next;
    GtkWidget *clear;

    if (panel == NULL || runtime == NULL) return panel;

    runtime->problems_summary_label = runtime_label("", 0.0F);
    runtime->problems_list = gtk_list_box_new();
    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    open = runtime_make_command_button(
        runtime,
        "Open",
        "view.problems",
        "Focus the Framework Problems surface");
    previous = runtime_make_command_button(
        runtime,
        "Previous",
        "navigate.problem.previous",
        "Navigate to the previous problem");
    next = runtime_make_command_button(
        runtime,
        "Next",
        "navigate.problem.next",
        "Navigate to the next problem");
    clear = gtk_button_new_with_label("Clear");

    if (runtime->problems_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->problems_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->problems_summary_label);
    }

    if (actions != NULL && open != NULL &&
        previous != NULL && next != NULL && clear != NULL) {
        g_signal_connect(
            clear,
            "clicked",
            G_CALLBACK(runtime_on_problems_clear),
            runtime);
        gtk_box_append(GTK_BOX(actions), open);
        gtk_box_append(GTK_BOX(actions), previous);
        gtk_box_append(GTK_BOX(actions), next);
        gtk_box_append(GTK_BOX(actions), clear);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    if (runtime->problems_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->problems_list),
            GTK_SELECTION_NONE);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->problems_list));
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Cross-navigation",
            "Previous and Next remain routed through the Framework problem "
            "navigation commands so a navigable diagnostic can focus its "
            "source document without GTK owning navigation state."));

    runtime_append_context_actions(runtime, panel, "problems");

    runtime_problems_refresh_list(runtime);
    return panel;
}
