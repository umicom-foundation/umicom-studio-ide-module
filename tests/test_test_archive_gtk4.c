/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive_gtk4.c
 * PURPOSE: Check native test-history actions and safe retained controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "test_archive_job.h"
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

/* Reserve isolated settings and database storage. All fixture tests are disabled:
 * this host exercises archive controls, not CTest or a user's projects. */
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
    GtkWidget *retained = NULL;
    char *databasePath = NULL, *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check())
        return 77;
    originalDirectory = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-test-archive-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.test-archive", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TEST_EXPLORER,
                                                          NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    const char *actions[] = {"open", "refresh", "save", "stop", "read", "remove", "detach"};
    for (size_t i = 0; i < sizeof(actions) / sizeof(actions[0]); ++i)
    {
        char tag[96];
        (void)snprintf(tag, sizeof(tag), "studio.tests.archive.%s", actions[i]);
        CHECK(GTK_IS_BUTTON(Find(root, tag)));
    }
    const char *compareActions[] = {"compare", "compare.stop", "compare.page"};
    for (size_t i = 0; i < sizeof(compareActions) / sizeof(compareActions[0]); ++i)
    {
        char tag[96];
        (void)snprintf(tag, sizeof(tag), "studio.tests.archive.%s", compareActions[i]);
        CHECK(GTK_IS_BUTTON(Find(root, tag)));
    }
    CHECK(GTK_IS_ENTRY(Find(root, "studio.tests.archive.baseline")));
    CHECK(GTK_IS_ENTRY(Find(root, "studio.tests.archive.page")));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "studio.tests.archive.entries")));
    GtkWidget *path = Find(root, "studio.tests.archive.path");
    GtkWidget *retainOutput = Find(root, "studio.tests.archive.output");
    CHECK(GTK_IS_ENTRY(path) && GTK_IS_CHECK_BUTTON(retainOutput) &&
          !gtk_check_button_get_active(GTK_CHECK_BUTTON(retainOutput)));
    if (strncmp(argv[1], "retained-", 9) == 0)
    {
        char tag[96];
        (void)snprintf(tag, sizeof(tag), "studio.tests.archive.%s", argv[1] + 9);
        GtkWidget *control = Find(root, tag);
        CHECK(GTK_IS_BUTTON(control));
        retained = g_object_ref(control);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        g_signal_emit_by_name(retained, "clicked");
    }
    else
    {
        UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
        UmiStudioTestService *tests = umi_studio_services_tests(services);
        databasePath = g_build_filename(directory, "test-history.sqlite", NULL);
        gtk_editable_set_text(GTK_EDITABLE(path), databasePath);
        g_signal_emit_by_name(Find(root, "studio.tests.archive.open"), "clicked");
        UmiStudioTestArchiveState state = {0};
        CHECK(UmiStudioTestArchiveStateRead(tests, &state) == UMI_STATUS_OK && state.attached);
        if (strcmp(argv[1], "invalid") == 0)
        {
            gtk_editable_set_text(GTK_EDITABLE(path), "relative.sqlite");
            g_signal_emit_by_name(Find(root, "studio.tests.archive.open"), "clicked");
            CHECK(UmiStudioTestArchiveStateRead(tests, &state) == UMI_STATUS_OK &&
                  strcmp(state.path, databasePath) == 0);
        }
        else if (strcmp(argv[1], "compare") == 0 || strcmp(argv[1], "compare-page-invalid") == 0)
        {
            CHECK(ArchiveDisabledRun(tests, umi_studio_services_task_queue(services)) == UMI_STATUS_OK);
            g_signal_emit_by_name(Find(root, "studio.tests.archive.save"), "clicked");
            CHECK(ArchiveAwaitSave(tests, &state) == UMI_STATUS_OK);
            gtk_editable_set_text(GTK_EDITABLE(Find(root, "studio.tests.archive.id")), "1");
            gtk_editable_set_text(GTK_EDITABLE(Find(root, "studio.tests.archive.baseline")), "1");
            g_signal_emit_by_name(Find(root, "studio.tests.archive.compare"), "clicked");
            CHECK(ArchiveAwaitComparison(tests, &state) == UMI_STATUS_OK && state.comparison.ready);
            g_signal_emit_by_name(Find(root, "studio.tests.archive.compare.page"), "clicked");
            GtkTextBuffer *buffer =
                gtk_text_view_get_buffer(GTK_TEXT_VIEW(Find(root, "studio.tests.archive.entries")));
            GtkTextIter begin, end;
            gtk_text_buffer_get_bounds(buffer, &begin, &end);
            char *contents = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
            bool shown = strstr(contents, "Unchanged outcome: 1") != NULL &&
                         strstr(contents, "archive.disabled") != NULL && strstr(contents, "0 passed") != NULL;
            g_free(contents);
            CHECK(shown);
            if (strcmp(argv[1], "compare-page-invalid") == 0)
            {
                gtk_editable_set_text(GTK_EDITABLE(Find(root, "studio.tests.archive.page")), "9999");
                g_signal_emit_by_name(Find(root, "studio.tests.archive.compare.page"), "clicked");
                gtk_text_buffer_get_bounds(buffer, &begin, &end);
                contents = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
                shown = strstr(contents, "Unchanged outcome: 1") != NULL;
                g_free(contents);
                CHECK(shown);
            }
        }
        else
        {
            CHECK(strcmp(argv[1], "save-read") == 0 || strcmp(argv[1], "remove") == 0);
            CHECK(ArchiveDisabledRun(tests, umi_studio_services_task_queue(services)) == UMI_STATUS_OK);
            g_signal_emit_by_name(Find(root, "studio.tests.archive.save"), "clicked");
            CHECK(ArchiveAwaitSave(tests, &state) == UMI_STATUS_OK && state.write.saved.id == 1);
            g_signal_emit_by_name(Find(root, "studio.tests.archive.refresh"), "clicked");
            gtk_editable_set_text(GTK_EDITABLE(Find(root, "studio.tests.archive.id")), "1");
            gtk_editable_set_text(GTK_EDITABLE(Find(root, "studio.tests.archive.attempt")), "1");
            g_signal_emit_by_name(Find(root, "studio.tests.archive.read"), "clicked");
            GtkTextBuffer *buffer =
                gtk_text_view_get_buffer(GTK_TEXT_VIEW(Find(root, "studio.tests.archive.entries")));
            GtkTextIter begin, end;
            gtk_text_buffer_get_bounds(buffer, &begin, &end);
            char *contents = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
            bool found =
                strstr(contents, "archive.disabled") != NULL && strstr(contents, "not retained") != NULL;
            g_free(contents);
            CHECK(found);
            if (strcmp(argv[1], "remove") == 0)
            {
                g_signal_emit_by_name(Find(root, "studio.tests.archive.remove"), "clicked");
                UmiTestArchiveEntry entry = {0};
                CHECK(UmiStudioTestArchiveRead(tests, 1, &entry) == UMI_STATUS_NOT_FOUND);
            }
        }
    }
cleanup:
    g_free(databasePath);
    if (retained != NULL)
        g_object_unref(retained);
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
        printf("Retained isolated test archive: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(originalDirectory);
    g_free(originalData);
    return failed;
}
