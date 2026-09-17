/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/problems.inc
 *
 * PURPOSE:
 *   Render retained Framework diagnostics in the Problems surface while keeping
 *   source navigation on the existing Framework problem-navigation commands.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Rows carry a retained diagnostic ID, not a mutable chronological index. */
static void RuntimeProblemsOpenSelected(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return;
    UmiStatus status = UmiStudioUiOpenProblem(runtime->owner->ui, runtime->problemsSelectedId);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "This problem has no available source location. Read Output for the complete message.");
    (void)runtime_refresh_services(runtime);
}

static void RuntimeProblemsSelected(GtkListBox *list, GtkListBoxRow *row, gpointer userData)
{
    UmiStudioGtkRuntimeChrome *runtime = userData;
    const char *id = row != NULL ? g_object_get_data(G_OBJECT(row), "umicom-problem-id") : NULL;
    (void)list;
    if (runtime != NULL) g_strlcpy(runtime->problemsSelectedId,
        id != NULL ? id : "", sizeof runtime->problemsSelectedId);
}

static void RuntimeProblemsActivated(GtkListBox *list, GtkListBoxRow *row, gpointer userData)
{
    RuntimeProblemsSelected(list, row, userData);
    RuntimeProblemsOpenSelected(userData);
}

static void RuntimeProblemsOpenClicked(GtkButton *button, gpointer userData)
{
    (void)button;
    RuntimeProblemsOpenSelected(userData);
}

static void runtime_problems_refresh_list(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiDiagnosticModel *model;
    size_t count, active = 0U, warnings = 0U, errors = 0U, fatal = 0U, shown = 0U;
    char text[256], selected[UMI_DIAGNOSTIC_ID_CAPACITY];
    if (runtime == NULL || runtime->problems_list == NULL || runtime->owner == NULL) return;
    services = umi_studio_ui_services(runtime->owner->ui);
    model = services != NULL ? umi_diagnostic_pipeline_model(
        umi_studio_services_diagnostic_pipeline(services)) : NULL;
    if (model == NULL) return;
    uint64_t revision = umi_diagnostic_model_revision(model);
    if (runtime->problemsRendered && runtime->problemsModelRevision == revision) return;
    g_strlcpy(selected, runtime->problemsSelectedId, sizeof selected);
    runtime_clear_list_box(runtime->problems_list);
    count = umi_diagnostic_model_count(model);
    /* Display the newest 300 active records while navigation remains able to
     * traverse every retained source problem. Output keeps raw build text. */
    for (size_t i = 0U; i < count; ++i) {
        UmiDiagnosticSnapshot problem;
        if (umi_diagnostic_model_at(model, i, &problem) != UMI_STATUS_OK || problem.resolved) continue;
        ++active;
        if (problem.severity == UMI_DIAGNOSTIC_WARNING) ++warnings;
        if (problem.severity == UMI_DIAGNOSTIC_ERROR) ++errors;
        if (problem.severity == UMI_DIAGNOSTIC_FATAL) ++fatal;
    }
    size_t skip = active > 300U ? active - 300U : 0U, seen = 0U;
    for (size_t i = 0U; i < count; ++i) {
        UmiDiagnosticSnapshot problem;
        if (umi_diagnostic_model_at(model, i, &problem) != UMI_STATUS_OK || problem.resolved) continue;
        if (seen++ < skip) continue;
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        char *heading = g_strdup_printf("%s — %s%s%s", umi_diagnostic_severity_text(problem.severity),
            problem.source, problem.code[0] != '\0' ? " — " : "", problem.code);
        GtkWidget *severity = runtime_label(heading, 0.0F);
        g_free(heading);
        gtk_widget_add_css_class(severity, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(box), severity);
        gtk_box_append(GTK_BOX(box), runtime_label(problem.message, 0.0F));
        if (problem.uri[0] != '\0') {
            char *location = g_strdup_printf("%s:%u:%u", problem.uri,
                (unsigned)problem.line, (unsigned)problem.column);
            GtkWidget *label = runtime_label(location, 0.0F);
            gtk_widget_set_tooltip_text(row, location);
            gtk_box_append(GTK_BOX(box), label); g_free(location);
        }
        if (problem.detail[0] != '\0') gtk_widget_set_tooltip_text(box, problem.detail);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
        g_object_set_data_full(G_OBJECT(row), "umicom-problem-id", g_strdup(problem.id), g_free);
        gtk_list_box_append(GTK_LIST_BOX(runtime->problems_list), row);
        if (strcmp(selected, problem.id) == 0) gtk_list_box_select_row(GTK_LIST_BOX(runtime->problems_list), GTK_LIST_BOX_ROW(row));
        ++shown;
    }
    if (shown == 0U) gtk_list_box_append(GTK_LIST_BOX(runtime->problems_list), runtime_label("No active problems. Build a project to collect compiler messages.", 0.0F));
    snprintf(text, sizeof text, "%zu active — %zu warnings, %zu errors, %zu fatal — showing %zu", active, warnings, errors, fatal, shown);
    if (runtime->problems_summary_label != NULL) gtk_label_set_text(GTK_LABEL(runtime->problems_summary_label), text);
    runtime->problemsModelRevision = revision; runtime->problemsRendered = 1;
}

/* Release or reset state held by runtime on problems so the same storage can be reused
 * safely. */
static void runtime_on_problems_clear(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL) return;
    (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_DIAGNOSTICS_CLEAR, NULL);
    runtime_problems_refresh_list(runtime);
}
