/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/tests.inc
 *
 * PURPOSE:
 *   Render the Framework Test Platform item registry as an operational Test
 *   Explorer with select, run, debug and source navigation actions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/test_platform/explorer_model.h"

static UmiTestPlatformService *runtime_test_platform(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioTestService *tests;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) return NULL;

    services = umi_studio_ui_services(runtime->owner->ui);
    tests = services != NULL
        ? umi_studio_services_tests(services)
        : NULL;

    return tests != NULL
        ? umi_studio_test_service_platform(tests)
        : NULL;
}

/*
 * Provide the runtime test select operation used by this module and its client
 * applications.
 */
static void runtime_test_select(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *test_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || test_id == NULL || test_id[0] == '\0') return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_SELECT,
            test_id) == UMI_STATUS_OK) {
        runtime_copy_selection(
            runtime->selected_test_id,
            sizeof(runtime->selected_test_id),
            test_id);
    }
}

/*
 * Provide the runtime on test select operation used by this module and its client
 * applications.
 */
static void runtime_on_test_select(
    GtkButton *button,
    gpointer user_data)
{
    const char *test_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-test-id");

    runtime_test_select(
        (UmiStudioGtkRuntimeChrome *)user_data,
        test_id);
}

/*
 * Perform runtime on test through the module contract so client applications do not
 * duplicate its policy.
 */
static void runtime_on_test_run(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *test_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-test-id");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || test_id == NULL) return;

    runtime_test_select(runtime, test_id);
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(runtime->selected_test_id, test_id) == 0) {
        (void)runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED,
            NULL);
    }
}

/*
 * Provide the runtime on test debug operation used by this module and its client
 * applications.
 */
static void runtime_on_test_debug(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *test_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-test-id");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || test_id == NULL) return;

    runtime_test_select(runtime, test_id);
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(runtime->selected_test_id, test_id) == 0) {
        (void)runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_DEBUG_SELECTED,
            NULL);
    }
}

/*
 * Provide the runtime on test source operation used by this module and its client
 * applications.
 */
static void runtime_on_test_source(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *uri = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-source-uri");
    const guint source_line = GPOINTER_TO_UINT(
        g_object_get_data(
            G_OBJECT(button),
            "umicom-source-line"));
    GFile *file;
    char *path;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || uri == NULL || uri[0] == '\0') return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (g_str_has_prefix(uri, "file:")) {
        file = g_file_new_for_uri(uri);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (file == NULL) return;
        path = g_file_get_path(file);
        g_object_unref(file);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        path = g_strdup(uri);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path != NULL) {
        runtime_explorer_open_path(runtime, path);

        /* Apply this branch only when its contract condition is satisfied. */
        if (source_line > 0U) {
            size_t offset = 0U;
            (void)umi_document_coordinator_go_to_line(
                umi_studio_ui_documents(runtime->owner->ui),
                (size_t)source_line,
                &offset);
        }

        g_free(path);
    }
}

/*
 * The previous Studio-local Test Explorer projection is retained for review.
 * Framework now owns filtering, grouping, sorting, latest-result selection and
 * outcome totals through UmiTestExplorerModel.
 */
#if 0
static void runtime_tests_refresh_list(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiTestPlatformService *platform;
    UmiTestPlatformItemRegistry *items;
    size_t count;
    size_t index;
    size_t visible = 0U;
    const char *query = "";
    char summary[256];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->test_list == NULL) return;

    runtime_clear_list_box(runtime->test_list);
    platform = runtime_test_platform(runtime);
    items = platform != NULL
        ? umi_test_platform_service_item(platform)
        : NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->test_search_entry != NULL) {
        query = gtk_editable_get_text(
            GTK_EDITABLE(runtime->test_search_entry));
    }

    count = items != NULL
        ? umi_test_platform_item_registry_count(items)
        : 0U;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < count && visible < UMI_STUDIO_GTK_TEST_ROW_LIMIT;
         ++index) {
        UmiTestPlatformItemSnapshot item;
        GtkWidget *row;
        GtkWidget *labels;
        GtkWidget *title;
        GtkWidget *detail;
        GtkWidget *spacer;
        GtkWidget *select;
        GtkWidget *run;
        GtkWidget *debug;
        GtkWidget *source;
        char detail_text[768];

        (void)memset(&item, 0, sizeof(item));
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_test_platform_item_registry_at(
                items,
                index,
                &item) != UMI_STATUS_OK) {
            continue;
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (!runtime_text_contains_casefold(item.name, query) &&
            !runtime_text_contains_casefold(item.id, query) &&
            !runtime_text_contains_casefold(item.labels, query)) {
            continue;
        }

        row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        title = runtime_label(item.name, 0.0F);
        detail = runtime_label("", 0.0F);
        spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        select = gtk_button_new_with_label("Select");
        run = gtk_button_new_with_label("Run");
        debug = gtk_button_new_with_label("Debug");
        source = gtk_button_new_with_label("Source");

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (row == NULL || labels == NULL || title == NULL ||
            detail == NULL || spacer == NULL || select == NULL ||
            run == NULL || debug == NULL || source == NULL) {
            continue;
        }

        (void)snprintf(
            detail_text,
            sizeof(detail_text),
            "%s — %s — %s — %.1f ms",
            item.framework,
            item.kind,
            item.labels,
            item.last_duration_ms);
        gtk_label_set_text(GTK_LABEL(detail), detail_text);
        gtk_widget_add_css_class(detail, "umicom-runtime-muted");
        gtk_widget_set_hexpand(spacer, TRUE);

        g_object_set_data_full(
            G_OBJECT(select),
            "umicom-test-id",
            g_strdup(item.id),
            g_free);
        g_signal_connect(
            select,
            "clicked",
            G_CALLBACK(runtime_on_test_select),
            runtime);

        g_object_set_data_full(
            G_OBJECT(run),
            "umicom-test-id",
            g_strdup(item.id),
            g_free);
        g_signal_connect(
            run,
            "clicked",
            G_CALLBACK(runtime_on_test_run),
            runtime);

        g_object_set_data_full(
            G_OBJECT(debug),
            "umicom-test-id",
            g_strdup(item.id),
            g_free);
        g_signal_connect(
            debug,
            "clicked",
            G_CALLBACK(runtime_on_test_debug),
            runtime);

        g_object_set_data_full(
            G_OBJECT(source),
            "umicom-source-uri",
            g_strdup(item.source_uri),
            g_free);
        g_object_set_data(
            G_OBJECT(source),
            "umicom-source-line",
            GUINT_TO_POINTER(item.source_line));
        g_signal_connect(
            source,
            "clicked",
            G_CALLBACK(runtime_on_test_source),
            runtime);

        gtk_widget_set_sensitive(run, item.enabled != 0);
        gtk_widget_set_sensitive(debug, item.enabled != 0);
        gtk_widget_set_sensitive(
            source,
            item.source_uri[0] != '\0');

        gtk_box_append(GTK_BOX(labels), title);
        gtk_box_append(GTK_BOX(labels), detail);
        gtk_box_append(GTK_BOX(row), labels);
        gtk_box_append(GTK_BOX(row), spacer);
        gtk_box_append(GTK_BOX(row), select);
        gtk_box_append(GTK_BOX(row), run);
        gtk_box_append(GTK_BOX(row), debug);
        gtk_box_append(GTK_BOX(row), source);

        gtk_list_box_append(GTK_LIST_BOX(runtime->test_list), row);
        ++visible;
    }

    (void)snprintf(
        summary,
        sizeof(summary),
        "%zu visible / %zu discovered — %zu passed, %zu failed%s",
        visible,
        count,
        runtime->test_snapshot.passed,
        runtime->test_snapshot.failed,
        runtime->test_snapshot.operation_running ? " — running" : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->test_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->test_summary_label),
            summary);
    }
}
#endif

/* Read the current grouping control without storing duplicate Test Explorer state. */
static UmiTestExplorerGroupMode RuntimeTestGroupMode(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *selector;
    guint selected;

    if (runtime == NULL || runtime->test_list == NULL) {
        return UMI_TEST_EXPLORER_GROUP_HIERARCHY;
    }

    selector = g_object_get_data(
        G_OBJECT(runtime->test_list),
        "umicom-test-group-selector");
    if (selector == NULL || !GTK_IS_DROP_DOWN(selector)) {
        return UMI_TEST_EXPLORER_GROUP_HIERARCHY;
    }

    selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(selector));
    if (selected > 3U) selected = 0U;
    return (UmiTestExplorerGroupMode)(selected + 1U);
}

/* Map the visible sort choice to the Framework enum explicitly. */
static UmiTestExplorerSortMode RuntimeTestSortMode(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *selector;
    guint selected;

    if (runtime == NULL || runtime->test_list == NULL) {
        return UMI_TEST_EXPLORER_SORT_NAME;
    }

    selector = g_object_get_data(
        G_OBJECT(runtime->test_list),
        "umicom-test-sort-selector");
    if (selector == NULL || !GTK_IS_DROP_DOWN(selector)) {
        return UMI_TEST_EXPLORER_SORT_NAME;
    }

    selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(selector));
    switch (selected) {
    case 1U: return UMI_TEST_EXPLORER_SORT_OUTCOME;
    case 2U: return UMI_TEST_EXPLORER_SORT_DURATION;
    case 3U: return UMI_TEST_EXPLORER_SORT_SOURCE;
    case 4U: return UMI_TEST_EXPLORER_SORT_FRAMEWORK;
    case 0U:
    default:
        return UMI_TEST_EXPLORER_SORT_NAME;
    }
}

/* Failed-only is presentation state; Framework evaluates the actual filter. */
static int RuntimeTestFailedOnly(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *toggle;

    if (runtime == NULL || runtime->test_list == NULL) return 0;

    toggle = g_object_get_data(
        G_OBJECT(runtime->test_list),
        "umicom-test-failed-only");
    return toggle != NULL && GTK_IS_CHECK_BUTTON(toggle)
        ? gtk_check_button_get_active(GTK_CHECK_BUTTON(toggle))
        : 0;
}

/* A changed grouping or sort choice rebuilds only the Test Explorer projection. */
static void RuntimeTestViewChanged(
    GObject *object,
    GParamSpec *parameter,
    gpointer userData)
{
    (void)object;
    (void)parameter;
    runtime_tests_refresh_list((UmiStudioGtkRuntimeChrome *)userData);
}

/* Keep the toggle callback signature native while reusing the same refresh path. */
static void RuntimeTestFailedOnlyChanged(
    GtkCheckButton *button,
    gpointer userData)
{
    (void)button;
    runtime_tests_refresh_list((UmiStudioGtkRuntimeChrome *)userData);
}

/*
 * Build the visible Test Explorer from the canonical Framework model. Studio
 * creates no second result history, filter engine, hierarchy or sort policy.
 */
static void runtime_tests_refresh_list(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiTestPlatformService *platform;
    UmiTestPlatformItemRegistry *items;
    UmiTestPlatformResultRegistry *results;
    UmiTestExplorerModel *model = NULL;
    UmiTestPlatformFilter filter;
    UmiTestExplorerModelSnapshot modelSnapshot;
    size_t visibleCount;
    size_t position;
    const char *query = "";
    char summary[320];
    UmiStatus status;

    if (runtime == NULL || runtime->test_list == NULL) return;

    runtime_clear_list_box(runtime->test_list);
    platform = runtime_test_platform(runtime);
    items = platform != NULL
        ? umi_test_platform_service_item(platform)
        : NULL;
    results = platform != NULL
        ? umi_test_platform_service_result(platform)
        : NULL;

    if (runtime->test_search_entry != NULL) {
        query = gtk_editable_get_text(
            GTK_EDITABLE(runtime->test_search_entry));
    }

    if (items == NULL || results == NULL) {
        if (runtime->test_summary_label != NULL) {
            gtk_label_set_text(
                GTK_LABEL(runtime->test_summary_label),
                "Test Platform is not available.");
        }
        return;
    }

    status = umi_test_explorer_model_create(&model);
    if (status != UMI_STATUS_OK) {
        if (runtime->test_summary_label != NULL) {
            gtk_label_set_text(
                GTK_LABEL(runtime->test_summary_label),
                umi_status_text(status));
        }
        return;
    }

    umi_test_platform_filter_init(&filter);
    runtime_copy_selection(
        filter.text,
        sizeof(filter.text),
        query);
    filter.include_disabled = 1;
    filter.failed_only = RuntimeTestFailedOnly(runtime);

    status = umi_test_explorer_model_set_filter(model, &filter);
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_model_set_group(
            model,
            RuntimeTestGroupMode(runtime));
    }
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_model_set_sort(
            model,
            RuntimeTestSortMode(runtime));
    }
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_model_rebuild(
            model,
            items,
            results);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_model_snapshot(
            model,
            &modelSnapshot);
    }

    if (status != UMI_STATUS_OK) {
        if (runtime->test_summary_label != NULL) {
            gtk_label_set_text(
                GTK_LABEL(runtime->test_summary_label),
                umi_status_text(status));
        }
        umi_test_explorer_model_destroy(model);
        return;
    }

    visibleCount = umi_test_explorer_model_visible_count(model);
    if (visibleCount > UMI_STUDIO_GTK_TEST_ROW_LIMIT) {
        visibleCount = UMI_STUDIO_GTK_TEST_ROW_LIMIT;
    }

    for (position = 0U; position < visibleCount; ++position) {
        UmiTestExplorerRow testRow;
        const UmiTestPlatformItemSnapshot *item;
        const char *outcomeText;
        double duration;
        GtkWidget *row;
        GtkWidget *labels;
        GtkWidget *title;
        GtkWidget *detail;
        GtkWidget *spacer;
        GtkWidget *select;
        GtkWidget *run;
        GtkWidget *debug;
        GtkWidget *source;
        char detailText[768];
        size_t depth;

        (void)memset(&testRow, 0, sizeof(testRow));
        if (umi_test_explorer_model_visible_row_at(
                model,
                position,
                &testRow) != UMI_STATUS_OK) {
            continue;
        }

        item = &testRow.item;
        outcomeText = testRow.has_result
            ? umi_test_platform_outcome_text(
                  (UmiTestPlatformOutcome)testRow.latest_result.outcome)
            : "not run";
        duration = testRow.has_result
            ? testRow.latest_result.duration_ms
            : item->last_duration_ms;

        row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        title = runtime_label(item->name, 0.0F);
        detail = runtime_label("", 0.0F);
        spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        select = gtk_button_new_with_label("Select");
        run = gtk_button_new_with_label("Run");
        debug = gtk_button_new_with_label("Debug");
        source = gtk_button_new_with_label("Source");

        if (row == NULL || labels == NULL || title == NULL ||
            detail == NULL || spacer == NULL || select == NULL ||
            run == NULL || debug == NULL || source == NULL) {
            continue;
        }

        depth = testRow.depth > 8U ? 8U : testRow.depth;
        gtk_widget_set_margin_start(labels, (int)(depth * 12U));
        if (testRow.child_count > 0U) {
            gtk_widget_add_css_class(
                title,
                "umicom-runtime-section-title");
        }

        (void)snprintf(
            detailText,
            sizeof(detailText),
            "%s — %s — %s — %s — %.1f ms",
            outcomeText,
            item->framework,
            item->kind,
            item->labels,
            duration);
        gtk_label_set_text(GTK_LABEL(detail), detailText);
        gtk_widget_add_css_class(detail, "umicom-runtime-muted");
        gtk_widget_set_hexpand(spacer, TRUE);

        g_object_set_data_full(
            G_OBJECT(select),
            "umicom-test-id",
            g_strdup(item->id),
            g_free);
        g_signal_connect(
            select,
            "clicked",
            G_CALLBACK(runtime_on_test_select),
            runtime);

        g_object_set_data_full(
            G_OBJECT(run),
            "umicom-test-id",
            g_strdup(item->id),
            g_free);
        g_signal_connect(
            run,
            "clicked",
            G_CALLBACK(runtime_on_test_run),
            runtime);

        g_object_set_data_full(
            G_OBJECT(debug),
            "umicom-test-id",
            g_strdup(item->id),
            g_free);
        g_signal_connect(
            debug,
            "clicked",
            G_CALLBACK(runtime_on_test_debug),
            runtime);

        g_object_set_data_full(
            G_OBJECT(source),
            "umicom-source-uri",
            g_strdup(item->source_uri),
            g_free);
        g_object_set_data(
            G_OBJECT(source),
            "umicom-source-line",
            GUINT_TO_POINTER(item->source_line));
        g_signal_connect(
            source,
            "clicked",
            G_CALLBACK(runtime_on_test_source),
            runtime);

        gtk_widget_set_sensitive(run, item->enabled != 0);
        gtk_widget_set_sensitive(debug, item->enabled != 0);
        gtk_widget_set_sensitive(
            source,
            item->source_uri[0] != '\0');

        gtk_box_append(GTK_BOX(labels), title);
        gtk_box_append(GTK_BOX(labels), detail);
        gtk_box_append(GTK_BOX(row), labels);
        gtk_box_append(GTK_BOX(row), spacer);
        gtk_box_append(GTK_BOX(row), select);
        gtk_box_append(GTK_BOX(row), run);
        gtk_box_append(GTK_BOX(row), debug);
        gtk_box_append(GTK_BOX(row), source);

        gtk_list_box_append(GTK_LIST_BOX(runtime->test_list), row);
    }

    if (modelSnapshot.visible_row_count == 0U) {
        gtk_list_box_append(
            GTK_LIST_BOX(runtime->test_list),
            runtime_label(
                filter.failed_only
                    ? "No failed tests match the current filter."
                    : "No tests match the current filter.",
                0.0F));
    }

    (void)snprintf(
        summary,
        sizeof(summary),
        "%zu visible / %zu discovered — %zu passed, %zu failed, "
        "%zu skipped, %zu not run%s",
        modelSnapshot.visible_row_count,
        modelSnapshot.row_count,
        modelSnapshot.passed_count,
        modelSnapshot.failed_count,
        modelSnapshot.skipped_count,
        modelSnapshot.not_run_count,
        runtime->test_snapshot.operation_running ? " — running" : "");

    if (runtime->test_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->test_summary_label),
            summary);
    }

    umi_test_explorer_model_destroy(model);
}

/*
 * Provide the runtime on test search changed operation used by this module and its client
 * applications.
 */
static void runtime_on_test_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    (void)entry;
    runtime_tests_refresh_list(
        (UmiStudioGtkRuntimeChrome *)user_data);
}
