/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_job_history_gtk4.c
 * PURPOSE: Check explicit job-history selection, reopen and safe retained controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(x))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                          \
            failed = 1;                                                                                      \
            goto cleanup;                                                                                    \
        }                                                                                                    \
    } while (0)
/* Keep the host test independent of translated labels and GTK child order. */
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    if (root == NULL)
        return NULL;
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0)
        return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkWidget *found = Find(child, tag);
        if (found != NULL)
            return found;
    }
    return NULL;
}
/* Isolate settings and evidence before starting the native host. This fixture
 * never submits a build, launches a compiler or opens a user's project. */
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
#ifndef UMICOM_HAS_SQLITE
    (void)argv;
    return 77;
#endif
    int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    UmiBuildResult *result = NULL;
    GtkWidget *retained = NULL;
    UmiStudioBuildJobHistory *history = NULL;
    char *databasePath = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check())
        return 77;
    originalDirectory = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-job-history-XXXXXX", &error);
    CHECK(originalDirectory != NULL && directory != NULL && g_chdir(directory) == 0);
    changedDirectory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.job-history-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_OUTPUT, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *refresh = Find(root, "build.output.refresh");
    CHECK(GTK_IS_BUTTON(refresh) && GTK_IS_CHECK_BUTTON(Find(root, "build.output.follow")));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "build.output.text")));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "studio.build.output")));
    UmiStudioBuildService *build = umi_studio_services_build(umi_studio_bootstrap_services(bootstrap));
    CHECK(!UmiStudioBuildBusy(build));

    GtkWidget *select = Find(root, "studio.build.history.open");
    GtkWidget *entry = Find(root, "studio.build.history.path");
    GtkWidget *refreshJobs = Find(root, "studio.build.history.refresh");
    GtkWidget *prune = Find(root, "studio.build.history.prune");
    GtkWidget *detach = Find(root, "studio.build.history.detach");
    CHECK(GTK_IS_BUTTON(select) && GTK_IS_ENTRY(entry) && GTK_IS_BUTTON(refreshJobs));
    CHECK(GTK_IS_BUTTON(prune) && GTK_IS_BUTTON(detach));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "studio.build.history.entries")));
    history = g_new0(UmiStudioBuildJobHistory, 1);
    databasePath = g_build_filename(directory, "job-history.sqlite", NULL);
    gtk_editable_set_text(GTK_EDITABLE(entry), databasePath);
    CHECK(UmiStudioBuildReadJobHistory(build, history) == UMI_STATUS_OK && !history->current.enabled);
    if (strncmp(argv[1], "retained-", 9) == 0)
    {
        retained = g_object_ref(strcmp(argv[1], "retained-open") == 0      ? select
                                : strcmp(argv[1], "retained-prune") == 0   ? prune
                                : strcmp(argv[1], "retained-refresh") == 0 ? refreshJobs
                                                                           : detach);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        g_signal_emit_by_name(retained, "clicked");
    }
    else
    {
        g_signal_emit_by_name(select, "clicked");
        CHECK(UmiStudioBuildReadJobHistory(build, history) == UMI_STATUS_OK && history->current.enabled);
        CHECK(strcmp(history->path, databasePath) == 0 && !UmiStudioBuildBusy(build));
        if (strcmp(argv[1], "invalid") == 0)
        {
            gtk_editable_set_text(GTK_EDITABLE(entry), "relative.sqlite");
            g_signal_emit_by_name(select, "clicked");
            CHECK(UmiStudioBuildReadJobHistory(build, history) == UMI_STATUS_OK &&
                  strcmp(history->path, databasePath) == 0);
        }
        else if (strcmp(argv[1], "detach") == 0)
        {
            g_signal_emit_by_name(detach, "clicked");
            CHECK(UmiStudioBuildReadJobHistory(build, history) == UMI_STATUS_OK && !history->current.enabled);
        }
        else
        {
            UmiDataServer *server = NULL;
            UmiJobHistory *records = NULL;
            UmiJobHistoryEntry unfinished = {0}, complete = {0};
            CHECK(umi_data_server_create_sqlite(databasePath, &server) == UMI_STATUS_OK);
            CHECK(UmiJobHistoryCreate(server, "studio.build", &records) == UMI_STATUS_OK);
            CHECK(UmiJobHistoryBegin(records, "build.workflow", "Earlier build", 2, &unfinished) ==
                  UMI_STATUS_OK);
            CHECK(UmiJobHistoryUpdate(records, unfinished.id, unfinished.revision, UMI_JOB_HISTORY_RUNNING, 0,
                                      UMI_STATUS_OK, &unfinished) == UMI_STATUS_OK);
            CHECK(UmiJobHistoryBegin(records, "build.workflow", "Earlier test", 1, &complete) ==
                  UMI_STATUS_OK);
            CHECK(UmiJobHistoryUpdate(records, complete.id, complete.revision, UMI_JOB_HISTORY_RUNNING, 0,
                                      UMI_STATUS_OK, &complete) == UMI_STATUS_OK);
            CHECK(UmiJobHistoryUpdate(records, complete.id, complete.revision, UMI_JOB_HISTORY_SUCCEEDED, 1,
                                      UMI_STATUS_OK, &complete) == UMI_STATUS_OK);
            UmiJobHistoryDestroy(records);
            umi_data_server_destroy(server);
            if (strcmp(argv[1], "prune") == 0)
                g_signal_emit_by_name(prune, "clicked");
            else
            {
                CHECK(strcmp(argv[1], "reopen") == 0);
                g_signal_emit_by_name(detach, "clicked");
                g_signal_emit_by_name(select, "clicked");
            }
            g_signal_emit_by_name(refreshJobs, "clicked");
            GtkTextBuffer *buffer =
                gtk_text_view_get_buffer(GTK_TEXT_VIEW(Find(root, "studio.build.history.entries")));
            GtkTextIter begin, end;
            gtk_text_buffer_get_bounds(buffer, &begin, &end);
            char *contents = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
            bool hasUnknown = strstr(contents, "outcome unknown") != NULL;
            bool hasFinished = strstr(contents, "Earlier test") != NULL;
            g_free(contents);
            CHECK(hasUnknown && hasFinished == (strcmp(argv[1], "reopen") == 0));
            CHECK(!UmiStudioBuildBusy(build));
        }
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    }

cleanup:
    g_free(history);
    g_free(databasePath);
    if (retained != NULL)
        g_object_unref(retained);
    umi_build_result_destroy(result);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0)
        failed = 1;
    if (isolated)
    {
        if (originalData != NULL)
        {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE))
                failed = 1;
        }
        else
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL)
        fprintf(stderr, "%s\n", error->message);
    if (directory != NULL)
        printf("Retained isolated output fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(originalDirectory);
    g_free(originalData);
    return failed;
}
