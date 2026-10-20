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

#include "umicom/vcs/working_tree_gtk4.h"

/* Framework owns the Git reader and worker lifetime. Studio supplies the observed
 * repository root and embeds its reusable review alongside the existing VCS commands. */
/* The embedded review previously kept the root from panel creation. Retained for review; the replacement follows the selected project and discards stale observations. */
#if 0
static void RuntimeSourceControlRepositoryReview(
    UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    void *review = NULL;
    if (runtime->source_control_snapshot.root[0] == '\0') return;
    UmiStatus status = UmiVcsWorkingTreeGtk4Create(
        runtime->source_control_snapshot.root, &review);
    if (status != UMI_STATUS_OK) return;
    GtkWidget *expander = gtk_expander_new("Repository and submodule review");
    gtk_expander_set_child(GTK_EXPANDER(expander), GTK_WIDGET(review));
    gtk_box_append(GTK_BOX(panel), expander);
}

#endif
/* The embedded review owns a captured repository. Replace that child when
 * project identity changes; Framework cancels its old observation on unmap. */
static void RuntimeRefreshRepositoryReview(GtkWidget *expander, const char *root)
{
    const char *previous = g_object_get_data(G_OBJECT(expander), "studio.repository.root");
    if (g_strcmp0(previous, root) == 0) return;
    void *review = NULL;
    GtkWidget *child;
    if (root[0] == '\0') {
        child = gtk_label_new("Open a project to inspect its repository.");
    } else {
        UmiStatus status = UmiVcsWorkingTreeGtk4Create(root, &review);
        child = status == UMI_STATUS_OK ? GTK_WIDGET(review)
            : gtk_label_new("Repository inspection is unavailable in this build.");
    }
    g_object_set_data_full(G_OBJECT(expander), "studio.repository.root", g_strdup(root), g_free);
    gtk_expander_set_child(GTK_EXPANDER(expander), child);
}

static void RuntimeSourceControlRepositoryReview(
    UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *expander = gtk_expander_new("Repository and submodule review");
    (void)umi_gtk4_automation_tag_widget(expander, "studio.repository.review");
    gtk_box_append(GTK_BOX(panel), expander);
    /* Both widgets belong to this panel. Refresh retains them before emitting
     * label or hierarchy notifications that may close the host window. */
    g_object_set_data(G_OBJECT(runtime->source_control_summary_label),
        "studio.repository.review", expander);
    RuntimeRefreshRepositoryReview(expander, runtime->source_control_snapshot.root);
}

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
/* Provider readiness changes after opening a project or refreshing Git. The live summary below replaces this creation-time label, retained for review. */
#if 0
    runtime_panel_add_stat(
        panel,
        "Provider",
        runtime->source_control_snapshot.available
            ? runtime->source_control_snapshot.provider_id
            : "Unavailable");
#endif

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
    (void)umi_gtk4_automation_tag_widget(refresh, "studio.source-control.refresh");
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

    RuntimeSourceControlRepositoryReview(runtime, panel);

    runtime_append_context_actions(runtime, panel, "source_control");

    runtime_source_control_refresh_list(runtime);
    runtime_source_control_refresh_details(runtime);
    return panel;
}
