/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_program_console_gtk4.c
 * PURPOSE: Exercise the real Studio program-console action with an isolated native producer.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/program_console.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #value);                            \
            failed = 1;                                                                            \
            goto cleanup;                                                                          \
        }                                                                                          \
    } while (0)
static void Drain(void)
{
    for (unsigned i = 0U; i < 30U; ++i)
    {
        while (g_main_context_iteration(NULL, FALSE))
        {
        }
        g_usleep(1000U);
    }
}
static bool Wait(GtkWidget *panel)
{
    for (unsigned i = 0U; i < 10000U; ++i)
    {
        while (g_main_context_iteration(NULL, FALSE))
        {
        }
        if (!UmiGtk4ProgramConsolePanelPending(panel))
            return true;
        g_usleep(1000U);
    }
    return false;
}
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    const char *mode = argv[1];
    const char *cases[] = {"input",         "retained",     "revoke",
                           "project-close", "parent-close", "denied"};
    bool known = false;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(mode, cases[i]) == 0)
            known = true;
    if (!known)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool moved = false;
    char *original = g_get_current_dir(),
         *previous_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory = g_dir_make_tmp("umicom-studio-console-XXXXXX", NULL);
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *open = NULL, *panel = NULL;
    GtkWindow *dialog = NULL;
    UmiBuildProfile *profile = g_new0(UmiBuildProfile, 1U);
    CHECK(directory != NULL && original != NULL && g_chdir(directory) == 0);
    moved = true;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
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
    UmiStudioServicesOptions service_options = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    CHECK(umi_studio_workspace_open(services, directory, strcmp(mode, "denied") != 0, 0) ==
          UMI_STATUS_OK);
    umi_build_profile_init(profile);
    CHECK(umi_build_profile_set(profile, "console-fixture", directory, "build") == UMI_STATUS_OK);
    CHECK(strlen(argv[2]) < sizeof profile->run_program);
    strcpy(profile->run_program, argv[2]);
    strcpy(profile->run_arguments, "echo");
    profile->timeout_ms = 5000U;
    CHECK(umi_studio_build_service_set_profile(umi_studio_services_build(services), profile) ==
          UMI_STATUS_OK);
    application =
        gtk_application_new("org.umicom.studio.program-console-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TERMINAL,
                                                          NULL) == UMI_STATUS_OK);
    GtkWindow *owner = umi_studio_gtk_workbench_window(workbench);
    gtk_window_present(owner);
    Drain();
    open = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(owner),
                                                  "studio.terminal.program-console");
    CHECK(open != NULL);
    g_object_ref(open);
    if (strcmp(mode, "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    guint before = g_list_model_get_n_items(gtk_window_get_toplevels());
    g_signal_emit_by_name(open, "clicked");
    if (strcmp(mode, "denied") == 0 || strcmp(mode, "retained") == 0)
    {
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
        goto cleanup;
    }
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
    {
        GtkWindow *candidate = g_list_model_get_item(windows, i);
        GtkWidget *found =
            umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate), "program.console.panel");
        if (found != NULL)
        {
            dialog = candidate;
            panel = g_object_ref(found);
            break;
        }
        g_object_unref(candidate);
    }
    CHECK(dialog != NULL && panel != NULL);
    Drain();
    GtkWidget *run = umi_gtk4_automation_find_tagged_widget(panel, "program.console.run");
    GtkWidget *input = umi_gtk4_automation_find_tagged_widget(panel, "program.console.input");
    GtkWidget *send = umi_gtk4_automation_find_tagged_widget(panel, "program.console.send");
    GtkWidget *end = umi_gtk4_automation_find_tagged_widget(panel, "program.console.end");
    CHECK(run != NULL && input != NULL && send != NULL && end != NULL);
    CHECK(!UmiGtk4ProgramConsolePanelPending(panel));
    g_signal_emit_by_name(run, "clicked");
    CHECK(UmiGtk4ProgramConsolePanelPending(panel));
    if (strcmp(mode, "revoke") == 0)
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    else if (strcmp(mode, "project-close") == 0)
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    else if (strcmp(mode, "parent-close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    else
    {
        gtk_editable_set_text(GTK_EDITABLE(input), "Studio input");
        g_signal_emit_by_name(send, "clicked");
        g_signal_emit_by_name(end, "clicked");
    }
    CHECK(Wait(panel));
    if (strcmp(mode, "input") == 0)
    {
        GtkWidget *output = umi_gtk4_automation_find_tagged_widget(panel, "program.console.output");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));
        GtkTextIter first, last;
        gtk_text_buffer_get_bounds(buffer, &first, &last);
        char *text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
        bool echoed = strstr(text, "Studio input\n") != NULL && strstr(text, "EOF") != NULL;
        g_free(text);
        CHECK(echoed);
    }
cleanup:
    if (dialog != NULL)
        gtk_window_destroy(dialog);
    umi_studio_gtk_workbench_destroy(workbench);
    if (panel != NULL)
        (void)Wait(panel);
    g_clear_object(&dialog);
    g_clear_object(&panel);
    g_clear_object(&open);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    g_free(profile);
    if (moved)
    {
        if (g_chdir(original) != 0)
            failed = 1;
    }
    if (previous_data != NULL)
        g_setenv("UMICOM_STUDIO_DATA_PATH", previous_data, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    if (directory != NULL)
        printf("Isolated program-console fixture retained: %s\n", directory);
    g_free(directory);
    g_free(original);
    g_free(previous_data);
    return failed;
}
