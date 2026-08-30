/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/source_control_details.inc
 *
 * PURPOSE:
 *   Project Framework VCS history and branch collections alongside the existing
 *   working-tree change list. Checkout remains a canonical registered Studio
 *   VCS command and the Framework workspace remains authoritative.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_vcs_branch_checkout(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *branch =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-vcs-branch");

    if (runtime == NULL || branch == NULL || branch[0] == '\0') return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_VCS_BRANCH_CHECKOUT,
        branch);
}

static void runtime_source_control_refresh_details(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioSourceControlService *service;
    UmiVcsWorkspace *workspace;
    const UmiVcsHistory *history;
    const UmiVcsBranchList *branches;
    size_t index;

    if (runtime == NULL) return;

    service = runtime_source_control_service(runtime);
    workspace = service != NULL
        ? umi_studio_source_control_service_workspace(service)
        : NULL;
    if (workspace == NULL) return;

    history = umi_vcs_workspace_history(workspace);
    branches = umi_vcs_workspace_branches(workspace);

    runtime_clear_list_box(runtime->source_control_history_list);
    runtime_clear_list_box(runtime->source_control_branches_list);

    if (runtime->source_control_history_list != NULL && history != NULL) {
        const size_t count = umi_vcs_history_count(history);

        for (index = 0U; index < count && index < 150U; ++index) {
            const UmiVcsCommit *commit = umi_vcs_history_at(history, index);
            GtkWidget *row;
            GtkWidget *subject;
            GtkWidget *detail;
            char detail_text[768];

            if (commit == NULL) continue;

            row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
            subject = runtime_label(commit->subject, 0.0F);
            (void)snprintf(
                detail_text,
                sizeof(detail_text),
                "%.16s — %s <%s>",
                commit->commit_id,
                commit->author,
                commit->email);
            detail = runtime_label(detail_text, 0.0F);

            if (row == NULL || subject == NULL || detail == NULL) continue;

            gtk_widget_add_css_class(detail, "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(row), subject);
            gtk_box_append(GTK_BOX(row), detail);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->source_control_history_list),
                row);
        }
    }

    if (runtime->source_control_branches_list != NULL && branches != NULL) {
        const size_t count = umi_vcs_branch_list_count(branches);

        for (index = 0U; index < count && index < 250U; ++index) {
            const UmiVcsBranch *branch =
                umi_vcs_branch_list_at(branches, index);
            GtkWidget *row;
            GtkWidget *label;
            GtkWidget *spacer;
            GtkWidget *checkout;
            char text[768];

            if (branch == NULL) continue;

            (void)snprintf(
                text,
                sizeof(text),
                "%s%s%s  ↑%d ↓%d%s%s",
                branch->current ? "▶ " : "",
                branch->detached ? "(detached) " : "",
                branch->name,
                branch->ahead,
                branch->behind,
                branch->upstream[0] != '\0' ? "  " : "",
                branch->upstream);

            row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            label = runtime_label(text, 0.0F);
            spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            checkout = gtk_button_new_with_label("Checkout");

            if (row == NULL || label == NULL ||
                spacer == NULL || checkout == NULL) {
                continue;
            }

            gtk_widget_set_hexpand(spacer, TRUE);
            gtk_widget_set_sensitive(checkout, !branch->current);

            g_object_set_data_full(
                G_OBJECT(checkout),
                "umicom-vcs-branch",
                g_strdup(branch->name),
                g_free);
            g_signal_connect(
                checkout,
                "clicked",
                G_CALLBACK(runtime_on_vcs_branch_checkout),
                runtime);

            gtk_box_append(GTK_BOX(row), label);
            gtk_box_append(GTK_BOX(row), spacer);
            gtk_box_append(GTK_BOX(row), checkout);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->source_control_branches_list),
                row);
        }
    }
}

static GtkWidget *runtime_source_control_notebook(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *notebook;

    if (runtime == NULL || runtime->source_control_list == NULL) return NULL;

    notebook = gtk_notebook_new();
    runtime->source_control_history_list = gtk_list_box_new();
    runtime->source_control_branches_list = gtk_list_box_new();

    if (notebook == NULL ||
        runtime->source_control_history_list == NULL ||
        runtime->source_control_branches_list == NULL) {
        return notebook;
    }

    gtk_widget_set_size_request(notebook, -1, 300);

    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->source_control_list),
        gtk_label_new("Changes"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->source_control_history_list),
        gtk_label_new("History"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->source_control_branches_list),
        gtk_label_new("Branches"));

    runtime_source_control_refresh_details(runtime);
    return notebook;
}
