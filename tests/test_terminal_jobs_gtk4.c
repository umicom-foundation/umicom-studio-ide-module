/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_terminal_jobs_gtk4.c
 * PURPOSE: Check Studio terminal command dispatch, output ownership and retained controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/session_store.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/learning_centre.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/teacher/learning_library.h"
#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition);                        \
            failed = 1;                                                                            \
            goto cleanup;                                                                          \
        }                                                                                          \
    } while (0)

/* Construct the real product without presenting a window, starting tools,
 * restoring personal sessions. Execution cases grant trust only to their isolated fixture. */
#include "umicom/studio/commands.h"
#include "umicom/terminal_ui/execution.h"
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0, changed_directory = 0, isolated = 0;
    GError *error = NULL;
    char *original_directory = g_get_current_dir();
    char *original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory = g_dir_make_tmp("umicom-terminal-jobs-XXXXXX", &error);
    char *project = NULL;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    GtkWidget *retained[7] = {NULL};
    UmiStudioServicesOptions service_options = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(original_directory && directory && g_chdir(directory) == 0);
    changed_directory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
    CHECK(!workspace.graph.open);
    application =
        gtk_application_new("org.umicom.studio.terminal-jobs-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TERMINAL,
                                                          NULL) == UMI_STATUS_OK);

    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *run = umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.run");
    GtkWidget *stop = umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.stop");
    GtkWidget *entry = umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.command");
    GtkWidget *output = umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.output");
    CHECK(GTK_IS_BUTTON(run) && GTK_IS_BUTTON(stop) && GTK_IS_ENTRY(entry) &&
          GTK_IS_TEXT_VIEW(output));
    retained[0] = g_object_ref(run);
    retained[1] = g_object_ref(stop);
    retained[2] = g_object_ref(entry);
    GtkWidget *folder_entry =
        umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.folder");
    GtkWidget *folder_apply =
        umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.apply-folder");
    GtkWidget *folder_browse =
        umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.browse");
    GtkWidget *folder_project =
        umi_gtk4_automation_find_tagged_widget(root, "studio.terminal.project-folder");
    CHECK(GTK_IS_ENTRY(folder_entry) && GTK_IS_BUTTON(folder_apply) &&
          GTK_IS_BUTTON(folder_browse) && GTK_IS_BUTTON(folder_project));
    retained[3] = g_object_ref(folder_entry);
    retained[4] = g_object_ref(folder_apply);
    retained[5] = g_object_ref(folder_browse);
    retained[6] = g_object_ref(folder_project);

    CHECK(!gtk_widget_get_sensitive(stop));
    UmiTerminalController *controller = umi_studio_services_terminal_controller(services);
    UmiTerminalProfile profile;
    umi_terminal_profile_init(&profile);
    strcpy(profile.profile_id, "fixture.direct");
    strcpy(profile.title, "Direct command");
    strcpy(profile.program, "fixture");
    profile.kind = UMI_TERMINAL_PROFILE_CUSTOM;
    CHECK(umi_terminal_profile_registry_register(umi_terminal_controller_profiles(controller),
                                                 &profile) == UMI_STATUS_OK);
    CHECK(umi_terminal_controller_open(controller, profile.profile_id, "fixture.job",
                                       "Fixture command", directory) == UMI_STATUS_OK);
    UmiTerminalCommand command;
    umi_terminal_command_init(&command);
    const char *values[] = {argv[2], strcmp(argv[1], "complete") == 0 ? "success" : "hold"};
    for (size_t index = 0U; index < 2U; ++index)
    {
        CHECK(strlen(values[index]) < sizeof command.argument_storage[index]);
        memcpy(command.argument_storage[index], values[index], strlen(values[index]) + 1U);
        command.arguments[index] = command.argument_storage[index];
    }
    command.argument_count = 2U;
    char text[UMI_TERMINAL_COMMAND_CAPACITY];
    CHECK(umi_terminal_command_format(&command, text, sizeof text) == UMI_STATUS_OK);
    gtk_editable_set_text(GTK_EDITABLE(entry), text);
    if (strcmp(argv[1], "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        guint before = g_list_model_get_n_items(gtk_window_get_toplevels());
        g_signal_emit_by_name(run, "clicked");
        g_signal_emit_by_name(stop, "clicked");
        g_signal_emit_by_name(entry, "activate");
        g_signal_emit_by_name(folder_apply, "clicked");
        g_signal_emit_by_name(folder_browse, "clicked");
        g_signal_emit_by_name(folder_project, "clicked");
        gtk_editable_set_text(GTK_EDITABLE(folder_entry), "retained draft");

        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
    }
    else if (strcmp(argv[1], "denied") == 0)
    {
        /* No trusted workspace exists, so GUI dispatch must not launch a child. */
        g_signal_emit_by_name(run, "clicked");
        CHECK(!UmiTerminalControllerJobPending(controller));
        CHECK(!UmiTerminalControllerBackgroundArmed(controller));
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(entry)), text) == 0);
        CHECK(umi_process_supervisor_stats(umi_terminal_controller_process_supervisor(controller))
                  .jobs == 0U);
    }
    else if (strcmp(argv[1], "directory") == 0)
    {
        CHECK(umi_studio_workspace_open(services, directory, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        g_signal_emit_by_name(folder_project, "clicked");
        CHECK(umi_path_equal(gtk_editable_get_text(GTK_EDITABLE(folder_entry)), directory));
        g_signal_emit_by_name(folder_apply, "clicked");
        UmiTerminalSessionSnapshot selected;
        CHECK(umi_terminal_session_snapshot(umi_terminal_controller_active_session(controller),
                                            &selected) == UMI_STATUS_OK);
        CHECK(umi_path_equal(selected.working_directory, directory));
        gtk_editable_set_text(GTK_EDITABLE(folder_entry), "relative");
        g_signal_emit_by_name(folder_apply, "clicked");
        CHECK(umi_terminal_session_snapshot(umi_terminal_controller_active_session(controller),
                                            &selected) == UMI_STATUS_OK);
        CHECK(umi_path_equal(selected.working_directory, directory));
        CHECK(!UmiTerminalControllerJobPending(controller));
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    }
    else if (strcmp(argv[1], "selection") == 0)
    {
        UmiTerminalSession *active = umi_terminal_controller_active_session(controller);
        CHECK(umi_terminal_transcript_append(umi_terminal_session_transcript(active), 0U,
                                             UMI_TERMINAL_STREAM_OUTPUT,
                                             "Selected session output") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));
        GtkTextIter begin, end;
        gtk_text_buffer_get_iter_at_offset(buffer, &begin, 1);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, 8);
        gtk_text_buffer_select_range(buffer, &begin, &end);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(gtk_text_buffer_get_selection_bounds(buffer, &begin, &end));
        CHECK(gtk_text_iter_get_offset(&begin) == 1 && gtk_text_iter_get_offset(&end) == 8);
        gtk_text_buffer_get_bounds(buffer, &begin, &end);
        char *rendered = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
        int found = strstr(rendered, "Selected session output") != NULL;
        g_free(rendered);
        CHECK(found);
    }
    else if (strcmp(argv[1], "complete") == 0 || strcmp(argv[1], "stop") == 0 ||
             strcmp(argv[1], "revoke") == 0)
    {
        CHECK(umi_studio_workspace_open(services, directory, 1, 0) == UMI_STATUS_OK);
        gint64 started = g_get_monotonic_time();
        g_signal_emit_by_name(run, "clicked");
        CHECK(g_get_monotonic_time() - started < 4000000);
        CHECK(!UmiTerminalControllerBackgroundArmed(controller));
        if (strcmp(argv[1], "complete") != 0)
        {
            CHECK(UmiTerminalControllerJobPending(controller));
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_BUSY);
            CHECK(umi_studio_workspace_open(services, directory, 1, 0) == UMI_STATUS_BUSY);
            if (strcmp(argv[1], "revoke") == 0)
                CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
            else
                g_signal_emit_by_name(stop, "clicked");
        }
        while (UmiTerminalControllerJobPending(controller))
        {
            CHECK(g_get_monotonic_time() - started < 10000000);
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
            g_usleep(10000U);
        }
        UmiTerminalControllerExecution result;
        CHECK(UmiTerminalControllerPollJob(controller, &result) == UMI_STATUS_OK);
        CHECK(strcmp(result.session_id, "fixture.job") == 0);
        if (strcmp(argv[1], "complete") == 0)
        {
            CHECK(result.execution.process.exit_code == 0);
            CHECK(strstr(result.execution.process.output, "last:") != NULL);
        }
        else
            CHECK(result.execution.process.state == UMI_PROCESS_JOB_CANCELLED);
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    }
    else
        CHECK(0);
cleanup:
    for (size_t i = 0U; i < 7U; ++i)
        g_clear_object(&retained[i]);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed_directory && g_chdir(original_directory) != 0)
        failed = 1;
    if (isolated)
    {
        if (original_data != NULL)
        {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data, TRUE))
                failed = 1;
        }
        else
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL)
        fprintf(stderr, "%s\n", error->message);
    if (directory != NULL)
        printf("Retained isolated terminal fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(project);
    g_free(directory);
    g_free(original_directory);
    g_free(original_data);
    return failed;
}
