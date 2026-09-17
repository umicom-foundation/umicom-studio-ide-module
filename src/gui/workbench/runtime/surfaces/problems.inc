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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    runtime->problems_summary_label = runtime_label("", 0.0F);
    runtime->problems_list = gtk_list_box_new();
    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    open = gtk_button_new_with_label("Open");
    gtk_widget_set_tooltip_text(open, "Open the selected problem's source file without overwriting unsaved text");
    g_signal_connect(open, "clicked", G_CALLBACK(RuntimeProblemsOpenClicked), runtime);
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->problems_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->problems_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->problems_summary_label);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->problems_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->problems_list),
            GTK_SELECTION_SINGLE);
        g_signal_connect(runtime->problems_list, "row-selected",
            G_CALLBACK(RuntimeProblemsSelected), runtime);
        g_signal_connect(runtime->problems_list, "row-activated",
            G_CALLBACK(RuntimeProblemsActivated), runtime);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->problems_list));
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Cross-navigation",
            "Select a problem and choose Open, or activate its row. Previous and Next "
            "use the same Framework diagnostic model to visit source locations "
            "without saving or replacing an existing draft."));

    runtime_append_context_actions(runtime, panel, "problems");

    runtime_problems_refresh_list(runtime);
    return panel;
}
