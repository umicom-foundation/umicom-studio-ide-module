/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/problems.inc
 *
 * PURPOSE:
 *   Render retained Framework diagnostics in the Problems surface while keeping
 *   source navigation on the existing Framework problem-navigation commands.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_problems_refresh_list(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiDiagnosticStore *store;
    UmiDiagnosticStoreSummary summary;
    size_t count;
    size_t start;
    size_t index;
    char text[256];

    if (runtime == NULL || runtime->problems_list == NULL ||
        runtime->owner == NULL) {
        return;
    }

    runtime_clear_list_box(runtime->problems_list);

    services = umi_studio_ui_services(runtime->owner->ui);
    store = services != NULL
        ? umi_studio_services_diagnostic_store(services)
        : NULL;

    if (store == NULL) {
        gtk_list_box_append(
            GTK_LIST_BOX(runtime->problems_list),
            runtime_label("Diagnostic store is unavailable.", 0.0F));
        return;
    }

    count = umi_diagnostic_store_count(store);
    start = count > 300U ? count - 300U : 0U;

    for (index = start; index < count; ++index) {
        UmiDiagnosticRecord record;
        GtkWidget *row;
        GtkWidget *severity;
        GtkWidget *message;
        char severity_text[64];

        (void)memset(&record, 0, sizeof(record));
        if (umi_diagnostic_store_record_at(
                store,
                index,
                &record) != UMI_STATUS_OK) {
            continue;
        }

        row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        if (row == NULL) continue;

        (void)snprintf(
            severity_text,
            sizeof(severity_text),
            "%s — %s",
            umi_diagnostic_severity_text(record.severity),
            record.source);

        severity = runtime_label(severity_text, 0.0F);
        message = runtime_label(record.message, 0.0F);
        if (severity == NULL || message == NULL) continue;

        gtk_widget_add_css_class(severity, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(row), severity);
        gtk_box_append(GTK_BOX(row), message);
        gtk_list_box_append(GTK_LIST_BOX(runtime->problems_list), row);
    }

    (void)memset(&summary, 0, sizeof(summary));
    if (umi_diagnostic_store_summary(store, &summary) == UMI_STATUS_OK) {
        (void)snprintf(
            text,
            sizeof(text),
            "%zu retained — %zu warning%s, %zu error%s, %zu fatal",
            summary.retained_count,
            summary.retained_by_severity[UMI_DIAGNOSTIC_WARNING],
            summary.retained_by_severity[UMI_DIAGNOSTIC_WARNING] == 1U
                ? ""
                : "s",
            summary.retained_by_severity[UMI_DIAGNOSTIC_ERROR],
            summary.retained_by_severity[UMI_DIAGNOSTIC_ERROR] == 1U
                ? ""
                : "s",
            summary.retained_by_severity[UMI_DIAGNOSTIC_FATAL]);

        if (runtime->problems_summary_label != NULL) {
            gtk_label_set_text(
                GTK_LABEL(runtime->problems_summary_label),
                text);
        }
    }
}

static void runtime_on_problems_clear(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;

    if (runtime == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_DIAGNOSTICS_CLEAR,
        NULL);
    runtime_problems_refresh_list(runtime);
}
