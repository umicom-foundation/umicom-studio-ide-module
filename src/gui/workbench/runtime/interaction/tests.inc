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

static UmiTestPlatformService *runtime_test_platform(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioTestService *tests;

    if (runtime == NULL || runtime->owner == NULL) return NULL;

    services = umi_studio_ui_services(runtime->owner->ui);
    tests = services != NULL
        ? umi_studio_services_tests(services)
        : NULL;

    return tests != NULL
        ? umi_studio_test_service_platform(tests)
        : NULL;
}

static void runtime_test_select(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *test_id)
{
    if (runtime == NULL || test_id == NULL || test_id[0] == '\0') return;

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

static void runtime_on_test_run(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *test_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-test-id");

    if (runtime == NULL || test_id == NULL) return;

    runtime_test_select(runtime, test_id);
    if (strcmp(runtime->selected_test_id, test_id) == 0) {
        (void)runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED,
            NULL);
    }
}

static void runtime_on_test_debug(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *test_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-test-id");

    if (runtime == NULL || test_id == NULL) return;

    runtime_test_select(runtime, test_id);
    if (strcmp(runtime->selected_test_id, test_id) == 0) {
        (void)runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_DEBUG_SELECTED,
            NULL);
    }
}

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

    if (runtime == NULL || uri == NULL || uri[0] == '\0') return;

    if (g_str_has_prefix(uri, "file:")) {
        file = g_file_new_for_uri(uri);
        if (file == NULL) return;
        path = g_file_get_path(file);
        g_object_unref(file);
    } else {
        path = g_strdup(uri);
    }

    if (path != NULL) {
        runtime_explorer_open_path(runtime, path);

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

    if (runtime == NULL || runtime->test_list == NULL) return;

    runtime_clear_list_box(runtime->test_list);
    platform = runtime_test_platform(runtime);
    items = platform != NULL
        ? umi_test_platform_service_item(platform)
        : NULL;

    if (runtime->test_search_entry != NULL) {
        query = gtk_editable_get_text(
            GTK_EDITABLE(runtime->test_search_entry));
    }

    count = items != NULL
        ? umi_test_platform_item_registry_count(items)
        : 0U;

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
        if (umi_test_platform_item_registry_at(
                items,
                index,
                &item) != UMI_STATUS_OK) {
            continue;
        }

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

    if (runtime->test_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->test_summary_label),
            summary);
    }
}

static void runtime_on_test_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    (void)entry;
    runtime_tests_refresh_list(
        (UmiStudioGtkRuntimeChrome *)user_data);
}
