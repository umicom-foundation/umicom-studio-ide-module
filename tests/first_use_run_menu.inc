/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/first_use_run_menu.inc
 *
 * PURPOSE:
 *   Verify native Run Stop cancels the owned launch and Run With opens existing settings without applying a draft.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static int FirstUseRunMenuScenario(const char *name)
{
    return !strcmp(name, "run-menu-stop") || !strcmp(name, "run-menu-stop-untrusted") ||
        !strcmp(name, "run-menu-stop-invalid") || !strcmp(name, "run-menu-stop-idle") ||
        !strcmp(name, "run-menu-settings") || !strcmp(name, "run-menu-settings-closed") ||
        !strcmp(name, "run-menu-settings-invalid");
}

/* One normal refresh consumes process results; bounded context dispatch keeps
 * persistent GTK sources from starving the deadline. No result is fabricated. */
static int FirstUseRunMenuPump(UmiStudioGtkWorkbench *workbench)
{
    if (umi_studio_gtk_workbench_workspace_synchronise(workbench) != UMI_STATUS_OK) return 0;
    for (unsigned i = 0U; i < 32U && g_main_context_pending(NULL); ++i)
        (void)g_main_context_iteration(NULL, FALSE);
    g_usleep(10000U);
    return 1;
}
static int ExerciseRunMenu(UmiStudioGtkWorkbench *workbench, UmiStudioServices *services,
    UmiDocumentCoordinator *documents, const char *directory, const char *mode)
{
    int failed = 0;
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiBuildProfile *profile = NULL;
    UmiBuildResult *result = NULL;
    GtkWindow *settings = NULL;
    GtkTextBuffer *buffer = NULL;
    char *source = g_build_filename(directory, "run-menu.c", NULL);
    char *marker = g_build_filename(directory, "run-peer-ready.txt", NULL);
    char *disk = NULL, *draft = NULL;
    const char *savedText = "saved source\n", *draftText = "unsaved source stays here\n";
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(g_file_set_contents(source, savedText, -1, NULL));
    const int closed = !strcmp(mode, "run-menu-settings-closed");
    if (!closed)
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(GTK_IS_TEXT_VIEW(editor));
    buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)));
    FirstUseReplaceText(buffer, draftText);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.run-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *popover = GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu)));
    GtkWidget *stop = FindMenuCommand(popover, "run.stop");
    GtkWidget *configure = FindMenuCommand(popover, "run.select-configuration");
    CHECK(GTK_IS_BUTTON(stop) && GTK_IS_BUTTON(configure));
    CHECK(!gtk_widget_get_sensitive(stop));
    UmiBuildProjectSessionSnapshot before, after;
    CHECK(UmiStudioBuildProgress(build, &before) == UMI_STATUS_OK && !before.active);
    profile = g_new(UmiBuildProfile, 1);
    *profile = *umi_studio_build_service_profile(build);
    if (!strncmp(mode, "run-menu-settings", 17U)) {
        UmiStudioWorkspaceSnapshot workspaceBefore, workspaceAfter;
        CHECK(umi_studio_workspace_snapshot(services, &workspaceBefore) == UMI_STATUS_OK);
        CHECK(gtk_widget_get_sensitive(configure) == !closed);
        if (closed) {
            CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.select-configuration", NULL) == UMI_STATUS_INVALID_STATE);
        } else if (!strcmp(mode, "run-menu-settings-invalid")) {
            CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.select-configuration", "apply") == UMI_STATUS_INVALID_ARGUMENT);
        } else {
            CHECK(FirstUseMenuBuildCommand(workbench, "run.select-configuration") == UMI_STATUS_OK);
            settings = FindSettingsWindow(GTK_WINDOW(window));
            CHECK(settings != NULL);
            gtk_window_close(settings);
            TabJourneyPump();
        }
        CHECK(umi_build_profile_equal(profile, umi_studio_build_service_profile(build)));
        CHECK(umi_studio_workspace_snapshot(services, &workspaceAfter) == UMI_STATUS_OK);
        CHECK(workspaceBefore.graph.revision == workspaceAfter.graph.revision &&
            workspaceBefore.graph.trusted == workspaceAfter.graph.trusted);
        CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && !after.active &&
            after.operation_id == before.operation_id);
    } else if (!strcmp(mode, "run-menu-stop-idle")) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.stop", NULL) == UMI_STATUS_INVALID_STATE);
        CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && !after.active &&
            after.operation_id == before.operation_id);
    } else {
        CHECK(strlen(UMICOM_STUDIO_STOP_PEER) < sizeof profile->run_program);
        CHECK(strlen(marker) < sizeof profile->run_argument);
        strcpy(profile->run_program, UMICOM_STUDIO_STOP_PEER);
        strcpy(profile->run_argument, marker);
        profile->run_arguments[0] = '\0';
        strcpy(profile->run_working_directory, ".");
        profile->timeout_ms = 25000U;
        CHECK(umi_studio_build_service_set_profile(build, profile) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL) == UMI_STATUS_OK);
        gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (!g_file_test(marker, G_FILE_TEST_IS_REGULAR) && g_get_monotonic_time() < deadline)
            CHECK(FirstUseRunMenuPump(workbench));
        CHECK(g_file_test(marker, G_FILE_TEST_IS_REGULAR) && UmiStudioBuildBusy(build));
        if (!strcmp(mode, "run-menu-stop-untrusted"))
            CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(gtk_widget_get_sensitive(stop) && !gtk_widget_get_sensitive(configure));
        if (!strcmp(mode, "run-menu-stop-invalid")) {
            CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.stop", "restart") == UMI_STATUS_INVALID_ARGUMENT);
            CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && after.active && !after.cancellation_requested);
        }
        /* Dispatch through the actual menu, then inspect the session outcome. */
        CHECK(FirstUseMenuBuildCommand(workbench, "run.stop") == UMI_STATUS_OK);
        deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (UmiStudioBuildBusy(build) && g_get_monotonic_time() < deadline)
            CHECK(FirstUseRunMenuPump(workbench));
        CHECK(!UmiStudioBuildBusy(build));
        CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && !after.active &&
            after.operation_id > before.operation_id && after.cancellation_requested &&
            after.status == UMI_STATUS_CANCELLED && after.completed_phase_count == 1U);
        CHECK(umi_build_result_create(&result) == UMI_STATUS_OK);
        CHECK(umi_build_history_latest(umi_studio_build_service_history(build), result) == UMI_STATUS_OK);
        CHECK(result->phase == UMI_BUILD_PHASE_RUN && result->status == UMI_STATUS_CANCELLED);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(!gtk_widget_get_sensitive(stop));
    }
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    GtkTextIter first, last;
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    draft = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(draft != NULL && !strcmp(draft, draftText));
    CHECK(g_file_get_contents(source, &disk, NULL, NULL) && !strcmp(disk, savedText));
cleanup:
    if (UmiStudioBuildBusy(build)) {
        umi_studio_build_service_cancel(build);
        gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (UmiStudioBuildBusy(build) && g_get_monotonic_time() < deadline)
            if (!FirstUseRunMenuPump(workbench)) break;
    }
    if (settings != NULL) { gtk_window_close(settings); g_object_unref(settings); }
    umi_build_result_destroy(result);
    g_clear_object(&buffer);
    g_free(profile); g_free(source); g_free(marker); g_free(disk); g_free(draft);
    return failed;
}
