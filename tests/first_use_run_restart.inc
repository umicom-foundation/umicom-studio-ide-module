/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/first_use_run_restart.inc
 *
 * PURPOSE:
 *   Exercise native Restart admission, deferred authority, history and unsaved document preservation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static int FirstUseRunRestartScenario(const char *name)
{
    return strncmp(name, "run-restart-", 12U) == 0;
}
static int ExerciseRunRestart(UmiStudioGtkWorkbench *workbench, UmiStudioUi *ui,
    UmiStudioServices *services, UmiDocumentCoordinator *documents,
    const char *directory, const char *mode)
{
    int failed = 0;
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiBuildProfile *profile = g_new(UmiBuildProfile, 1);
    GtkTextBuffer *buffer = NULL;
    char *source = g_build_filename(directory, "restart-source.c", NULL);
    char *marker = g_build_filename(directory, "restart-ready.txt", NULL);
    char *log = g_build_filename(directory, "restart-output.log", NULL);
    char *fresh = NULL, *disk = NULL, *draft = NULL;
    const char *saved = "saved text\n", *unsaved = "unsaved restart draft\n";
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(g_file_set_contents(source, saved, -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", directory) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(GTK_IS_TEXT_VIEW(editor));
    buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)));
    FirstUseReplaceText(buffer, unsaved);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.run-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *restart = FindMenuCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), "run.restart");
    CHECK(GTK_IS_BUTTON(restart) && !gtk_widget_get_sensitive(restart));
    if (!strcmp(mode, "run-restart-idle")) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.restart", NULL) == UMI_STATUS_INVALID_STATE);
    } else {
        *profile = *umi_studio_build_service_profile(build);
        CHECK(strlen(UMICOM_STUDIO_STOP_PEER) < sizeof profile->run_program);
        CHECK(strlen(marker) < sizeof profile->run_argument);
        strcpy(profile->run_program, UMICOM_STUDIO_STOP_PEER);
        strcpy(profile->run_argument, marker);
        profile->run_arguments[0] = '\0';
        strcpy(profile->run_working_directory, ".");
        profile->timeout_ms = 25000U;
        CHECK(umi_studio_build_service_set_profile(build, profile) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        bool logged = !strcmp(mode, "run-restart-log") || !strcmp(mode, "run-restart-log-collision");
        if (logged) CHECK(UmiStudioBuildArmLog(build, log) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT, NULL) == UMI_STATUS_OK);
        gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (!g_file_test(marker, G_FILE_TEST_IS_REGULAR) && g_get_monotonic_time() < deadline)
            CHECK(FirstUseRunMenuPump(workbench));
        CHECK(g_file_test(marker, G_FILE_TEST_IS_REGULAR));
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(gtk_widget_get_sensitive(restart));
        UmiBuildProjectSessionSnapshot before, after;
        CHECK(UmiStudioBuildProgress(build, &before) == UMI_STATUS_OK && before.active);
        size_t history_before = umi_build_history_count(umi_studio_build_service_history(build));
        fresh = g_strdup_printf("%s.restart-%" G_GUINT64_FORMAT, log, (guint64)(before.operation_id + 1U));
        if (!strcmp(mode, "run-restart-invalid")) {
            CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.restart", "build") == UMI_STATUS_INVALID_ARGUMENT);
            CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && !after.cancellation_requested);
        } else if (!strcmp(mode, "run-restart-untrusted")) {
            CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            CHECK(!gtk_widget_get_sensitive(restart));
            CHECK(umi_studio_gtk_workbench_execute_command(workbench, "run.restart", NULL) == UMI_STATUS_PERMISSION_DENIED);
            CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK && !after.cancellation_requested);
        } else {
            CHECK(g_remove(marker) == 0);
            bool collision = !strcmp(mode, "run-restart-log-collision");
            if (collision) CHECK(g_file_set_contents(fresh, "retain existing evidence\n", -1, NULL));
            bool revoked = !strcmp(mode, "run-restart-revoked"), stopped = !strcmp(mode, "run-restart-stop");
            bool direct = revoked || stopped || !strcmp(mode, "run-restart-duplicate") ||
                !strcmp(mode, "run-restart-profile-blocked");
            if (direct) {
                /* The canonical registry call leaves a deterministic gap before
                 * GUI refresh, allowing the test to revoke launch authority. */
                char message[512];
                UmiCommandRegistry *registry = umi_ui_workbench_commands(umi_studio_ui_workbench(ui));
                CHECK(umi_command_registry_execute(registry, UMI_STUDIO_COMMAND_BUILD_RUN_RESTART,
                    NULL, message, sizeof message) == UMI_STATUS_OK);
                if (revoked) CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
                if (stopped) umi_studio_build_service_cancel(build);
                if (!strcmp(mode, "run-restart-duplicate"))
                    CHECK(umi_command_registry_execute(registry, UMI_STUDIO_COMMAND_BUILD_RUN_RESTART,
                        NULL, message, sizeof message) == UMI_STATUS_BUSY);
                if (!strcmp(mode, "run-restart-profile-blocked")) {
                    strcpy(profile->run_argument, "a different launch");
                    CHECK(umi_studio_build_service_set_profile(build, profile) == UMI_STATUS_BUSY);
                }
            } else CHECK(FirstUseMenuBuildCommand(workbench, "run.restart") == UMI_STATUS_OK);
            bool refused = revoked || stopped || collision;
            deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
            do {
                CHECK(FirstUseRunMenuPump(workbench));
                CHECK(UmiStudioBuildProgress(build, &after) == UMI_STATUS_OK);
                if (refused ? !UmiStudioBuildBusy(build) :
                    after.operation_id == before.operation_id + 1U && g_file_test(marker, G_FILE_TEST_IS_REGULAR)) break;
            } while (g_get_monotonic_time() < deadline);
            UmiBuildRunRestartSnapshot intent;
            CHECK(UmiStudioBuildRestartRead(build, &intent) == UMI_STATUS_OK && !intent.pending);
            CHECK(umi_build_history_count(umi_studio_build_service_history(build)) >= history_before + 1U);
            if (refused) {
                CHECK(!UmiStudioBuildBusy(build) && after.operation_id == before.operation_id);
                CHECK(!g_file_test(marker, G_FILE_TEST_EXISTS));
                CHECK(intent.submitted_operation_id == 0U && intent.status != UMI_STATUS_OK);
                if (collision) {
                    CHECK(g_file_get_contents(fresh, &disk, NULL, NULL));
                    CHECK(!strcmp(disk, "retain existing evidence\n"));
                    g_clear_pointer(&disk, g_free);
                }
            } else {
                CHECK(after.active && after.operation_id == before.operation_id + 1U);
                CHECK(g_file_test(marker, G_FILE_TEST_IS_REGULAR));
                CHECK(intent.submitted_operation_id == after.operation_id);
                if (logged) CHECK(g_file_test(log, G_FILE_TEST_IS_REGULAR) && g_file_test(fresh, G_FILE_TEST_IS_REGULAR));
            }
        }
    }
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.dirty);
    GtkTextIter first, last;
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    draft = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(draft != NULL && !strcmp(draft, unsaved));
    CHECK(g_file_get_contents(source, &disk, NULL, NULL) && !strcmp(disk, saved));
cleanup:
    if (UmiStudioBuildBusy(build)) {
        umi_studio_build_service_cancel(build);
        gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (UmiStudioBuildBusy(build) && g_get_monotonic_time() < deadline)
            if (!FirstUseRunMenuPump(workbench)) break;
    }
    g_clear_object(&buffer);
    g_free(profile); g_free(source); g_free(marker); g_free(log); g_free(fresh); g_free(disk); g_free(draft);
    return failed;
}
