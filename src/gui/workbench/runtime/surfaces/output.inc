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
