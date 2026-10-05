/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_adapter_preferences_gtk4.c
 * PURPOSE: Exercise saved debugger drafts, pending edits and retained controls through the native Studio panel.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/debug_runtime/adapter_preferences.h"
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
static char preferenceRoot[UMI_PATH_CAPACITY];
/* Only path resolution is replaced for this executable. Real Framework JSON,
 * native file replacement, GTK workers and Studio actions are exercised. */
UmiStatus FixtureAdapterPreferencesPath(const char *application, const char *base, char *out, size_t capacity)
{
    (void)application;
    (void)base;
    return umi_path_join(preferenceRoot, "adapter.json", out, capacity);
}
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
static int Wait(GtkWidget *entry)
{
    gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
    while (g_object_get_data(G_OBJECT(entry), "umicom-debug-preference-pending") != NULL &&
           g_get_monotonic_time() < deadline)
    {
        g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    return g_object_get_data(G_OBJECT(entry), "umicom-debug-preference-pending") == NULL;
}
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!gtk_init_check())
        return 77;
    const char *mode = argv[1];
    int failed = 0, changed = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *entry = NULL, *kind = NULL, *save = NULL, *load = NULL, *use = NULL;
    char *directory = NULL, *original = NULL, *originalData = NULL;
    GError *error = NULL;
    original = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-adapter-preferences-XXXXXX", &error);
    CHECK(original != NULL && directory != NULL && g_chdir(directory) == 0);
    changed = 1;
    CHECK(g_strlcpy(preferenceRoot, directory, sizeof(preferenceRoot)) < sizeof(preferenceRoot));
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
    UmiStudioDebuggerService *debugger =
        umi_studio_services_debugger(umi_studio_bootstrap_services(bootstrap));
    CHECK(UmiStudioDebuggerConfigureNative(debugger, "lldb", "") == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.adapter-preferences-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    const char *ids[] = {"studio.debug.adapter.executable", "studio.debug.adapter.kind",
                         "studio.debug.adapter.save", "studio.debug.adapter.load",
                         "studio.debug.adapter.use"};
    GtkWidget **controls[] = {&entry, &kind, &save, &load, &use};
    for (size_t i = 0U; i < 5U; ++i)
    {
        *controls[i] = Find(root, ids[i]);
        CHECK(*controls[i] != NULL);
        g_object_ref(*controls[i]);
    }
    CHECK(gtk_drop_down_get_selected(GTK_DROP_DOWN(kind)) == 1U);
    char path[UMI_PATH_CAPACITY];
    CHECK(FixtureAdapterPreferencesPath("Studio", NULL, path, sizeof(path)) == UMI_STATUS_OK);
    UmiDebugAdapterPreferences expected = {0}, actual = {0};
    strcpy(expected.kind, "gdb");
#ifdef _WIN32
    strcpy(expected.executable, "C:/tools/caf\xc3\xa9/gdb.exe");
#else
    strcpy(expected.executable, "/tools/caf\xc3\xa9/gdb");
#endif
    if (strcmp(mode, "current") == 0)
        goto cleanup;
    if (strcmp(mode, "save") == 0 || strcmp(mode, "save-close") == 0)
    {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(kind), 0U);
        gtk_editable_set_text(GTK_EDITABLE(entry), expected.executable);
        g_signal_emit_by_name(save, "clicked");
        if (strcmp(mode, "save-close") == 0)
        {
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            umi_studio_bootstrap_destroy(bootstrap);
            bootstrap = NULL;
        }
        CHECK(Wait(entry));
        CHECK(UmiDebugAdapterPreferencesLoad(path, &actual) == UMI_STATUS_OK &&
              memcmp(&actual, &expected, sizeof(actual)) == 0);
    }
    else
    {
        if (strcmp(mode, "missing") != 0)
            CHECK(UmiDebugAdapterPreferencesSave(path, &expected) == UMI_STATUS_OK);
        gtk_editable_set_text(GTK_EDITABLE(entry), "original-draft");
        if (strcmp(mode, "malformed") == 0)
            CHECK(g_file_set_contents(path, "invalid json", -1, NULL));
        g_signal_emit_by_name(load, "clicked");
        g_signal_emit_by_name(use, "clicked"); /* Pending drafts never become active settings. */
        char currentKind[16], currentPath[1024];
        CHECK(UmiStudioDebuggerNativeChoice(debugger, currentKind, sizeof(currentKind), currentPath,
                                            sizeof(currentPath)) == UMI_STATUS_OK);
        CHECK(strcmp(currentKind, "lldb") == 0 && currentPath[0] == '\0');
        if (strcmp(mode, "changed") == 0)
            gtk_editable_set_text(GTK_EDITABLE(entry), "newer-draft");
        if (strcmp(mode, "load-close") == 0)
        {
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            umi_studio_bootstrap_destroy(bootstrap);
            bootstrap = NULL;
        }
        CHECK(Wait(entry));
        if (strcmp(mode, "load-close") != 0)
        {
            int loaded = strcmp(mode, "load") == 0 || strcmp(mode, "use") == 0;
            CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(entry)), loaded ? expected.executable
                                                                     : strcmp(mode, "changed") == 0
                                                                         ? "newer-draft"
                                                                         : "original-draft") == 0);
            CHECK(gtk_drop_down_get_selected(GTK_DROP_DOWN(kind)) == (loaded ? 0U : 1U));
            if (strcmp(mode, "use") == 0)
            {
                g_signal_emit_by_name(use, "clicked");
                CHECK(UmiStudioDebuggerNativeChoice(debugger, currentKind, sizeof(currentKind), currentPath,
                                                    sizeof(currentPath)) == UMI_STATUS_OK);
                CHECK(strcmp(currentKind, expected.kind) == 0 &&
                      strcmp(currentPath, expected.executable) == 0);
            }
            CHECK(!UmiStudioDebuggerNativeBusy(debugger));
        }
    }
cleanup:
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    if (entry != NULL && !Wait(entry))
        failed = 1;
    /* Strongly retained controls must do nothing once their weak owner is gone. */
    if (save != NULL)
        g_signal_emit_by_name(save, "clicked");
    if (load != NULL)
        g_signal_emit_by_name(load, "clicked");
    if (use != NULL)
        g_signal_emit_by_name(use, "clicked");
    GtkWidget *retained[] = {entry, kind, save, load, use};
    for (size_t i = 0U; i < 5U; ++i)
        if (retained[i] != NULL)
            g_object_unref(retained[i]);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed && g_chdir(original) != 0)
        failed = 1;
    if (isolated)
    {
        if (originalData != NULL)
            g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE);
        else
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL)
        fprintf(stderr, "%s\n", error->message);
    if (directory != NULL)
        printf("Retained isolated adapter preferences: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(original);
    g_free(originalData);
    return failed;
}
