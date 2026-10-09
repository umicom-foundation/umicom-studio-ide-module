/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_learning_locations_gtk4.c
 * PURPOSE: Check explicit lesson and project locations and the lifetime of learning controls.
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
 * restoring personal sessions or granting a temporary folder workspace trust. */
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
    char *directory = g_dir_make_tmp("umicom-learning-locations-XXXXXX", &error);
    char *project = NULL;
    char library[UMI_PATH_CAPACITY] = {0};
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    GtkWidget *retained[4] = {NULL, NULL, NULL, NULL};
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
    if (strcmp(argv[1], "first-lesson") == 0)
    {
        /* An explicit in-memory choice exercises the product path without
         * presenting a platform file chooser or writing personal settings. */
        CHECK(UmiLearningLibrarySelect(argv[2], library, sizeof(library)) == UMI_STATUS_OK);
        CHECK(umi_session_store_set(umi_studio_services_session(services), "learning.library",
                                    library) == UMI_STATUS_OK);
    }

    application =
        gtk_application_new("org.umicom.studio.learning-locations-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_LEARNING,
                                                          NULL) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *label = umi_gtk4_automation_find_tagged_widget(root, "studio.learning.library");
    CHECK(GTK_IS_LABEL(label));
    if (strcmp(argv[1], "first-lesson") != 0)
    {
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), "Choose the folder") != NULL);
    }
    else
    {
        CHECK(umi_path_equal(gtk_label_get_text(GTK_LABEL(label)), library));
    }
    const char *actions[] = {"studio.learning.choose-library", "studio.learning.create-project",
                             "studio.learning.open-project", "studio.learning.reference"};
    for (size_t i = 0U; i < 4U; ++i)
    {
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(root, actions[i]);
        CHECK(GTK_IS_BUTTON(button));
        retained[i] = g_object_ref(button);
    }
    if (strcmp(argv[1], "first-lesson") == 0)
    {
        g_signal_emit_by_name(retained[3], "clicked");
        GtkWidget *preview =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.preview");
        GtkWidget *location =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.location");
        CHECK(GTK_IS_TEXT_VIEW(preview) && GTK_IS_ENTRY(location));
        CHECK(gtk_text_buffer_get_char_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview))) > 0);
        UmiStudioLearningLessonSnapshot lesson;
        char expected[UMI_PATH_CAPACITY];
        CHECK(umi_studio_learning_centre_find_lesson("foundations.c-hello", &lesson) ==
              UMI_STATUS_OK);
        CHECK(UmiLearningLibraryResource(library, lesson.resource_path, expected,
                                         sizeof(expected)) == UMI_STATUS_OK);
        char *actual =
            g_filename_from_uri(gtk_editable_get_text(GTK_EDITABLE(location)), NULL, NULL);
        int same = actual != NULL && umi_path_equal(actual, expected);
        g_free(actual);
        CHECK(same);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.open);
    }
    else if (strcmp(argv[1], "relative-without-project") == 0)
    {
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(
                  workbench, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL) == UMI_STATUS_OK);
        GtkWidget *location =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.location");
        GtkWidget *open = umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.open");
        GtkWidget *notice =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.summary");
        CHECK(GTK_IS_ENTRY(location) && GTK_IS_BUTTON(open) && GTK_IS_LABEL(notice));
        gtk_editable_set_text(GTK_EDITABLE(location), "welcome.html");
        g_signal_emit_by_name(open, "clicked");
        CHECK(strcmp(gtk_label_get_text(GTK_LABEL(notice)), "Choose a document location") == 0);
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(location)), "welcome.html") == 0);
    }
    else if (strcmp(argv[1], "controls") == 0)
    {
        CHECK(!gtk_widget_get_visible(root));
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.open);
    }
    else if (strcmp(argv[1], "project-separation") == 0)
    {
        project = g_build_filename(directory, "practice", NULL);
        CHECK(project && g_mkdir(project, 0700) == 0);
        CHECK(umi_studio_workspace_open(services, project, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.open);
        /* Selecting a project does not invent or replace the lesson library. */
        label = umi_gtk4_automation_find_tagged_widget(root, "studio.learning.library");
        CHECK(GTK_IS_LABEL(label));
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), "Choose the folder") != NULL);
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    }
    else if (strcmp(argv[1], "retained-controls") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        guint before = g_list_model_get_n_items(gtk_window_get_toplevels());
        for (size_t i = 0U; i < 4U; ++i)
            g_signal_emit_by_name(retained[i], "clicked");
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
    }
    else
    {
        failed = 2;
    }
cleanup:
    for (size_t i = 0U; i < 4U; ++i)
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
        printf("Retained isolated learning fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(project);
    g_free(directory);
    g_free(original_directory);
    g_free(original_data);
    return failed;
}
