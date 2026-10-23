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
#include "umicom/diagnostic_ui/page.h"
#include "umicom/ui/gtk4/list_publication.h"

/* Rows carry a retained diagnostic ID, not a mutable chronological index. */
/* Source navigation must reveal the selected editor, retaining the existing diagnostic-to-document resolver and its selection. Keep the earlier refresh-only callback for review. */
#if 0
static void RuntimeProblemsOpenSelected(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return;
    UmiStatus status = UmiStudioUiOpenProblem(runtime->owner->ui, runtime->problemsSelectedId);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "This problem has no available source location. Read Output for the complete message.");
    (void)runtime_refresh_services(runtime);
}


#endif
static UmiStatus RuntimeNavigateProblem(UmiStudioGtkRuntimeChrome *runtime, int previous)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = UmiStudioUiNavigateProblem(runtime->owner->ui, previous);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK) status = RuntimePresentActiveSource(runtime);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK) runtime_show_notification(runtime,
        "No available source location was opened. Read Problems and Output for details.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

static void RuntimeProblemsOpenSelected(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return;
    UmiStatus status = UmiStudioUiOpenProblem(runtime->owner->ui, runtime->problemsSelectedId);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status == UMI_STATUS_OK) status = RuntimePresentActiveSource(runtime);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "This problem has no available source location. Read Output for the complete message.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}

/* Raw runtime signal data is replaced by weak host and list bindings below. Retained for engineering review. */
#if 0
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


#endif
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
/* The former renderer read borrowed model/runtime state across native notifications. Immutable Framework pages and weak control bindings now replace that publication path. Retained for engineering review. */
#if 0
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

#endif
/* Immutable pages and the Framework native-list publisher keep presentation
 * separate from model access. Native notifications may retire this window or
 * replace its query; only a complete, still-current page grants row actions. */
static void RuntimeProblemsQuery(UmiStudioGtkRuntimeChrome *runtime, UmiDiagnosticQuery *query)
{
    umi_diagnostic_query_init(query);
    query->limit = UMI_DIAGNOSTIC_PAGE_LIMIT;
    query->sort = RuntimeProblemsSort(runtime);
    query->filter.include_resolved = RuntimeProblemsIncludeResolved(runtime);
    RuntimeProblemsApplySeverity(runtime, &query->filter);
    RuntimeProblemsApplyText(runtime, &query->filter);
}
static UmiDiagnosticModel *RuntimeProblemsModel(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    return services != NULL ? umi_diagnostic_pipeline_model(
        umi_studio_services_diagnostic_pipeline(services)) : NULL;
}
static void RuntimeProblemsPageFree(gpointer data)
{ UmiDiagnosticPageDestroy(data); }
static UmiStatus RuntimeProblemsTokenCurrent(UmiStudioGtkRuntimeChrome *runtime, GObject *token)
{
    if (token == NULL || g_object_get_data(token, "query-generation") !=
        g_object_get_data(G_OBJECT(runtime->problems_list), "studio-problems-query"))
        return UMI_STATUS_INVALID_STATE;
    UmiDiagnosticQuery query;
    RuntimeProblemsQuery(runtime, &query);
    return UmiDiagnosticPageValidate(g_object_get_data(token, "page"), RuntimeProblemsModel(runtime), &query);
}
static int RuntimeProblemsRowCurrent(UmiStudioGtkRuntimeChrome *runtime, GtkListBoxRow *row)
{
    if (runtime == NULL || runtime->problems_list == NULL || row == NULL ||
        gtk_widget_get_parent(GTK_WIDGET(row)) != runtime->problems_list) return 0;
    GObject *token = g_object_get_data(G_OBJECT(row), "studio-problem-page");
    return token != NULL && token == g_object_get_data(G_OBJECT(runtime->problems_list), "studio-problems-page") &&
        RuntimeProblemsTokenCurrent(runtime, token) == UMI_STATUS_OK;
}
typedef struct RuntimeProblemsPublication {
    UmiStudioFileDialogRequest *owner;
    GtkWidget *list;
    GObject *token;
    GtkWidget *summary;
} RuntimeProblemsPublication;
static UmiStatus RuntimeProblemsPublicationCurrent(GtkListBox *list, void *context)
{
    RuntimeProblemsPublication *capture = context;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(capture->owner, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) &&
        runtime->problems_list == capture->list && GTK_WIDGET(list) == capture->list &&
        runtime->problems_summary_label == capture->summary)
        status = RuntimeProblemsTokenCurrent(runtime, capture->token);
    g_clear_object(&window);
    return status;
}
static GtkWidget *RuntimeProblemsPrepareRow(const UmiDiagnosticSnapshot *problem, GObject *token)
{
    GtkWidget *row = gtk_list_box_row_new();
    g_object_ref_sink(row);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    char *heading = g_strdup_printf("%s%s — %s%s%s", problem->resolved ? "Resolved — " : "",
        umi_diagnostic_severity_text(problem->severity), problem->source,
        problem->code[0] != '\0' ? " — " : "", problem->code);
    GtkWidget *severity = runtime_label(heading, 0.0F);
    g_free(heading);
    gtk_widget_add_css_class(severity, "umicom-runtime-muted");
    gtk_box_append(GTK_BOX(box), severity);
    gtk_box_append(GTK_BOX(box), runtime_label(problem->message, 0.0F));
    if (problem->uri[0] != '\0') {
        char *location = g_strdup_printf("%s:%u:%u", problem->uri,
            (unsigned)problem->line, (unsigned)problem->column);
        gtk_widget_set_tooltip_text(row, location);
        gtk_box_append(GTK_BOX(box), runtime_label(location, 0.0F));
        g_free(location);
    }
    if (problem->detail[0] != '\0') gtk_widget_set_tooltip_text(box, problem->detail);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    g_object_set_data_full(G_OBJECT(row), "umicom-problem-id", g_strdup(problem->id), g_free);
    g_object_set_data_full(G_OBJECT(row), "studio-problem-page", g_object_ref(token), g_object_unref);
    return row;
}
static void runtime_problems_refresh_list(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->problems_list == NULL) return;
    GtkWidget *list = runtime->problems_list;
    if (g_object_get_data(G_OBJECT(list), "studio-problems-refreshing") != NULL) return;
    GObject *old = g_object_get_data(G_OBJECT(list), "studio-problems-page");
    if (runtime->problemsRendered && RuntimeProblemsTokenCurrent(runtime, old) == UMI_STATUS_OK) return;
    RuntimeProblemsPublication capture = {0};
    capture.owner = runtime_file_dialog_request_new(runtime, 0U);
    if (capture.owner == NULL) return;
    capture.list = g_object_ref(list);
    capture.summary = runtime->problems_summary_label != NULL ? g_object_ref(runtime->problems_summary_label) : NULL;
    GObject *window = NULL;
    GPtrArray *rows = NULL;
    UmiDiagnosticPage *page = NULL;
    UmiDiagnosticQuery query;
    UmiDiagnosticPageSummary summary;
    UmiStatus status = UMI_STATUS_CANCELLED;
    char selected[UMI_DIAGNOSTIC_ID_CAPACITY];
    g_strlcpy(selected, runtime->problemsSelectedId, sizeof selected);
    /* The query token records even an edit away from and back to equal text.
     * Do not reset it on a timer refresh: it belongs to control changes. */
    GObject *queryGeneration = g_object_get_data(G_OBJECT(list), "studio-problems-query");
    if (queryGeneration == NULL) {
        queryGeneration = g_object_new(G_TYPE_OBJECT, NULL);
        g_object_set_data_full(G_OBJECT(list), "studio-problems-query", queryGeneration, g_object_unref);
    }
    g_object_set_data(G_OBJECT(list), "studio-problems-refreshing", &capture);
    RuntimeProblemsQuery(runtime, &query);
    status = UmiDiagnosticPageCreate(RuntimeProblemsModel(runtime), &query, &page);
    if (status != UMI_STATUS_OK) goto done;
    capture.token = g_object_new(G_TYPE_OBJECT, NULL);
    g_object_set_data_full(capture.token, "page", page, RuntimeProblemsPageFree);
    g_object_set_data_full(capture.token, "query-generation", g_object_ref(queryGeneration), g_object_unref);
    (void)UmiDiagnosticPageReadSummary(page, &summary);
    rows = g_ptr_array_new_with_free_func(g_object_unref);
    for (size_t index = 0U; index < summary.count; ++index)
        g_ptr_array_add(rows, RuntimeProblemsPrepareRow(UmiDiagnosticPageItem(page, index), capture.token));
    if (summary.count == 0U) {
        GtkWidget *empty = runtime_label(summary.active == 0U && !query.filter.include_resolved
            ? "No active problems. Build a project to collect compiler messages."
            : "No problems match the current filter.", 0.0F);
        g_ptr_array_add(rows, g_object_ref_sink(empty));
    }
    status = RuntimeProblemsPublicationCurrent(GTK_LIST_BOX(list), &capture);
    if (status != UMI_STATUS_OK) goto done;
    runtime = RuntimeResolveOwner(capture.owner, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    runtime->problemsRendered = 0;
    runtime->problemsSelectedId[0] = '\0';
    g_object_set_data(G_OBJECT(list), "studio-problems-page", NULL);
    status = UmiGtk4ListBoxPublish(GTK_LIST_BOX(list), (GtkWidget *const *)rows->pdata, rows->len,
        RuntimeProblemsPublicationCurrent, &capture);
    if (status != UMI_STATUS_OK) goto done;
    runtime = RuntimeResolveOwner(capture.owner, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    char text[320];
    snprintf(text, sizeof text,
        "%zu matching / %zu active — %zu warnings, %zu errors, %zu fatal — %zu resolved — showing %zu",
        summary.total_matches, summary.active, summary.warnings, summary.errors, summary.fatal,
        summary.resolved, summary.count);
    if (capture.summary != NULL)
        gtk_label_set_text(GTK_LABEL(capture.summary), text);
    status = RuntimeProblemsPublicationCurrent(GTK_LIST_BOX(list), &capture);
    if (status != UMI_STATUS_OK) goto done;
    g_object_set_data_full(G_OBJECT(list), "studio-problems-page", g_object_ref(capture.token), g_object_unref);
    for (guint index = 0U; index < rows->len; ++index) {
        GtkWidget *row = g_ptr_array_index(rows, index);
        const char *id = g_object_get_data(G_OBJECT(row), "umicom-problem-id");
        if (id != NULL && strcmp(selected, id) == 0) {
            gtk_list_box_select_row(GTK_LIST_BOX(list), GTK_LIST_BOX_ROW(row));
            break;
        }
    }
    status = RuntimeProblemsPublicationCurrent(GTK_LIST_BOX(list), &capture);
    if (status != UMI_STATUS_OK) goto done;
    runtime = RuntimeResolveOwner(capture.owner, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    runtime->problemsModelRevision = summary.revision;
    runtime->problemsRendered = 1;
 done:
    if (status != UMI_STATUS_OK) {
        /* Retained or partial rows remain visible only as non-actionable
         * evidence until a later timer/control refresh captures current data. */
        g_object_set_data(G_OBJECT(list), "studio-problems-page", NULL);
        runtime = RuntimeResolveOwner(capture.owner, &window);
        if (runtime != NULL && runtime->problems_list == list) {
            runtime->problemsRendered = 0;
            runtime->problemsSelectedId[0] = '\0';
        }
    }
    g_object_set_data(G_OBJECT(list), "studio-problems-refreshing", NULL);
    g_clear_object(&window);
    if (rows != NULL) g_ptr_array_unref(rows);
    g_clear_object(&capture.token);
    g_clear_object(&capture.summary);
    g_object_unref(list);
    runtime_file_dialog_request_free(capture.owner);
}

/* Signals retain only a weak host/list binding, never the runtime allocation.
 * This also makes externally retained controls harmless after pane replacement. */
typedef struct RuntimeProblemsBinding {
    UmiStudioFileDialogRequest *owner;
    GWeakRef list;
} RuntimeProblemsBinding;
static void RuntimeProblemsBindingFree(gpointer data, GClosure *closure)
{
    RuntimeProblemsBinding *binding = data;
    (void)closure;
    runtime_file_dialog_request_free(binding->owner);
    g_weak_ref_clear(&binding->list);
    g_free(binding);
}
static UmiStudioGtkRuntimeChrome *RuntimeProblemsBindingOwner(RuntimeProblemsBinding *binding,
    GObject **window, GObject **list)
{
    *list = g_weak_ref_get(&binding->list);
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, window);
    return runtime != NULL && *list != NULL && runtime->problems_list == GTK_WIDGET(*list) &&
        gtk_widget_get_root(GTK_WIDGET(*list)) == GTK_ROOT(*window) ? runtime : NULL;
}
static void RuntimeProblemsBind(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *widget,
    const char *signal, GCallback callback, const char *id)
{
    RuntimeProblemsBinding *binding = g_new0(RuntimeProblemsBinding, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL) { g_free(binding); gtk_widget_set_sensitive(widget, FALSE); return; }
    g_weak_ref_init(&binding->list, G_OBJECT(runtime->problems_list));
    if (id != NULL) g_object_set_data_full(G_OBJECT(widget), "umicom-automation-id", g_strdup(id), g_free);
    g_signal_connect_data(widget, signal, callback, binding, RuntimeProblemsBindingFree, 0);
}
static void RuntimeProblemsSelected(GtkListBox *sender, GtkListBoxRow *row, gpointer data)
{
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(data, &window, &list);
    if (runtime != NULL && GTK_WIDGET(sender) == runtime->problems_list) {
        const char *id = RuntimeProblemsRowCurrent(runtime, row) ? g_object_get_data(G_OBJECT(row), "umicom-problem-id") : NULL;
        g_strlcpy(runtime->problemsSelectedId, id != NULL ? id : "", sizeof runtime->problemsSelectedId);
    }
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeProblemsActivated(GtkListBox *sender, GtkListBoxRow *row, gpointer data)
{
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(data, &window, &list);
    if (runtime != NULL && GTK_WIDGET(sender) == runtime->problems_list && RuntimeProblemsRowCurrent(runtime, row)) {
        g_strlcpy(runtime->problemsSelectedId, g_object_get_data(G_OBJECT(row), "umicom-problem-id"), sizeof runtime->problemsSelectedId);
        RuntimeProblemsOpenSelected(runtime);
    }
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeProblemsOpenClicked(GtkButton *button, gpointer data)
{
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(data, &window, &list);
    (void)button;
    if (runtime != NULL && RuntimeProblemsRowCurrent(runtime, gtk_list_box_get_selected_row(GTK_LIST_BOX(list))))
        RuntimeProblemsOpenSelected(runtime);
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeProblemsFilterChanged(gpointer data)
{
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(data, &window, &list);
    if (runtime != NULL) {
        g_object_set_data_full(list, "studio-problems-query", g_object_new(G_TYPE_OBJECT, NULL), g_object_unref);
        g_object_set_data(list, "studio-problems-page", NULL);
        runtime->problemsRendered = 0;
        /* The captured ID may still match the next filtered page. Its action
         * token is already revoked, so retaining selection cannot open it. */
        runtime_problems_refresh_list(runtime);
    }
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeProblemsSearchChanged(GtkEditable *entry, gpointer data)
{ (void)entry; RuntimeProblemsFilterChanged(data); }
static void RuntimeProblemsChoiceChanged(GObject *object, GParamSpec *property, gpointer data)
{ (void)object; (void)property; RuntimeProblemsFilterChanged(data); }
static void RuntimeProblemsResolvedChanged(GtkCheckButton *button, gpointer data)
{ (void)button; RuntimeProblemsFilterChanged(data); }
static void runtime_on_problems_clear(GtkButton *button, gpointer data)
{
    RuntimeProblemsBinding *binding = data;
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(binding, &window, &list);
    (void)button;
    if (runtime != NULL) {
        /* Command dispatch may notify observers which close Studio. */
        UmiStudioFileDialogRequest *owner = runtime_file_dialog_request_new(runtime, 0U);
        if (owner != NULL) {
            (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_DIAGNOSTICS_CLEAR, NULL);
            runtime = RuntimeResolveOwner(owner, &window);
            if (runtime != NULL && runtime->problems_list == GTK_WIDGET(list)) {
                runtime->problemsRendered = 0;
                runtime_problems_refresh_list(runtime);
            }
            runtime_file_dialog_request_free(owner);
        }
    }
    g_clear_object(&list); g_clear_object(&window);
}
/* Clear borrowed fields when this particular list is retired; the weak host
 * request remains valid even if teardown already released runtime_chrome. */
typedef struct RuntimeProblemsListOwner {
    UmiStudioFileDialogRequest *owner;
    GtkWidget *list;
} RuntimeProblemsListOwner;
static void RuntimeProblemsListReleased(gpointer data)
{
    RuntimeProblemsListOwner *binding = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    if (runtime != NULL && runtime->problems_list == binding->list) {
        runtime->problems_list = NULL;
        runtime->problems_summary_label = NULL;
        runtime->problemsRendered = 0;
        runtime->problemsSelectedId[0] = '\0';
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
