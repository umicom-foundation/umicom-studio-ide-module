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

#include "umicom/diagnostics/query.h"

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

/*
 * The previous chronological Problems projection is retained for review.
 * Framework's diagnostics query service now owns filtering, severity selection
 * and sorting so GTK4 does not duplicate those policies.
 */
#if 0
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
#endif

/* Read one Problems control retained by the live list. */
static GtkWidget *RuntimeProblemsControl(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *key)
{
    if (runtime == NULL || runtime->problems_list == NULL || key == NULL) {
        return NULL;
    }
    return g_object_get_data(G_OBJECT(runtime->problems_list), key);
}

/* Map the visible severity choice onto Framework's minimum-severity filter. */
static void RuntimeProblemsApplySeverity(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiDiagnosticFilter *filter)
{
    GtkWidget *selector = RuntimeProblemsControl(
        runtime, "umicom-problems-severity");
    guint selected = selector != NULL && GTK_IS_DROP_DOWN(selector)
        ? gtk_drop_down_get_selected(GTK_DROP_DOWN(selector))
        : 0U;

    if (filter == NULL) return;
    switch (selected) {
    case 1U:
        umi_diagnostic_filter_set_minimum_severity(
            filter, UMI_DIAGNOSTIC_WARNING);
        break;
    case 2U:
        umi_diagnostic_filter_set_minimum_severity(
            filter, UMI_DIAGNOSTIC_ERROR);
        break;
    case 3U:
        umi_diagnostic_filter_set_minimum_severity(
            filter, UMI_DIAGNOSTIC_FATAL);
        break;
    case 0U:
    default:
        break;
    }
}

/* Map the visible sort choice to the canonical Framework query order. */
static UmiDiagnosticSort RuntimeProblemsSort(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *selector = RuntimeProblemsControl(
        runtime, "umicom-problems-sort");
    guint selected = selector != NULL && GTK_IS_DROP_DOWN(selector)
        ? gtk_drop_down_get_selected(GTK_DROP_DOWN(selector))
        : 0U;

    switch (selected) {
    case 1U:
        return UMI_DIAGNOSTIC_SORT_SEVERITY_DESCENDING;
    case 2U:
        return UMI_DIAGNOSTIC_SORT_SEQUENCE_ASCENDING;
    case 0U:
    default:
        return UMI_DIAGNOSTIC_SORT_SEQUENCE_DESCENDING;
    }
}

/* Return whether resolved findings should participate in the current query. */
static int RuntimeProblemsIncludeResolved(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *toggle = RuntimeProblemsControl(
        runtime, "umicom-problems-resolved");

    return toggle != NULL && GTK_IS_CHECK_BUTTON(toggle)
        ? gtk_check_button_get_active(GTK_CHECK_BUTTON(toggle))
        : 0;
}

/* Copy the current search text into the Framework query. */
static void RuntimeProblemsApplyText(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiDiagnosticFilter *filter)
{
    GtkWidget *search = RuntimeProblemsControl(
        runtime, "umicom-problems-search");
    const char *text;

    if (filter == NULL || search == NULL || !GTK_IS_EDITABLE(search)) return;
    text = gtk_editable_get_text(GTK_EDITABLE(search));
    if (text == NULL) return;
    runtime_copy_selection(
        filter->text,
        sizeof(filter->text),
        text);
}

/*
 * Render a bounded page stream from Framework's diagnostic query. The
 * diagnostic model remains authoritative; the GTK list stores only copied IDs
 * used by the established navigation command.
 */
static void runtime_problems_refresh_list(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiDiagnosticModel *model;
    UmiDiagnosticQuery query;
    UmiDiagnosticQueryResult result;
    size_t count;
    size_t active = 0U;
    size_t resolved = 0U;
    size_t warnings = 0U;
    size_t errors = 0U;
    size_t fatal = 0U;
    size_t shown = 0U;
    size_t offset = 0U;
    size_t totalMatches = 0U;
    char text[320];
    char selected[UMI_DIAGNOSTIC_ID_CAPACITY];
    uint64_t revision;
    UmiStatus status;

    if (runtime == NULL || runtime->problems_list == NULL ||
        runtime->owner == NULL) {
        return;
    }

    services = umi_studio_ui_services(runtime->owner->ui);
    model = services != NULL
        ? umi_diagnostic_pipeline_model(
              umi_studio_services_diagnostic_pipeline(services))
        : NULL;
    if (model == NULL) return;

    revision = umi_diagnostic_model_revision(model);
    if (runtime->problemsRendered &&
        runtime->problemsModelRevision == revision) {
        return;
    }

    g_strlcpy(
        selected,
        runtime->problemsSelectedId,
        sizeof selected);
    runtime_clear_list_box(runtime->problems_list);

    count = umi_diagnostic_model_count(model);
    for (size_t index = 0U; index < count; ++index) {
        UmiDiagnosticSnapshot problem;

        if (umi_diagnostic_model_at(
                model, index, &problem) != UMI_STATUS_OK) {
            continue;
        }
        if (problem.resolved) {
            ++resolved;
            continue;
        }
        ++active;
        if (problem.severity == UMI_DIAGNOSTIC_WARNING) ++warnings;
        if (problem.severity == UMI_DIAGNOSTIC_ERROR) ++errors;
        if (problem.severity == UMI_DIAGNOSTIC_FATAL) ++fatal;
    }

    umi_diagnostic_query_init(&query);
    query.sort = RuntimeProblemsSort(runtime);
    query.filter.include_resolved =
        RuntimeProblemsIncludeResolved(runtime);
    RuntimeProblemsApplySeverity(runtime, &query.filter);
    RuntimeProblemsApplyText(runtime, &query.filter);

    /*
     * Query in Framework-sized pages while keeping the native surface bounded
     * to the same 300-row presentation limit used by the previous Studio view.
     */
    while (shown < 300U) {
        query.offset = offset;
        query.limit = UMI_DIAGNOSTIC_QUERY_RESULT_MAX;
        status = umi_diagnostic_query_execute(
            model, &query, &result);
        if (status != UMI_STATUS_OK) {
            if (runtime->problems_summary_label != NULL) {
                gtk_label_set_text(
                    GTK_LABEL(runtime->problems_summary_label),
                    umi_status_text(status));
            }
            return;
        }

        if (offset == 0U) {
            totalMatches = result.total_matches;
        }

        for (size_t index = 0U;
             index < result.count && shown < 300U;
             ++index) {
            const UmiDiagnosticSnapshot *problem =
                &result.items[index];
            GtkWidget *row = gtk_list_box_row_new();
            GtkWidget *box = gtk_box_new(
                GTK_ORIENTATION_VERTICAL, 2);
            char *heading;
            GtkWidget *severity;

            if (row == NULL || box == NULL) continue;

            heading = g_strdup_printf(
                "%s%s — %s%s%s",
                problem->resolved ? "Resolved — " : "",
                umi_diagnostic_severity_text(problem->severity),
                problem->source,
                problem->code[0] != '\0' ? " — " : "",
                problem->code);
            severity = runtime_label(
                heading != NULL ? heading : "", 0.0F);
            g_free(heading);

            if (severity != NULL) {
                gtk_widget_add_css_class(
                    severity, "umicom-runtime-muted");
                gtk_box_append(GTK_BOX(box), severity);
            }

            gtk_box_append(
                GTK_BOX(box),
                runtime_label(problem->message, 0.0F));

            if (problem->uri[0] != '\0') {
                char *location = g_strdup_printf(
                    "%s:%u:%u",
                    problem->uri,
                    (unsigned)problem->line,
                    (unsigned)problem->column);
                GtkWidget *label = runtime_label(
                    location != NULL ? location : "", 0.0F);
                if (location != NULL) {
                    gtk_widget_set_tooltip_text(row, location);
                }
                if (label != NULL) {
                    gtk_box_append(GTK_BOX(box), label);
                }
                g_free(location);
            }

            if (problem->detail[0] != '\0') {
                gtk_widget_set_tooltip_text(
                    box, problem->detail);
            }

            gtk_list_box_row_set_child(
                GTK_LIST_BOX_ROW(row), box);
            g_object_set_data_full(
                G_OBJECT(row),
                "umicom-problem-id",
                g_strdup(problem->id),
                g_free);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->problems_list),
                row);

            if (strcmp(selected, problem->id) == 0) {
                gtk_list_box_select_row(
                    GTK_LIST_BOX(runtime->problems_list),
                    GTK_LIST_BOX_ROW(row));
            }
            ++shown;
        }

        if (!result.truncated || result.count == 0U) break;
        offset += result.count;
    }

    if (shown == 0U) {
        gtk_list_box_append(
            GTK_LIST_BOX(runtime->problems_list),
            runtime_label(
                active == 0U && !RuntimeProblemsIncludeResolved(runtime)
                    ? "No active problems. Build a project to collect compiler messages."
                    : "No problems match the current filter.",
                0.0F));
    }

    (void)snprintf(
        text,
        sizeof text,
        "%zu matching / %zu active — %zu warnings, %zu errors, "
        "%zu fatal — %zu resolved — showing %zu",
        totalMatches,
        active,
        warnings,
        errors,
        fatal,
        resolved,
        shown);
    if (runtime->problems_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->problems_summary_label),
            text);
    }

    runtime->problemsModelRevision = revision;
    runtime->problemsRendered = 1;
}

/* Search and filter state changes do not mutate the diagnostics model, so force
 * one presentation refresh even when the Framework revision is unchanged. */
static void RuntimeProblemsSearchChanged(
    GtkSearchEntry *entry,
    gpointer userData)
{
    UmiStudioGtkRuntimeChrome *runtime = userData;
    (void)entry;
    if (runtime == NULL) return;
    runtime->problemsRendered = 0;
    runtime_problems_refresh_list(runtime);
}

static void RuntimeProblemsChoiceChanged(
    GObject *object,
    GParamSpec *parameter,
    gpointer userData)
{
    UmiStudioGtkRuntimeChrome *runtime = userData;
    (void)object;
    (void)parameter;
    if (runtime == NULL) return;
    runtime->problemsRendered = 0;
    runtime_problems_refresh_list(runtime);
}

static void RuntimeProblemsResolvedChanged(
    GtkCheckButton *button,
    gpointer userData)
{
    UmiStudioGtkRuntimeChrome *runtime = userData;
    (void)button;
    if (runtime == NULL) return;
    runtime->problemsRendered = 0;
    runtime_problems_refresh_list(runtime);
}

/* Release or reset state held by runtime on problems so the same storage can be reused
 * safely. */
static void runtime_on_problems_clear(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL) return;
    (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_DIAGNOSTICS_CLEAR, NULL);
    runtime->problemsRendered = 0;
    runtime_problems_refresh_list(runtime);
}
