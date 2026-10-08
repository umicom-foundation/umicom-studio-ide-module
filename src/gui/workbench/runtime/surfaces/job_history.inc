/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/job_history.inc
 * PURPOSE: Bind Studio controls to shared persistent build evidence.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Owner actions are weakly bound by the workbench. Retained buttons cannot use a
 * destroyed service. History reads are explicit and refused while a job runs. */
static void RuntimeBuildHistoryCapture(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioBuildService *service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    UmiJobHistorySnapshot *snapshot = g_new0(UmiJobHistorySnapshot, 1);
    UmiStatus status = UmiStudioBuildCaptureJobs(service, snapshot);
    (void)button;
    if (status != UMI_STATUS_OK)
    {
        runtime_show_notification(runtime, umi_status_text(status));
        g_free(snapshot);
        return;
    }
    GString *text =
        g_string_new("Saved job outcomes. Unfinished entries are not proof of a running process.\n");
    for (size_t i = 0; i < snapshot->count; ++i)
    {
        UmiJobHistoryEntry *entry = &snapshot->entries[i];
        char *caption = g_utf8_make_valid(entry->label, -1);
        g_string_append_printf(text, "\n#%" G_GUINT64_FORMAT "  %s\n%s; %u/%u phases recorded; %s\n",
                               (guint64)entry->id, caption, UmiJobHistoryStateText(entry->state),
                               entry->completed_steps, entry->total_steps,
                               UmiJobHistoryIsFinished(entry->state) ? umi_status_text(entry->result)
                                                                     : "outcome unknown; no automatic retry");
        g_free(caption);
    }
    if (snapshot->count == 0)
        g_string_append(text, "\nNo saved jobs.");
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(runtime->build_history_entries)),
                             text->str, -1);
    g_string_free(text, TRUE);
    g_free(snapshot);
}
static void RuntimeBuildHistoryOpen(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioBuildService *service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    bool detach = g_object_get_data(G_OBJECT(button), "umicom-detach-job-history") != NULL;
    const char *path = detach ? NULL : gtk_editable_get_text(GTK_EDITABLE(runtime->build_history_path));
    UmiStatus status = UmiStudioBuildOpenJobHistory(service, path);
    runtime_show_notification(runtime,
                              status != UMI_STATUS_OK ? umi_status_text(status)
                              : detach ? "Job history detached. The database remains on disk."
                                       : "Local job history opened. Builds are never resumed automatically.");
    if (status == UMI_STATUS_OK)
    {
        if (detach)
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(runtime->build_history_entries)),
                                     "No local job history attached.", -1);
        else
            RuntimeBuildHistoryCapture(button, data);
    }
}
static void RuntimeBuildHistoryPrune(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioBuildService *service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    size_t removed = 0;
    UmiStatus status = UmiStudioBuildPruneFinishedJobs(service, &removed);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, umi_status_text(status));
    else
    {
        char message[160];
        (void)snprintf(message, sizeof(message),
                       "Removed %zu finished job record(s). Unfinished evidence is retained.", removed);
        runtime_show_notification(runtime, message);
        RuntimeBuildHistoryCapture(button, data);
    }
}
/* This periodic read copies memory only. A slow database cannot block the
 * compiler-output refresh while the worker is recording its checkpoint. */
static void RuntimeBuildHistoryRefresh(UmiStudioGtkRuntimeChrome *runtime, UmiStudioBuildService *service)
{
    if (runtime->build_history_status == NULL)
        return;
    UmiStudioBuildJobHistory *state = g_new0(UmiStudioBuildJobHistory, 1);
    UmiStatus status = UmiStudioBuildReadJobHistory(service, state);
    char *message;
    if (status != UMI_STATUS_OK)
        message = g_strdup(umi_status_text(status));
    else if (!state->current.enabled)
        message = g_strdup("Persistent job history is disabled.");
    else if (state->current.entry.id == 0)
        message = g_strdup_printf("%s\nNo job recorded for this selection; storage: %s", state->path,
                                  umi_status_text(state->current.storage_status));
    else
        message =
            g_strdup_printf("%s\nCurrent job record: #%" G_GUINT64_FORMAT "; storage: %s", state->path,
                            (guint64)state->current.entry.id, umi_status_text(state->current.storage_status));
    gtk_label_set_text(GTK_LABEL(runtime->build_history_status), message);
    g_free(message);
    g_free(state);
}
static void RuntimeBuildHistoryControls(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    GtkWidget *expander = gtk_expander_new("Local job history");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_expander_set_child(GTK_EXPANDER(expander), box);
    GtkWidget *help =
        gtk_label_new("Choose a private SQLite file outside your source repositories. Reopen the same file "
                      "after restarting Studio. Saved outcomes never restart jobs.");
    gtk_label_set_wrap(GTK_LABEL(help), TRUE);
    gtk_label_set_xalign(GTK_LABEL(help), 0.0F);
    gtk_box_append(GTK_BOX(box), help);
    runtime->build_history_path = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->build_history_path),
                                   "Absolute path to a private job-history database");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->build_history_path), (int)UMI_PATH_CAPACITY - 1);
    (void)umi_gtk4_automation_tag_widget(runtime->build_history_path, "studio.build.history.path");
    gtk_box_append(GTK_BOX(box), runtime->build_history_path);
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *open = gtk_button_new_with_label("Open history");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh saved jobs");
    GtkWidget *prune = gtk_button_new_with_label("Remove finished records");
    GtkWidget *detach = gtk_button_new_with_label("Detach history");
    (void)umi_gtk4_automation_tag_widget(open, "studio.build.history.open");
    (void)umi_gtk4_automation_tag_widget(refresh, "studio.build.history.refresh");
    (void)umi_gtk4_automation_tag_widget(prune, "studio.build.history.prune");
    (void)umi_gtk4_automation_tag_widget(detach, "studio.build.history.detach");
    g_object_set_data(G_OBJECT(detach), "umicom-detach-job-history", GINT_TO_POINTER(1));
    runtime_bind_editor_action(runtime, open, RuntimeBuildHistoryOpen);
    runtime_bind_editor_action(runtime, refresh, RuntimeBuildHistoryCapture);
    runtime_bind_editor_action(runtime, prune, RuntimeBuildHistoryPrune);
    runtime_bind_editor_action(runtime, detach, RuntimeBuildHistoryOpen);
    gtk_box_append(GTK_BOX(controls), open);
    gtk_box_append(GTK_BOX(controls), refresh);
    gtk_box_append(GTK_BOX(controls), prune);
    gtk_box_append(GTK_BOX(controls), detach);
    gtk_box_append(GTK_BOX(box), controls);
    runtime->build_history_status = gtk_label_new(NULL);
    gtk_label_set_wrap(GTK_LABEL(runtime->build_history_status), TRUE);
    gtk_label_set_xalign(GTK_LABEL(runtime->build_history_status), 0.0F);
    (void)umi_gtk4_automation_tag_widget(runtime->build_history_status, "studio.build.history.status");
    gtk_box_append(GTK_BOX(box), runtime->build_history_status);
    runtime->build_history_entries = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(runtime->build_history_entries), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(runtime->build_history_entries), TRUE);
    (void)umi_gtk4_automation_tag_widget(runtime->build_history_entries, "studio.build.history.entries");
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 180);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), runtime->build_history_entries);
    gtk_box_append(GTK_BOX(box), scroll);
    gtk_box_append(GTK_BOX(panel), expander);
}
