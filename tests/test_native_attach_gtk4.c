/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_native_attach_gtk4.c
 * PURPOSE: Exercise explicit attachment controls and retained-widget authorization.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            fprintf(stderr, "%d: %s\n", __LINE__, #v);                                             \
            failed = 1;                                                                            \
            goto done;                                                                             \
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
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    const char *mode = argv[1];
    bool denied = strcmp(mode, "denied") == 0, invalid = strcmp(mode, "invalid") == 0;
    bool retained = strcmp(mode, "retained") == 0, hidden = strcmp(mode, "hidden") == 0;
    bool revoke = strcmp(mode, "revoke") == 0;
    if (!denied && !invalid && !retained && !hidden && !revoke && strcmp(mode, "normal") != 0)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool moved = false;
    char *original = g_get_current_dir(), *previous = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory = g_dir_make_tmp("umicom-studio-attach-XXXXXX", NULL), *log = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *attach = NULL;
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
    CHECK(umi_studio_workspace_open(services, directory, !denied, 0) == UMI_STATUS_OK);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiBuildProfile profile = *umi_studio_build_service_profile(build);
    CHECK(umi_build_profile_set(&profile, "attach.test", directory, "build") == UMI_STATUS_OK);
    CHECK(umi_studio_build_service_set_profile(build, &profile) == UMI_STATUS_OK);
    UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
    CHECK(UmiStudioDebuggerConfigureNative(debugger, "gdb", argv[2]) == UMI_STATUS_OK);
    CHECK(g_file_set_contents("attach-fixture-mode.txt", "normal", -1, NULL));
    CHECK(g_file_set_contents("attach-fixture-requests.jsonl", "", -1, NULL));
    application = gtk_application_new("org.umicom.studio.attach-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG,
                                                          NULL) == UMI_STATUS_OK);
    GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
    gtk_window_present(window);
    Drain();
    attach = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.debug.attach");
    GtkWidget *pid =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.debug.attach.pid");
    CHECK(attach != NULL && pid != NULL);
    g_object_ref(attach);
    for (GtkWidget *parent = gtk_widget_get_parent(attach); parent != NULL;
         parent = gtk_widget_get_parent(parent))
        if (GTK_IS_EXPANDER(parent))
            gtk_expander_set_expanded(GTK_EXPANDER(parent), TRUE);
    gtk_editable_set_text(GTK_EDITABLE(pid), invalid ? "not-a-process" : "123456");
    Drain();
    CHECK(gtk_widget_get_mapped(attach));
    if (revoke)
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    if (hidden)
        gtk_widget_set_visible(attach, FALSE);
    if (retained)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    /* Emitting a retained or hidden control must not bypass its workbench owner. */
    g_signal_emit_by_name(attach, "clicked");
    bool allowed = !denied && !invalid && !retained && !hidden && !revoke;
    CHECK(UmiStudioDebuggerNativeBusy(debugger) == allowed);
    if (allowed)
        CHECK(umi_studio_debugger_service_stop(debugger, 0) == UMI_STATUS_OK);
    gsize bytes = 0U;
    CHECK(g_file_get_contents("attach-fixture-requests.jsonl", &log, &bytes, NULL));
    if (!allowed)
        CHECK(bytes == 0U);
    else
        CHECK(strstr(log, "\"command\":\"attach\"") != NULL &&
              strstr(log, "\"terminateDebuggee\":false") != NULL &&
              strstr(log, "\"terminateDebuggee\":true") == NULL);
done:
    umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&attach);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved && g_chdir(original) != 0)
        failed = 1;
    if (previous != NULL)
        g_setenv("UMICOM_STUDIO_DATA_PATH", previous, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    if (directory != NULL)
        printf("Isolated attach fixture retained: %s\n", directory);
    g_free(directory);
    g_free(original);
    g_free(previous);
    g_free(log);
    return failed;
}
