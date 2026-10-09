/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_restart_gtk4.c
 * PURPOSE: Exercise restart through the product control and retain a button after its owner closes.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/debugger.h"
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
/* Find semantic controls without depending on child positions or labels. */
/* Rendered-child traversal missed controls owned by collapsed inspectors. The Framework logical lookup preserves those controls and rejects ambiguous identities. The original fixture traversal is retained for review. The previous implementation is retained for engineering review. */
#if 0
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
#endif
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    /* Inspect logical children without expanding or activating a panel. */
    return umi_gtk4_automation_find_tagged_widget(root, tag);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    const char *name = argv[2];
    int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *retained = NULL;
    UmiDebugRuntimePlatform *platform = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check())
        return 77;
    originalDirectory = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-debug-restart-XXXXXX", &error);
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
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiStudioDebuggerService *debugger =
        umi_studio_services_debugger(umi_studio_bootstrap_services(bootstrap));

    UmiDebugService *model = umi_studio_debugger_service_model(debugger);
    platform = UmiStudioDebuggerNativePlatform(debugger);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
    (void)snprintf(profile.arguments, sizeof profile.arguments, "session-restart-%s",
                   strcmp(name, "close") == 0 ? "supported" : name);
    CHECK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile) ==
          UMI_STATUS_OK);
    CHECK(umi_debug_runtime_platform_start(platform, profile.id, "session", "launch", "{}", 0, NULL, 2000U) ==
          UMI_STATUS_OK);
    UmiDebugRuntimePlatformSnapshot state;
    for (unsigned i = 0U; i < 80U; ++i)
    {
        CHECK(umi_debug_runtime_platform_snapshot(platform, &state) == UMI_STATUS_OK);
        if (state.paused)
            break;
        int handled = 0;
        UmiStatus status = umi_debug_runtime_platform_pump_event(platform, 25U, &handled);
        CHECK(status == UMI_STATUS_OK || status == UMI_STATUS_NOT_FOUND);
    }
    CHECK(state.paused);
    CHECK(UmiStudioDebuggerPollNative(debugger, umi_studio_services_build(umi_studio_bootstrap_services(
                                                    bootstrap))) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.debug-restart-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));

    GtkWidget *restart = Find(root, "studio.debug.restart");
    CHECK(GTK_IS_BUTTON(restart));
    retained = g_object_ref(restart);
    UmiDebugRuntimePlatformSnapshot before, after;
    CHECK(umi_debug_runtime_platform_snapshot(platform, &before) == UMI_STATUS_OK);
    if (strcmp(name, "close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        platform = NULL;
        g_signal_emit_by_name(retained, "clicked");
    }
    else
    {
        g_signal_emit_by_name(restart, "clicked");
        CHECK(umi_debug_runtime_platform_snapshot(platform, &after) == UMI_STATUS_OK);
        if (strcmp(name, "unsupported") == 0)
            CHECK(after.adapter.messages_sent == before.adapter.messages_sent && after.paused);
        else
        {
            CHECK(strcmp(name, "supported") == 0);
            CHECK(after.adapter.messages_sent > before.adapter.messages_sent);
        }
    }
cleanup:
    if (retained != NULL)
        g_object_unref(retained);
    if (platform != NULL)
        (void)umi_debug_runtime_platform_stop(platform, 1, 2000U);
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
        printf("Retained isolated debug restart fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(originalDirectory);
    g_free(originalData);
    return failed;
}
