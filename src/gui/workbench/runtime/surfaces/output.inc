/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/output.inc
 *
 * PURPOSE:
 *   Build the Framework-backed output presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/build/live_output_gtk4.h"

/* Resolve the current service only during a weakly bound owner action. The
 * controls keep no raw service pointer after the workbench closes. */
static void RuntimeBuildLogSelect(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioBuildService *service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    const char *path = g_object_get_data(G_OBJECT(button), "umicom-clear-build-log") != NULL ? NULL :
        gtk_editable_get_text(GTK_EDITABLE(runtime->build_log_path));
    UmiStatus status = UmiStudioBuildArmLog(service, path);
    runtime_show_notification(runtime, status != UMI_STATUS_OK ? umi_status_text(status) :
        (path == NULL || path[0] == '\0' ? "The next job will not create a log file." :
        "Log selected for the next job. The destination must be a new file in an existing folder."));
}

/* Copy the actual accepted path rather than an unsubmitted entry draft. */
static void RuntimeBuildLogCopyPath(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioBuildLogState *state = g_new0(UmiStudioBuildLogState, 1);
    UmiStudioBuildService *service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    if (UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK && state->captured.enabled)
        gdk_clipboard_set_text(gtk_widget_get_clipboard(GTK_WIDGET(button)), state->captured.file.path);
    else runtime_show_notification(runtime, "No job has an accepted log file yet.");
    g_free(state);
}

/* Poll only published metadata. File flushing remains on Framework's worker. */
static void RuntimeBuildLogRefresh(UmiStudioGtkRuntimeChrome *runtime, UmiStudioBuildService *service)
{
    if (runtime->build_log_status == NULL) return;
    UmiStudioBuildLogState *state = g_new0(UmiStudioBuildLogState, 1);
    if (UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK) {
        UmiBuildLogSnapshot *log = &state->captured;
        const char *capture = !log->enabled ? "No log for the last submitted job" :
            log->file.status != UMI_STATUS_OK ? "Log incomplete: storage error" :
            !log->file.closed ? "Writing log" :
            log->capture_complete ? "Emitted output saved" : "Log closed; capture is incomplete";
        char *message = g_strdup_printf("%s\n%s%s\nBytes written: %" G_GUINT64_FORMAT " of %" G_GUINT64_FORMAT " received\nNext job: %s", capture,
            log->enabled ? log->file.path : "", log->enabled && log->file.status != UMI_STATUS_OK ?
                " (inspect build results separately)" : "",
            (guint64)log->file.bytes_written, (guint64)log->bytes_received,
            state->next_path[0] != '\0' ? state->next_path : "no log selected");
        gtk_label_set_text(GTK_LABEL(runtime->build_log_status), message);
        g_free(message);
    }
    g_free(state);
}

/* Live output has its own bounded Framework presenter. The completed result
 * view below remains available for commands, diagnostics and history review. */
/* The Framework runner publishes complete phase output. The current phase
 * remains visible while the worker runs; partial output is not mislabelled as
 * a successful build. Compiler bytes are repaired for GTK's UTF-8 text API. */
static void RuntimeRefreshBuildOutput(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioBuildService *service;
    UmiStudioBuildSnapshot state;
    UmiBuildProjectSessionSnapshot progress;
    UmiBuildResult *result = NULL;
    char label[192];
    if (runtime == NULL || runtime->owner == NULL) return;
    service = umi_studio_services_build(umi_studio_ui_services(runtime->owner->ui));
    if (service == NULL) return;
    RuntimeBuildLogRefresh(runtime, service);
    if (runtime->build_live_output != NULL) {
        UmiBuildOutputSnapshot *output = g_new0(UmiBuildOutputSnapshot, 1);
        if (UmiStudioBuildReadOutput(service, output) == UMI_STATUS_OK)
            (void)UmiBuildOutputGtk4Update(runtime->build_live_output, output);
        g_free(output);
    }
    if (UmiStudioBuildProgress(service, &progress) == UMI_STATUS_OK &&
        runtime->build_progress_label != NULL) {
        if (progress.active) {
            (void)snprintf(label, sizeof(label), "%s%s — %zu phase(s) completed",
                umi_build_phase_text(progress.current_phase),
                progress.cancellation_requested ? " (cancelling)" : " running",
                progress.completed_phase_count);
        } else if (progress.operation_id != 0U) {
            (void)snprintf(label, sizeof(label), "%s finished: %s",
                umi_build_phase_text(progress.requested_phase), umi_status_text(progress.status));
        } else {
            (void)snprintf(label, sizeof(label), "Ready. Review Project Settings, then configure or build.");
        }
        gtk_label_set_text(GTK_LABEL(runtime->build_progress_label), label);
    }
    if (runtime->build_output_view == NULL ||
        umi_studio_build_service_snapshot(service, &state) != UMI_STATUS_OK ||
        state.history_count == 0U ||
        umi_build_result_create(&result) != UMI_STATUS_OK) return;
    if (umi_build_history_latest(umi_studio_build_service_history(service), result) == UMI_STATUS_OK &&
        result->operation_id != runtime->build_output_operation) {
        char summary[512];
        char *validOutput = g_utf8_make_valid(result->output, -1);
        char *validCommand = g_utf8_make_valid(result->command, -1);
        (void)umi_build_result_summary(result, summary, sizeof(summary));
        char *text = g_strdup_printf("%s\n%s\n\n%s", summary, validCommand, validOutput);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(runtime->build_output_view)), text, -1);
        runtime->build_output_operation = result->operation_id;
        g_free(text);
        g_free(validCommand);
        g_free(validOutput);
    }
    umi_build_result_destroy(result);
}

static GtkWidget *runtime_build_surface_output(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Output");
    GtkWidget *view;
    char value[64];
    if (panel == NULL || runtime == NULL) return panel;
    /* Retain the existing task and diagnostic readouts alongside the new
     * compiler transcript. Adding output must not remove earlier information. */
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.queued_tasks);
    runtime_panel_add_stat(panel, "Queued tasks", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.running_tasks);
    runtime_panel_add_stat(panel, "Running tasks", value);
    (void)snprintf(value, sizeof(value), "%zu",
                   runtime->view_model_snapshot.retained_diagnostics);
    runtime_panel_add_stat(panel, "Diagnostics", value);

    /* Saving is deliberate and uses a fresh file. The entry is only a draft
     * until Use log for next job succeeds; typing never changes a running job. */
    GtkWidget *log_title = gtk_label_new("Save complete emitted output to a new file");
    gtk_label_set_xalign(GTK_LABEL(log_title), 0.0F);
    gtk_box_append(GTK_BOX(panel), log_title);
    runtime->build_log_path = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->build_log_path), "Absolute path to a new log file");
    gtk_entry_set_max_length(GTK_ENTRY(runtime->build_log_path), (int)UMI_PATH_CAPACITY - 1);
    (void)umi_gtk4_automation_tag_widget(runtime->build_log_path, "studio.build.log.path");
    gtk_box_append(GTK_BOX(panel), runtime->build_log_path);
    GtkWidget *log_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *select_log = gtk_button_new_with_label("Use log for next job");
    GtkWidget *clear_log = gtk_button_new_with_label("Disable next log");
    GtkWidget *copy_log = gtk_button_new_with_label("Copy log path");
    (void)umi_gtk4_automation_tag_widget(select_log, "studio.build.log.select");
    (void)umi_gtk4_automation_tag_widget(clear_log, "studio.build.log.clear");
    (void)umi_gtk4_automation_tag_widget(copy_log, "studio.build.log.copy-path");
    g_object_set_data(G_OBJECT(clear_log), "umicom-clear-build-log", GINT_TO_POINTER(1));
    runtime_bind_editor_action(runtime, select_log, RuntimeBuildLogSelect);
    runtime_bind_editor_action(runtime, clear_log, RuntimeBuildLogSelect);
    runtime_bind_editor_action(runtime, copy_log, RuntimeBuildLogCopyPath);
    gtk_box_append(GTK_BOX(log_controls), select_log);
    gtk_box_append(GTK_BOX(log_controls), clear_log);
    gtk_box_append(GTK_BOX(log_controls), copy_log);
    gtk_box_append(GTK_BOX(panel), log_controls);
    runtime->build_log_status = gtk_label_new(NULL);
    gtk_label_set_wrap(GTK_LABEL(runtime->build_log_status), TRUE);
    gtk_label_set_xalign(GTK_LABEL(runtime->build_log_status), 0.0F);
    (void)umi_gtk4_automation_tag_widget(runtime->build_log_status, "studio.build.log.status");
    gtk_box_append(GTK_BOX(panel), runtime->build_log_status);
    runtime->build_live_output = UmiBuildOutputGtk4Create();
    gtk_box_append(GTK_BOX(panel), runtime->build_live_output);
    GtkWidget *completed = gtk_label_new("Latest completed phase");
    gtk_label_set_xalign(GTK_LABEL(completed), 0.0F);
    gtk_box_append(GTK_BOX(panel), completed);
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
    gtk_widget_set_vexpand(view, TRUE);
    gtk_widget_set_hexpand(view, TRUE);
    gtk_widget_set_tooltip_text(view, "Compiler, test or program output from the latest completed phase.");
    (void)umi_gtk4_automation_tag_widget(view, "studio.build.output");
    runtime->build_output_view = view;
    runtime->build_output_operation = 0U;
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)),
        "No build output yet. Output is updated when each phase finishes.", -1);
    gtk_box_append(GTK_BOX(panel), view);
    RuntimeRefreshBuildOutput(runtime);
    return runtime_scroll(panel);
}
