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
/* Wait for the actual native mapping needed by process actions. A short fixed
 * drain is not proof that GTK has presented an expanded panel under build load. */
static bool WaitMapped(GtkWidget *widget)
{
    gint64 deadline = g_get_monotonic_time() + 5 * G_TIME_SPAN_SECOND;
    while (!gtk_widget_get_mapped(widget) && g_get_monotonic_time() < deadline)
    {
        /* Bound dispatch even if a source continuously reschedules itself. */
        for (unsigned i = 0U; i < 64U && g_main_context_pending(NULL); ++i)
            (void)g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    return gtk_widget_get_mapped(widget);
}

/* Observe the native field boundary: selection is not permission to attach, and
 * a callback may replace the requested PID or retire its workspace. */
typedef struct ProcessChoiceProbe {
    UmiStudioGtkWorkbench **owner;
    UmiStudioServices *services;
    GtkWidget *attach;
    const char *mode;
    unsigned calls;
    UmiStatus status;
} ProcessChoiceProbe;
static void ProcessChoiceChanged(GtkEditable *editable, gpointer data)
{
    ProcessChoiceProbe *probe = data;
    if (probe->calls++ != 0U) return;
    if (strcmp(probe->mode, "choose-close") == 0) {
        UmiStudioGtkWorkbench *owner = *probe->owner;
        *probe->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (strcmp(probe->mode, "choose-revoke") == 0) {
        probe->status = umi_studio_workspace_set_trusted(probe->services, 0);
    } else if (strcmp(probe->mode, "choose-edit") == 0) {
        gtk_editable_set_text(editable, "765432");
    } else if (strcmp(probe->mode, "choose-attach") == 0) {
        g_signal_emit_by_name(probe->attach, "clicked");
    }
}

int main(int argc, char **argv)
{
    if (argc != 4)
        return 2;
    const char *mode = argv[1];
    bool denied = false, revoke = strcmp(mode, "revoke") == 0,
         retained_mode = strcmp(mode, "retained") == 0;
    bool manual = strcmp(mode, "manual") == 0;
    bool chooseClose = strcmp(mode, "choose-close") == 0, chooseRevoke = strcmp(mode, "choose-revoke") == 0;
    bool chooseEdit = strcmp(mode, "choose-edit") == 0, chooseAttach = strcmp(mode, "choose-attach") == 0;
    bool observeChoice = chooseClose || chooseRevoke || chooseEdit || chooseAttach;
    if (strcmp(mode, "choose") != 0 && !revoke && !retained_mode && !manual && !observeChoice)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool moved = false;
    char *original = g_get_current_dir(), *previous = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
/* Qualification retains process-discovery evidence in its supplied build-tree directory; keep the earlier temporary-directory choice for review. */
#if 0
    char *directory = g_dir_make_tmp("umicom-studio-attach-XXXXXX", NULL), *log = NULL;
#endif

    char *directory = g_build_filename(argv[3], "process-picker-XXXXXX", NULL), *log = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *attach = NULL, *pid = NULL;
    gulong choiceHandler = 0U;
    ProcessChoiceProbe probe = {&workbench, NULL, NULL, mode, 0U, UMI_STATUS_OK};
    CHECK(directory != NULL && g_mkdtemp(directory) != NULL && original != NULL && g_chdir(directory) == 0);
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
    pid =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "studio.debug.attach.pid");
    CHECK(attach != NULL && pid != NULL);
    g_object_ref(attach);
    g_object_ref(pid);
    /* A collapsed section need not have a native parent chain. Use the
     * Framework logical route, then retain the existing mapped-action assertion. */
#if 0
for (GtkWidget *parent = gtk_widget_get_parent(attach); parent != NULL;
         parent = gtk_widget_get_parent(parent))
        if (GTK_IS_EXPANDER(parent))
            gtk_expander_set_expanded(GTK_EXPANDER(parent), TRUE);
#endif
    CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window),
        g_object_get_data(G_OBJECT(attach), "umicom-automation-id")) == UMI_STATUS_OK);

    GtkWidget *picker =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "process.picker");
    GtkWidget *refresh = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.refresh");
    GtkWidget *search = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.search");
    CHECK(picker != NULL && refresh != NULL && search != NULL);
    /* A collapsed section need not have a native parent chain. Use the
     * Framework logical route, then retain the existing mapped-action assertion. */
#if 0
for (GtkWidget *parent = gtk_widget_get_parent(refresh); parent != NULL;
         parent = gtk_widget_get_parent(parent))
        if (GTK_IS_EXPANDER(parent))
            gtk_expander_set_expanded(GTK_EXPANDER(parent), TRUE);
#endif
    CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window),
        g_object_get_data(G_OBJECT(refresh), "umicom-automation-id")) == UMI_STATUS_OK);
    Drain();
    CHECK(WaitMapped(refresh));
    g_signal_emit_by_name(refresh, "clicked");
    /* A mapped action must enter discovery before readiness can mean finished. */
    CHECK(!gtk_widget_get_sensitive(refresh));
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
    if (choice == NULL || !gtk_widget_get_sensitive(choice)) {
        GtkWidget *state = umi_gtk4_automation_find_tagged_widget(picker, "process.picker.status");
        fprintf(stderr, "Process selection unavailable: %s\n",
            GTK_IS_LABEL(state) ? gtk_label_get_text(GTK_LABEL(state)) : "missing status");
    }
    CHECK(choice != NULL && gtk_widget_get_sensitive(choice));
    CHECK(WaitMapped(choice));
    g_object_ref(choice);
    if (revoke)
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    if (retained_mode)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    if (observeChoice) {
        probe.services = services; probe.attach = attach;
        choiceHandler = g_signal_connect(pid, "changed", G_CALLBACK(ProcessChoiceChanged), &probe);
    }
    g_signal_emit_by_name(choice, "clicked");
    g_object_unref(choice);
    if (observeChoice) CHECK(probe.calls >= 1U && probe.status == UMI_STATUS_OK);
    if (chooseClose) CHECK(workbench == NULL);
    if (!retained_mode && !chooseClose)
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(pid)), revoke ? "" : chooseEdit ? "765432" : expected) == 0);
    CHECK(g_object_get_data(G_OBJECT(pid), "umicom-process-selection-pending") == NULL);
    if (chooseClose || chooseRevoke || chooseEdit)
        CHECK(g_object_get_data(G_OBJECT(pid), "umicom-selected-process") == NULL);
    if (chooseAttach)
        CHECK(g_object_get_data(G_OBJECT(pid), "umicom-selected-process") != NULL);
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
    if (choiceHandler != 0U) g_signal_handler_disconnect(pid, choiceHandler);
    g_clear_object(&pid);
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
