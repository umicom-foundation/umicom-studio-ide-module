/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_process_picker_gtk4.c
 * PURPOSE: Exercise process selection and manual PID editing through the real Studio workbench.
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
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif
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
    bool denied = false, revoke = strcmp(mode, "revoke") == 0,
         retained_mode = strcmp(mode, "retained") == 0;
    bool manual = strcmp(mode, "manual") == 0;
    if (strcmp(mode, "choose") != 0 && !revoke && !retained_mode && !manual)
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

    GtkWidget *picker =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "process.picker");
    GtkWidget *refresh = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.refresh");
    GtkWidget *search = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.search");
    CHECK(picker != NULL && refresh != NULL && search != NULL);
    for (GtkWidget *parent = gtk_widget_get_parent(refresh); parent != NULL;
         parent = gtk_widget_get_parent(parent))
        if (GTK_IS_EXPANDER(parent))
            gtk_expander_set_expanded(GTK_EXPANDER(parent), TRUE);
    Drain();
    g_signal_emit_by_name(refresh, "clicked");
    gint64 until = g_get_monotonic_time() + 5000000;
    while (!gtk_widget_get_sensitive(refresh) && g_get_monotonic_time() < until)
        Drain();
    CHECK(gtk_widget_get_sensitive(refresh));
#if defined(_WIN32)
    uint64_t current_pid = GetCurrentProcessId();
#else
    uint64_t current_pid = (uint64_t)getpid();
#endif
    char filter[64], expected[32];
    g_snprintf(filter, sizeof filter, "pid:%llu", (unsigned long long)current_pid);
    g_snprintf(expected, sizeof expected, "%llu", (unsigned long long)current_pid);
    gtk_editable_set_text(GTK_EDITABLE(search), filter);
    Drain();
    GtkWidget *choice = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.choose.0");
    CHECK(choice != NULL && gtk_widget_get_sensitive(choice));
    g_object_ref(choice);
    if (revoke)
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    if (retained_mode)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    g_signal_emit_by_name(choice, "clicked");
    g_object_unref(choice);
    if (!retained_mode)
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(pid)), revoke ? "" : expected) == 0);
    CHECK(!UmiStudioDebuggerNativeBusy(debugger));
    if (manual)
    {
        /* The controlled adapter never attaches a real process. Changing the
         * field must discard the creation identity of the earlier observation. */
        gtk_editable_set_text(GTK_EDITABLE(pid), "123456");
        g_signal_emit_by_name(attach, "clicked");
        CHECK(UmiStudioDebuggerNativeBusy(debugger));
        CHECK(umi_studio_debugger_service_stop(debugger, 0) == UMI_STATUS_OK);
    }
    gsize bytes = 0U;
    CHECK(g_file_get_contents("attach-fixture-requests.jsonl", &log, &bytes, NULL));
    CHECK(manual ? strstr(log, "\"command\":\"attach\"") != NULL : bytes == 0U);
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
