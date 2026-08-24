/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/source_control.inc
 *
 * PURPOSE:
 *   Render and operate the existing provider-neutral Framework VCS change list,
 *   including stage/unstage, commit, pull/push and unified-diff inspection.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiStudioSourceControlService *runtime_source_control_service(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;

    if (runtime == NULL || runtime->owner == NULL) return NULL;
    services = umi_studio_ui_services(runtime->owner->ui);
    return services != NULL
        ? umi_studio_services_source_control(services)
        : NULL;
}

static void runtime_diff_window_ensure(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    GtkWidget *notebook;
    GtkWidget *paned;
    GtkWidget *left_scroll;
    GtkWidget *right_scroll;
    GtkWidget *unified_scroll;

    if (runtime == NULL || runtime->diff_window != NULL) return;

    runtime->diff_window = GTK_WIDGET(gtk_window_new());
    runtime->diff_summary_label = gtk_label_new("");
    runtime->diff_text_view = gtk_text_view_new();
    runtime->diff_left_view = gtk_text_view_new();
    runtime->diff_right_view = gtk_text_view_new();
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    notebook = gtk_notebook_new();
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    if (runtime->diff_window == NULL || root == NULL ||
        notebook == NULL || paned == NULL ||
        runtime->diff_text_view == NULL ||
        runtime->diff_left_view == NULL ||
        runtime->diff_right_view == NULL ||
        runtime->diff_summary_label == NULL) {
        return;
    }

    gtk_window_set_title(
        GTK_WINDOW(runtime->diff_window),
        "Source Control Compare");
    gtk_window_set_default_size(
        GTK_WINDOW(runtime->diff_window),
        1180,
        760);
    gtk_window_set_transient_for(
        GTK_WINDOW(runtime->diff_window),
        runtime->owner->window);

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->diff_text_view),
        FALSE);
    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->diff_left_view),
        FALSE);
    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->diff_right_view),
        FALSE);

    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->diff_text_view),
        TRUE);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->diff_left_view),
        TRUE);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->diff_right_view),
        TRUE);

    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(runtime->diff_text_view),
        GTK_WRAP_NONE);
    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(runtime->diff_left_view),
        GTK_WRAP_NONE);
    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(runtime->diff_right_view),
        GTK_WRAP_NONE);

    gtk_widget_set_margin_top(root, 8);
    gtk_widget_set_margin_bottom(root, 8);
    gtk_widget_set_margin_start(root, 8);
    gtk_widget_set_margin_end(root, 8);

    gtk_widget_add_css_class(
        runtime->diff_summary_label,
        "umicom-runtime-muted");

    left_scroll = runtime_scroll(runtime->diff_left_view);
    right_scroll = runtime_scroll(runtime->diff_right_view);
    unified_scroll = runtime_scroll(runtime->diff_text_view);

    gtk_paned_set_start_child(
        GTK_PANED(paned),
        left_scroll);
    gtk_paned_set_end_child(
        GTK_PANED(paned),
        right_scroll);
    gtk_paned_set_position(GTK_PANED(paned), 560);

    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        paned,
        gtk_label_new("Side by Side"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        unified_scroll,
        gtk_label_new("Unified"));

    gtk_box_append(
        GTK_BOX(root),
        runtime->diff_summary_label);
    gtk_box_append(GTK_BOX(root), notebook);
    gtk_window_set_child(
        GTK_WINDOW(runtime->diff_window),
        root);

    g_signal_connect(
        runtime->diff_window,
        "close-request",
        G_CALLBACK(runtime_on_palette_close),
        runtime);
}

static void runtime_show_selected_diff(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioSourceControlService *service;
    UmiVcsWorkspace *workspace;
    const UmiVcsDiffDocument *document;
    UmiVcsDiffDocumentSnapshot snapshot;
    GtkTextBuffer *buffer;
    GString *text;
    GString *left;
    GString *right;
    size_t index;
    char summary[256];

    if (runtime == NULL) return;

    service = runtime_source_control_service(runtime);
    workspace = service != NULL
        ? umi_studio_source_control_service_workspace(service)
        : NULL;
    document = workspace != NULL
        ? umi_vcs_workspace_diff(workspace)
        : NULL;

    if (document == NULL ||
        umi_vcs_diff_document_snapshot(
            document,
            &snapshot) != UMI_STATUS_OK) {
        runtime_show_notification(runtime, "No diff is available.");
        return;
    }

    runtime_diff_window_ensure(runtime);
    if (runtime->diff_window == NULL ||
        runtime->diff_text_view == NULL ||
        runtime->diff_left_view == NULL ||
        runtime->diff_right_view == NULL) {
        return;
    }

    text = g_string_sized_new(4096U);
    left = g_string_sized_new(4096U);
    right = g_string_sized_new(4096U);
    if (text == NULL || left == NULL || right == NULL) {
        if (text != NULL) g_string_free(text, TRUE);
        if (left != NULL) g_string_free(left, TRUE);
        if (right != NULL) g_string_free(right, TRUE);
        return;
    }

    for (index = 0U;
         index < umi_vcs_diff_document_line_count(document);
         ++index) {
        const UmiVcsDiffLine *line =
            umi_vcs_diff_document_line_at(document, index);
        const char *prefix = " ";

        if (line == NULL) continue;

        switch (line->kind) {
            case UMI_VCS_DIFF_ADDITION:
                prefix = "+";
                g_string_append(right, "+ ");
                g_string_append(right, line->text);
                g_string_append_c(right, '\n');
                g_string_append(left, "  \n");
                break;

            case UMI_VCS_DIFF_DELETION:
                prefix = "-";
                g_string_append(left, "- ");
                g_string_append(left, line->text);
                g_string_append_c(left, '\n');
                g_string_append(right, "  \n");
                break;

            case UMI_VCS_DIFF_CONTEXT:
                g_string_append(left, "  ");
                g_string_append(left, line->text);
                g_string_append_c(left, '\n');
                g_string_append(right, "  ");
                g_string_append(right, line->text);
                g_string_append_c(right, '\n');
                break;

            case UMI_VCS_DIFF_HEADER:
            case UMI_VCS_DIFF_HUNK:
            case UMI_VCS_DIFF_NOTICE:
                prefix = "";
                g_string_append(left, line->text);
                g_string_append_c(left, '\n');
                g_string_append(right, line->text);
                g_string_append_c(right, '\n');
                break;

            default:
                break;
        }

        g_string_append(text, prefix);
        g_string_append(text, line->text);
        if (text->len == 0U ||
            text->str[text->len - 1U] != '\n') {
            g_string_append_c(text, '\n');
        }
    }

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->diff_text_view));
    gtk_text_buffer_set_text(buffer, text->str, -1);

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->diff_left_view));
    gtk_text_buffer_set_text(buffer, left->str, -1);

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->diff_right_view));
    gtk_text_buffer_set_text(buffer, right->str, -1);

    (void)snprintf(
        summary,
        sizeof(summary),
        "%.160s - %zu lines, +%zu / -%zu, %zu hunk%s%s",
        snapshot.path,
        snapshot.lines,
        snapshot.additions,
        snapshot.deletions,
        snapshot.hunks,
        snapshot.hunks == 1U ? "" : "s",
        snapshot.staged ? " - staged" : "");

    gtk_label_set_text(
        GTK_LABEL(runtime->diff_summary_label),
        summary);
    gtk_window_present(GTK_WINDOW(runtime->diff_window));

    g_string_free(text, TRUE);
    g_string_free(left, TRUE);
    g_string_free(right, TRUE);
}

static void runtime_on_vcs_stage_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *path = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-vcs-path");
    const int staged = GPOINTER_TO_INT(
        g_object_get_data(
            G_OBJECT(button),
            "umicom-vcs-staged"));

    if (runtime == NULL || path == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        staged
            ? UMI_STUDIO_COMMAND_VCS_UNSTAGE
            : UMI_STUDIO_COMMAND_VCS_STAGE,
        path);
}

static void runtime_on_vcs_diff_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *path = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-vcs-path");

    if (runtime == NULL || path == NULL) return;

    runtime_copy_selection(
        runtime->selected_vcs_path,
        sizeof(runtime->selected_vcs_path),
        path);

    if (runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_DIFF,
            path) == UMI_STATUS_OK) {
        runtime_show_selected_diff(runtime);
    }
}

static UmiStatus runtime_vcs_commit_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *value)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_VCS_COMMIT,
        value);
}

static void runtime_on_vcs_commit_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *message = NULL;
    (void)button;

    if (runtime == NULL) return;

    if (runtime->source_control_commit_entry != NULL) {
        message = gtk_editable_get_text(
            GTK_EDITABLE(runtime->source_control_commit_entry));
    }

    if (message != NULL && message[0] != '\0') {
        if (runtime_vcs_commit_accepted(runtime, message) == UMI_STATUS_OK) {
            gtk_editable_set_text(
                GTK_EDITABLE(runtime->source_control_commit_entry),
                "");
        }
        return;
    }

    (void)runtime_prompt_text(
        runtime,
        "Commit Changes",
        "Enter the Git commit message.",
        "",
        "Commit",
        runtime_vcs_commit_accepted);
}

static void runtime_source_control_refresh_list(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioSourceControlService *service;
    const UmiVcsChangeList *changes;
    size_t count;
    size_t index;
    char summary[256];

    if (runtime == NULL || runtime->source_control_list == NULL) return;

    runtime_clear_list_box(runtime->source_control_list);
    service = runtime_source_control_service(runtime);
    changes = service != NULL
        ? umi_studio_source_control_service_changes(service)
        : NULL;

    count = changes != NULL
        ? umi_vcs_change_list_count(changes)
        : 0U;

    for (index = 0U;
         index < count && index < UMI_STUDIO_GTK_VCS_ROW_LIMIT;
         ++index) {
        const UmiVcsChange *change =
            umi_vcs_change_list_at(changes, index);
        GtkWidget *row;
        GtkWidget *label;
        GtkWidget *spacer;
        GtkWidget *stage;
        GtkWidget *diff;
        char text[2300];
        const UmiVcsChangeState display_state =
            change != NULL &&
            change->worktree_state != UMI_VCS_CHANGE_UNMODIFIED
                ? change->worktree_state
                : (change != NULL
                    ? change->index_state
                    : UMI_VCS_CHANGE_UNMODIFIED);

        if (change == NULL) continue;

        (void)snprintf(
            text,
            sizeof(text),
            "%s  %s",
            umi_vcs_change_state_text(display_state),
            change->path);

        row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        label = runtime_label(text, 0.0F);
        spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        stage = gtk_button_new_with_label(
            change->staged ? "Unstage" : "Stage");
        diff = gtk_button_new_with_label("Diff");

        if (row == NULL || label == NULL || spacer == NULL ||
            stage == NULL || diff == NULL) {
            continue;
        }

        gtk_widget_set_hexpand(spacer, TRUE);
        gtk_widget_add_css_class(stage, "flat");
        gtk_widget_add_css_class(diff, "flat");

        g_object_set_data_full(
            G_OBJECT(stage),
            "umicom-vcs-path",
            g_strdup(change->path),
            g_free);
        g_object_set_data(
            G_OBJECT(stage),
            "umicom-vcs-staged",
            GINT_TO_POINTER(change->staged));
        g_signal_connect(
            stage,
            "clicked",
            G_CALLBACK(runtime_on_vcs_stage_clicked),
            runtime);

        g_object_set_data_full(
            G_OBJECT(diff),
            "umicom-vcs-path",
            g_strdup(change->path),
            g_free);
        g_signal_connect(
            diff,
            "clicked",
            G_CALLBACK(runtime_on_vcs_diff_clicked),
            runtime);

        gtk_box_append(GTK_BOX(row), label);
        gtk_box_append(GTK_BOX(row), spacer);
        gtk_box_append(GTK_BOX(row), stage);
        gtk_box_append(GTK_BOX(row), diff);
        gtk_list_box_append(
            GTK_LIST_BOX(runtime->source_control_list),
            row);
    }

    (void)snprintf(
        summary,
        sizeof(summary),
        "%.128s - %zu change%s, %zu staged, ahead %d / behind %d",
        runtime->source_control_snapshot.branch[0] != '\0'
            ? runtime->source_control_snapshot.branch
            : "No repository",
        runtime->source_control_snapshot.changes,
        runtime->source_control_snapshot.changes == 1U ? "" : "s",
        runtime->source_control_snapshot.staged,
        runtime->source_control_snapshot.ahead,
        runtime->source_control_snapshot.behind);

    if (runtime->source_control_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->source_control_summary_label),
            summary);
    }
}
