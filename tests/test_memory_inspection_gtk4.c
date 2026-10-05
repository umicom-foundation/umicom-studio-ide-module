/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_memory_inspection_gtk4.c
 * PURPOSE: Exercise memory reads, draft validation, stale captures and retained native controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

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

/* A text observer can react while a new report is being published. A second
 * click in that callback must not recursively issue another adapter request. */
static void RepeatMemoryRead(GtkTextBuffer *buffer, gpointer data)
{
    (void)buffer;
    g_signal_emit_by_name(data, "clicked");
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
    GtkWidget *retained = NULL, *retainedChild = NULL;
    UmiDebugRuntimePlatform *platform = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check())
        return 77;
    originalDirectory = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-memory-inspection-XXXXXX", &error);
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

    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(debugger);
    UmiDebugService *model = umi_studio_debugger_service_model(debugger);
    platform = UmiStudioDebuggerNativePlatform(debugger);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
    (void)snprintf(profile.arguments, sizeof profile.arguments, "memory-inspection-%s", name);
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
    application = gtk_application_new("org.umicom.studio.memory-inspection-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));

    GtkWidget *scope = Find(root, "studio.debug.variable.inspect.scope.0");
    CHECK(GTK_IS_BUTTON(scope));
    g_signal_emit_by_name(scope, "clicked");
    GtkWidget *button = Find(root, "studio.debug.variable.memory-read.scope.0.0");
    GtkWidget *offset = Find(root, "studio.debug.variable.memory-offset.scope.0.0");
    GtkWidget *count = Find(root, "studio.debug.variable.memory-count.scope.0.0");
    GtkWidget *output = Find(root, "studio.debug.variable.memory-output.scope.0.0");
    GtkWidget *label = Find(root, "studio.debug.variable.memory-status.scope.0.0");
    CHECK(GTK_IS_BUTTON(button) && GTK_IS_ENTRY(offset) && GTK_IS_SPIN_BUTTON(count) &&
          GTK_IS_TEXT_VIEW(output) && GTK_IS_LABEL(label));
    CHECK(gtk_widget_get_sensitive(button) == (strcmp(name, "unsupported") != 0));
    retained = g_object_ref(button);
    retainedChild = g_object_ref(offset);
    UmiDebugRuntimePlatformSnapshot before, after;
    CHECK(umi_debug_runtime_platform_snapshot(platform, &before) == UMI_STATUS_OK);
    if (strcmp(name, "offset") == 0)
        gtk_editable_set_text(GTK_EDITABLE(offset), "-16");
    if (strcmp(name, "invalid-offset") == 0)
        gtk_editable_set_text(GTK_EDITABLE(offset), "12x");
    if (strcmp(name, "invalid-count") == 0)
        gtk_editable_set_text(GTK_EDITABLE(count), "4097");
    if (strcmp(name, "close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        platform = NULL;
        g_signal_emit_by_name(retained, "clicked");
        gtk_editable_set_text(GTK_EDITABLE(retainedChild), "1");
    }
    else if (strcmp(name, "stale") == 0 || strcmp(name, "detached") == 0)
    {
        UmiDebugVariableSnapshot variable;
        CHECK(umi_debug_workspace_variable_at(workspace, 0U, &variable) == UMI_STATUS_OK);
        CHECK(umi_debug_variable_registry_upsert(umi_debug_service_variable(model), &variable) ==
              UMI_STATUS_OK);
        if (strcmp(name, "detached") == 0)
            CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        g_signal_emit_by_name(retained, "clicked");
    }
    else
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));
        if (strcmp(name, "reentrant") == 0)
            g_signal_connect_object(buffer, "changed", G_CALLBACK(RepeatMemoryRead), button, 0);
        /* Typing is not a request; only the explicit action can contact DAP. */
        CHECK(umi_debug_runtime_platform_snapshot(platform, &after) == UMI_STATUS_OK);
        CHECK(after.adapter.messages_sent == before.adapter.messages_sent);
        g_signal_emit_by_name(button, "clicked");
        CHECK(umi_debug_runtime_platform_snapshot(platform, &after) == UMI_STATUS_OK);
        int blocked = strcmp(name, "invalid-offset") == 0 || strcmp(name, "invalid-count") == 0 ||
                      strcmp(name, "unsupported") == 0;
        CHECK(after.adapter.messages_sent == before.adapter.messages_sent + (blocked ? 0U : 1U));
        if (!blocked && strcmp(name, "malformed") != 0)
        {
            GtkTextIter start, end;
            gtk_text_buffer_get_bounds(buffer, &start, &end);
            char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
            CHECK(strstr(text, "41 42 43 00 7F 80") != NULL && strstr(text, "|ABC...|") != NULL);
            if (strcmp(name, "offset") == 0)
                CHECK(strstr(text, "offset: -16") != NULL);
            if (strcmp(name, "failed-refresh") == 0)
            {
                g_signal_emit_by_name(button, "clicked");
                gtk_text_buffer_get_bounds(buffer, &start, &end);
                char *again = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
                CHECK(strcmp(text, again) == 0 &&
                      strstr(gtk_label_get_text(GTK_LABEL(label)), "not replaced") != NULL);
                g_free(again);
            }
            else
                CHECK(strcmp(name, "success") == 0 || strcmp(name, "offset") == 0 ||
                      strcmp(name, "reentrant") == 0);
            g_free(text);
        }
        else
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), "not replaced") != NULL &&
                  gtk_text_buffer_get_char_count(buffer) == 0);
    }
    if (platform != NULL && (strcmp(name, "stale") == 0 || strcmp(name, "detached") == 0))
    {
        CHECK(umi_debug_runtime_platform_snapshot(platform, &after) == UMI_STATUS_OK);
        CHECK(after.adapter.messages_sent == before.adapter.messages_sent);
    }
cleanup:
    if (retained != NULL)
        g_object_unref(retained);
    if (retainedChild != NULL)
        g_object_unref(retainedChild);
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
        printf("Retained isolated memory inspection fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(originalDirectory);
    g_free(originalData);
    return failed;
}
