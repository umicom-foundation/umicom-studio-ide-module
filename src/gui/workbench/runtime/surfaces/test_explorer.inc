/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/test_explorer.inc
 *
 * PURPOSE:
 *   Build the operational Framework Test Platform explorer surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
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

static GtkWidget *runtime_build_surface_test_explorer(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Test Explorer");
    GtkWidget *actions;
    GtkWidget *discover;
    GtkWidget *run_all;
    GtkWidget *rerun_failed;
    GtkWidget *stop;

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

        gtk_box_append(GTK_BOX(actions), discover);
        gtk_box_append(GTK_BOX(actions), run_all);
        gtk_box_append(GTK_BOX(actions), rerun_failed);
        gtk_box_append(GTK_BOX(actions), stop);
        gtk_box_append(GTK_BOX(panel), actions);
    }

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

    if (runtime->test_list != NULL) {
        gtk_list_box_set_selection_mode(
            GTK_LIST_BOX(runtime->test_list),
            GTK_SELECTION_NONE);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->test_list));
    }

    runtime_tests_refresh_list(runtime);
    return panel;
}
