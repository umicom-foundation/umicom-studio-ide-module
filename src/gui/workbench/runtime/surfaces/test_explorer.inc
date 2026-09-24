/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/test_explorer.inc
 *
 * PURPOSE:
 *   Build the operational Framework Test Platform explorer surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_tests_discover_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    {
        UmiStudioGtkRuntimeChrome *runtime =
            (UmiStudioGtkRuntimeChrome *)user_data;
        const char *build_directory =
            runtime != NULL &&
            runtime->test_snapshot.build_directory[0] != '\0'
                ? runtime->test_snapshot.build_directory
                : NULL;
        (void)runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_DISCOVER,
            build_directory);
    }
}

/*
 * Provide the runtime on tests all button operation used by this module and its client
 * applications.
 */
static void runtime_on_tests_all_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_TESTS_RUN_ALL,
        NULL);
}

/*
 * Provide the runtime on tests failed button operation used by this module and its client
 * applications.
 */
static void runtime_on_tests_failed_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED,
        NULL);
}

/*
 * Provide the runtime on tests stop button operation used by this module and its client
 * applications.
 */
static void runtime_on_tests_stop_button(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_TESTS_STOP,
        NULL);
}

/*
 * The previous surface composition is retained for review. The replacement
 * keeps every existing command and adds view controls backed by Framework's
 * Test Explorer model rather than adding another Studio test model.
 */
#if 0
static GtkWidget *runtime_build_surface_test_explorer(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Test Explorer");
    GtkWidget *actions;
    GtkWidget *discover;
    GtkWidget *run_all;
    GtkWidget *rerun_failed;
    GtkWidget *stop;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    runtime->test_summary_label = runtime_label("", 0.0F);
    runtime->test_search_entry = gtk_search_entry_new();
    runtime->test_list = gtk_list_box_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->test_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->test_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->test_summary_label);
    }

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    discover = gtk_button_new_with_label("Discover");
    run_all = gtk_button_new_with_label("Run All");
    rerun_failed = gtk_button_new_with_label("Failed");
    stop = gtk_button_new_with_label("Stop");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (actions != NULL && discover != NULL &&
        run_all != NULL && rerun_failed != NULL && stop != NULL) {
        g_signal_connect(
            discover, "clicked",
            G_CALLBACK(runtime_on_tests_discover_button), runtime);
        g_signal_connect(
            run_all, "clicked",
            G_CALLBACK(runtime_on_tests_all_button), runtime);
        g_signal_connect(
            rerun_failed, "clicked",
            G_CALLBACK(runtime_on_tests_failed_button), runtime);
        g_signal_connect(
            stop, "clicked",
            G_CALLBACK(runtime_on_tests_stop_button), runtime);

        gtk_box_append(GTK_BOX(actions), discover);
        gtk_box_append(GTK_BOX(actions), run_all);
        gtk_box_append(GTK_BOX(actions), rerun_failed);
        gtk_box_append(GTK_BOX(actions), stop);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->test_search_entry != NULL) {
        g_object_set(G_OBJECT(runtime->test_search_entry), "placeholder-text", "Filter tests, labels or ids", NULL);
        g_signal_connect(
            runtime->test_search_entry,
            "search-changed",
            G_CALLBACK(runtime_on_test_search_changed),
            runtime);
        gtk_box_append(
            GTK_BOX(panel),
            runtime->test_search_entry);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->test_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->test_list),
            GTK_SELECTION_NONE);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->test_list));
    }

    {
        GtkWidget *details = runtime_test_details_notebook(runtime);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (details != NULL) {
            gtk_box_append(GTK_BOX(panel), details);
        }
    }

    runtime_append_context_actions(runtime, panel, "tests");

    runtime_tests_refresh_list(runtime);
    runtime_test_details_refresh(runtime);
    return panel;
}
#endif

/*
 * Build the Test Explorer using Framework-owned grouping, sorting and filtering
 * while retaining the existing Discover/Run/Debug/Source execution paths.
 */
static GtkWidget *runtime_build_surface_test_explorer(
    UmiStudioGtkRuntimeChrome *runtime)
{
    static const char *const groupOptions[] = {
        "Hierarchy",
        "Suite",
        "Framework",
        "Outcome",
        NULL
    };
    static const char *const sortOptions[] = {
        "Name",
        "Outcome",
        "Duration",
        "Source",
        "Framework",
        NULL
    };
    GtkWidget *panel = runtime_panel_new("Test Explorer");
    GtkWidget *actions;
    GtkWidget *discover;
    GtkWidget *run_all;
    GtkWidget *rerun_failed;
    GtkWidget *stop;
    GtkWidget *viewOptions;
    GtkWidget *groupLabel;
    GtkWidget *groupSelector;
    GtkWidget *sortLabel;
    GtkWidget *sortSelector;
    GtkWidget *failedOnly;

    if (panel == NULL || runtime == NULL) return panel;

    runtime->test_summary_label = runtime_label("", 0.0F);
    runtime->test_search_entry = gtk_search_entry_new();
    runtime->test_list = gtk_list_box_new();

    if (runtime->test_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->test_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->test_summary_label);
    }

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    discover = gtk_button_new_with_label("Discover");
    run_all = gtk_button_new_with_label("Run All");
    rerun_failed = gtk_button_new_with_label("Failed");
    stop = gtk_button_new_with_label("Stop");

    if (actions != NULL && discover != NULL &&
        run_all != NULL && rerun_failed != NULL && stop != NULL) {
        g_signal_connect(
            discover, "clicked",
            G_CALLBACK(runtime_on_tests_discover_button), runtime);
        g_signal_connect(
            run_all, "clicked",
            G_CALLBACK(runtime_on_tests_all_button), runtime);
        g_signal_connect(
            rerun_failed, "clicked",
            G_CALLBACK(runtime_on_tests_failed_button), runtime);
        g_signal_connect(
            stop, "clicked",
            G_CALLBACK(runtime_on_tests_stop_button), runtime);

        gtk_widget_add_css_class(actions, "umicom-runtime-panel-actions");
        gtk_box_append(GTK_BOX(actions), discover);
        gtk_box_append(GTK_BOX(actions), run_all);
        gtk_box_append(GTK_BOX(actions), rerun_failed);
        gtk_box_append(GTK_BOX(actions), stop);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    viewOptions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    groupLabel = runtime_label("Group", 0.0F);
    groupSelector = gtk_drop_down_new_from_strings(groupOptions);
    sortLabel = runtime_label("Sort", 0.0F);
    sortSelector = gtk_drop_down_new_from_strings(sortOptions);
    failedOnly = gtk_check_button_new_with_label("Failed only");

    if (viewOptions != NULL && groupLabel != NULL &&
        groupSelector != NULL && sortLabel != NULL &&
        sortSelector != NULL && failedOnly != NULL &&
        runtime->test_list != NULL) {
        gtk_drop_down_set_selected(
            GTK_DROP_DOWN(groupSelector),
            0U);
        gtk_drop_down_set_selected(
            GTK_DROP_DOWN(sortSelector),
            0U);

        g_object_set_data(
            G_OBJECT(runtime->test_list),
            "umicom-test-group-selector",
            groupSelector);
        g_object_set_data(
            G_OBJECT(runtime->test_list),
            "umicom-test-sort-selector",
            sortSelector);
        g_object_set_data(
            G_OBJECT(runtime->test_list),
            "umicom-test-failed-only",
            failedOnly);

        g_signal_connect(
            groupSelector,
            "notify::selected",
            G_CALLBACK(RuntimeTestViewChanged),
            runtime);
        g_signal_connect(
            sortSelector,
            "notify::selected",
            G_CALLBACK(RuntimeTestViewChanged),
            runtime);
        g_signal_connect(
            failedOnly,
            "toggled",
            G_CALLBACK(RuntimeTestFailedOnlyChanged),
            runtime);

        gtk_widget_set_tooltip_text(
            groupSelector,
            "Group discovered tests using the Framework Test Explorer model");
        gtk_widget_set_tooltip_text(
            sortSelector,
            "Sort the current Test Explorer projection");
        gtk_widget_set_tooltip_text(
            failedOnly,
            "Show only tests whose latest retained outcome failed or timed out");

        gtk_box_append(GTK_BOX(viewOptions), groupLabel);
        gtk_box_append(GTK_BOX(viewOptions), groupSelector);
        gtk_box_append(GTK_BOX(viewOptions), sortLabel);
        gtk_box_append(GTK_BOX(viewOptions), sortSelector);
        gtk_box_append(GTK_BOX(viewOptions), failedOnly);
        gtk_box_append(GTK_BOX(panel), viewOptions);
    }

    if (runtime->test_search_entry != NULL) {
        g_object_set(
            G_OBJECT(runtime->test_search_entry),
            "placeholder-text",
            "Filter tests, labels or ids",
            NULL);
        g_signal_connect(
            runtime->test_search_entry,
            "search-changed",
            G_CALLBACK(runtime_on_test_search_changed),
            runtime);
        gtk_box_append(
            GTK_BOX(panel),
            runtime->test_search_entry);
    }

    if (runtime->test_list != NULL) {
        GtkWidget *scroll;

        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->test_list),
            GTK_SELECTION_NONE);
        scroll = runtime_scroll(runtime->test_list);
        if (scroll != NULL) {
            gtk_box_append(GTK_BOX(panel), scroll);
        }
    }

    {
        GtkWidget *details = runtime_test_details_notebook(runtime);
        if (details != NULL) {
            gtk_box_append(GTK_BOX(panel), details);
        }
    }

    runtime_append_context_actions(runtime, panel, "tests");

    runtime_tests_refresh_list(runtime);
    runtime_test_details_refresh(runtime);
    return panel;
}
