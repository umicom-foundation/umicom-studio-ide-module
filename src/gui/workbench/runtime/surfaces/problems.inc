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

/*
 * The previous Problems surface is retained for review. The replacement keeps
 * every existing command and adds search, severity and sort controls that feed
 * Framework's diagnostics query service.
 */
#if 0
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
#endif

static GtkWidget *runtime_build_surface_problems(
    UmiStudioGtkRuntimeChrome *runtime)
{
    static const char *const severityOptions[] = {
        "All severities",
        "Warning and above",
        "Error and fatal",
        "Fatal only",
        NULL
    };
    static const char *const sortOptions[] = {
        "Newest",
        "Severity",
        "Oldest",
        NULL
    };
    GtkWidget *panel = runtime_panel_new("Problems");
    GtkWidget *actions;
    GtkWidget *open;
    GtkWidget *previous;
    GtkWidget *next;
    GtkWidget *clear;
    GtkWidget *filters;
    GtkWidget *search;
    GtkWidget *severity;
    GtkWidget *sort;
    GtkWidget *resolved;

    if (panel == NULL || runtime == NULL) return panel;

    runtime->problems_summary_label = runtime_label("", 0.0F);
    runtime->problems_list = gtk_list_box_new();

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    open = gtk_button_new_with_label("Open");
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

    if (open != NULL) {
        gtk_widget_set_tooltip_text(
            open,
            "Open the selected problem's source file without overwriting unsaved text");
        g_signal_connect(
            open,
            "clicked",
            G_CALLBACK(RuntimeProblemsOpenClicked),
            runtime);
    }

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
        gtk_widget_add_css_class(
            actions, "umicom-runtime-panel-actions");
        gtk_box_append(GTK_BOX(actions), open);
        gtk_box_append(GTK_BOX(actions), previous);
        gtk_box_append(GTK_BOX(actions), next);
        gtk_box_append(GTK_BOX(actions), clear);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    filters = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    search = gtk_search_entry_new();
    severity = gtk_drop_down_new_from_strings(severityOptions);
    sort = gtk_drop_down_new_from_strings(sortOptions);
    resolved = gtk_check_button_new_with_label("Resolved");

    if (filters != NULL && search != NULL &&
        severity != NULL && sort != NULL && resolved != NULL &&
        runtime->problems_list != NULL) {
        gtk_widget_set_hexpand(search, TRUE);
        g_object_set(
            G_OBJECT(search),
            "placeholder-text",
            "Filter message, code, detail or path",
            NULL);
        gtk_drop_down_set_selected(
            GTK_DROP_DOWN(severity), 0U);
        gtk_drop_down_set_selected(
            GTK_DROP_DOWN(sort), 0U);

        /*
         * Retain the controls with the live list so timer refreshes never
         * follow a destroyed sibling widget.
         */
        g_object_set_data_full(
            G_OBJECT(runtime->problems_list),
            "umicom-problems-search",
            g_object_ref(search),
            g_object_unref);
        g_object_set_data_full(
            G_OBJECT(runtime->problems_list),
            "umicom-problems-severity",
            g_object_ref(severity),
            g_object_unref);
        g_object_set_data_full(
            G_OBJECT(runtime->problems_list),
            "umicom-problems-sort",
            g_object_ref(sort),
            g_object_unref);
        g_object_set_data_full(
            G_OBJECT(runtime->problems_list),
            "umicom-problems-resolved",
            g_object_ref(resolved),
            g_object_unref);

        g_signal_connect(
            search,
            "search-changed",
            G_CALLBACK(RuntimeProblemsSearchChanged),
            runtime);
        g_signal_connect(
            severity,
            "notify::selected",
            G_CALLBACK(RuntimeProblemsChoiceChanged),
            runtime);
        g_signal_connect(
            sort,
            "notify::selected",
            G_CALLBACK(RuntimeProblemsChoiceChanged),
            runtime);
        g_signal_connect(
            resolved,
            "toggled",
            G_CALLBACK(RuntimeProblemsResolvedChanged),
            runtime);

        gtk_widget_set_tooltip_text(
            severity,
            "Filter through Framework's diagnostic severity policy");
        gtk_widget_set_tooltip_text(
            sort,
            "Sort the canonical diagnostic query");
        gtk_widget_set_tooltip_text(
            resolved,
            "Include diagnostics already marked resolved");

        gtk_box_append(GTK_BOX(filters), search);
        gtk_box_append(GTK_BOX(filters), severity);
        gtk_box_append(GTK_BOX(filters), sort);
        gtk_box_append(GTK_BOX(filters), resolved);
        gtk_box_append(GTK_BOX(panel), filters);
    }

    if (runtime->problems_list != NULL) {
        GtkWidget *scroll;

        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->problems_list),
            GTK_SELECTION_SINGLE);
        g_signal_connect(
            runtime->problems_list,
            "row-selected",
            G_CALLBACK(RuntimeProblemsSelected),
            runtime);
        g_signal_connect(
            runtime->problems_list,
            "row-activated",
            G_CALLBACK(RuntimeProblemsActivated),
            runtime);

        scroll = runtime_scroll(runtime->problems_list);
        if (scroll != NULL) {
            gtk_box_append(GTK_BOX(panel), scroll);
        }
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Cross-navigation",
            "Select a problem and choose Open, or activate its row. Previous and Next "
            "use the same Framework diagnostic model to visit source locations "
            "without saving or replacing an existing draft."));

    runtime_append_context_actions(runtime, panel, "problems");

    runtime->problemsRendered = 0;
    runtime_problems_refresh_list(runtime);
    return panel;
}
