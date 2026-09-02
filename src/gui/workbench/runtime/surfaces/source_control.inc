/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/source_control.inc
 *
 * PURPOSE:
 *   Build the operational provider-neutral Source Control surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_vcs_refresh_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_VCS_REFRESH,
        NULL);
}

/*
 * Provide the runtime on vcs stage all button operation used by this module and its client
 * applications.
 */
static void runtime_on_vcs_stage_all_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_VCS_STAGE_ALL,
        NULL);
}

/*
 * Provide the runtime on vcs pull button operation used by this module and its client
 * applications.
 */
static void runtime_on_vcs_pull_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_VCS_PULL,
        NULL);
}

/*
 * Provide the runtime on vcs push button operation used by this module and its client
 * applications.
 */
static void runtime_on_vcs_push_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_VCS_PUSH,
        NULL);
}

/*
 * Provide the runtime build surface source control operation used by this module and its
 * client applications.
 */
static GtkWidget *runtime_build_surface_source_control(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Source Control");
    GtkWidget *actions;
    GtkWidget *refresh;
    GtkWidget *stage_all;
    GtkWidget *pull;
    GtkWidget *push;
    GtkWidget *commit_row;
    GtkWidget *commit;
    GtkWidget *notebook;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    runtime->source_control_summary_label = runtime_label("", 0.0F);
    runtime_panel_add_stat(
        panel,
        "Provider",
        runtime->source_control_snapshot.available
            ? runtime->source_control_snapshot.provider_id
            : "Unavailable");
    runtime->source_control_list = gtk_list_box_new();
    runtime->source_control_commit_entry = gtk_entry_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->source_control_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->source_control_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->source_control_summary_label);
    }

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    refresh = gtk_button_new_with_label("Refresh");
    stage_all = gtk_button_new_with_label("Stage All");
    pull = gtk_button_new_with_label("Pull");
    push = gtk_button_new_with_label("Push");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (actions != NULL && refresh != NULL &&
        stage_all != NULL && pull != NULL && push != NULL) {
        g_signal_connect(
            refresh, "clicked",
            G_CALLBACK(runtime_on_vcs_refresh_button), runtime);
        g_signal_connect(
            stage_all, "clicked",
            G_CALLBACK(runtime_on_vcs_stage_all_button), runtime);
        g_signal_connect(
            pull, "clicked",
            G_CALLBACK(runtime_on_vcs_pull_button), runtime);
        g_signal_connect(
            push, "clicked",
            G_CALLBACK(runtime_on_vcs_push_button), runtime);

        gtk_box_append(GTK_BOX(actions), refresh);
        gtk_box_append(GTK_BOX(actions), stage_all);
        gtk_box_append(GTK_BOX(actions), pull);
        gtk_box_append(GTK_BOX(actions), push);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    commit_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    commit = gtk_button_new_with_label("Commit");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commit_row != NULL &&
        runtime->source_control_commit_entry != NULL &&
        commit != NULL) {
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(runtime->source_control_commit_entry),
            "Commit message");
        gtk_widget_set_hexpand(
            runtime->source_control_commit_entry,
            TRUE);
        g_signal_connect(
            commit,
            "clicked",
            G_CALLBACK(runtime_on_vcs_commit_clicked),
            runtime);
        gtk_box_append(
            GTK_BOX(commit_row),
            runtime->source_control_commit_entry);
        gtk_box_append(GTK_BOX(commit_row), commit);
        gtk_box_append(GTK_BOX(panel), commit_row);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->source_control_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->source_control_list),
            GTK_SELECTION_NONE);
        notebook = runtime_source_control_notebook(runtime);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (notebook != NULL) {
            gtk_box_append(GTK_BOX(panel), notebook);
        }
    }

    runtime_append_context_actions(runtime, panel, "source_control");

    runtime_source_control_refresh_list(runtime);
    runtime_source_control_refresh_details(runtime);
    return panel;
}
