/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_setup_gtk4.c
 * PURPOSE: Exercise reviewed debug setup files, retained controls and stale approvals in Studio.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/debug/setup_document.h"
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

static int Wait(GtkWidget *panel)
{
    gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
    while (g_object_get_data(G_OBJECT(panel), "umicom-debug-setup-pending") != NULL &&
           g_get_monotonic_time() < deadline)
    {
        g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    return g_object_get_data(G_OBJECT(panel), "umicom-debug-setup-pending") == NULL;
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
    GtkWidget *entry = NULL, *title = NULL, *save = NULL, *load = NULL, *apply = NULL, *previous = NULL,
              *approval = NULL, *panel = NULL;
    UmiDebugSetup *setup = NULL, *loaded = NULL;
    char *directory = NULL, *original = NULL, *originalData = NULL;
    GError *error = NULL;
    original = g_get_current_dir();
    originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-debug-setup-XXXXXX", &error);
    CHECK(original != NULL && directory != NULL && g_chdir(directory) == 0);
    changed = 1;

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
    application = gtk_application_new("org.umicom.studio.debug-setup-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));

    const char *ids[] = {"studio.debug.setup.path",    "studio.debug.setup.title",
                         "studio.debug.setup.save",    "studio.debug.setup.load",
                         "studio.debug.setup.apply",   "studio.debug.setup.previous",
                         "studio.debug.setup.approval"};
    GtkWidget **controls[] = {&entry, &title, &save, &load, &apply, &previous, &approval};
    for (size_t i = 0U; i < 7U; ++i)
    {
        *controls[i] = Find(root, ids[i]);
        CHECK(*controls[i] != NULL);
        g_object_ref(*controls[i]);
    }
    panel = g_object_ref(gtk_widget_get_parent(entry));
    char path[UMI_PATH_CAPACITY];
    CHECK(umi_path_join(directory, "setup.json", path, sizeof path) == UMI_STATUS_OK);
    gtk_editable_set_text(GTK_EDITABLE(entry), path);
    CHECK(umi_studio_debugger_service_add_breakpoint(debugger, "C:/source/original.c", 11, 0) ==
          UMI_STATUS_OK);
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(debugger);
    if (strcmp(mode, "save") == 0 || strcmp(mode, "save-close") == 0)
    {
        g_signal_emit_by_name(save, "clicked");
        if (strcmp(mode, "save-close") == 0)
        {
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            umi_studio_bootstrap_destroy(bootstrap);
            bootstrap = NULL;
        }
        CHECK(Wait(panel));
        CHECK(UmiDebugSetupLoad(path, &loaded) == UMI_STATUS_OK);
        UmiDebugSetupBreakpoint point;
        CHECK(UmiDebugSetupBreakpointAt(loaded, 0U, &point) == UMI_STATUS_OK && point.line == 11U);
    }
    else
    {
        CHECK(UmiDebugSetupCreate("Loaded settings", &setup) == UMI_STATUS_OK);
        UmiDebugSetupBreakpoint point = {0};
        strcpy(point.source, "C:/source/loaded.c");
        point.line = 42U;
        point.enabled = 1;
        CHECK(UmiDebugSetupAddBreakpoint(setup, &point) == UMI_STATUS_OK);
        UmiDebugSetupWatch watch = {0};
        strcpy(watch.expression, "count");
        watch.enabled = 1;
        CHECK(UmiDebugSetupAddWatch(setup, &watch) == UMI_STATUS_OK);
        if (strcmp(mode, "missing") != 0)
            CHECK(UmiDebugSetupSaveNew(path, setup, NULL) == UMI_STATUS_OK);
        if (strcmp(mode, "malformed") == 0)
            CHECK(g_file_set_contents(path, "broken", -1, NULL));
        g_signal_emit_by_name(load, "clicked");
        if (strcmp(mode, "changed") == 0)
            gtk_editable_set_text(GTK_EDITABLE(title), "Newer draft");
        if (strcmp(mode, "load-close") == 0)
        {
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            umi_studio_bootstrap_destroy(bootstrap);
            bootstrap = NULL;
        }
        CHECK(Wait(panel));
        if (bootstrap != NULL)
        {
            CHECK(umi_debug_workspace_breakpoint_at(workspace, 0U, &point) == UMI_STATUS_OK &&
                  point.line == 11U);
            if (strcmp(mode, "stale") == 0)
                CHECK(umi_debug_workspace_add_watch(workspace, "newer watch", NULL, 0U) == UMI_STATUS_OK);
            if (strcmp(mode, "no-approval") != 0)
                gtk_check_button_set_active(GTK_CHECK_BUTTON(approval), TRUE);
            g_signal_emit_by_name(apply, "clicked");
            int accepted = strcmp(mode, "apply") == 0 || strcmp(mode, "previous") == 0;
            CHECK(umi_debug_workspace_breakpoint_at(workspace, 0U, &point) == UMI_STATUS_OK &&
                  point.line == (accepted ? 42U : 11U));
            if (strcmp(mode, "previous") == 0)
            {
                g_signal_emit_by_name(previous, "clicked");
                gtk_check_button_set_active(GTK_CHECK_BUTTON(approval), TRUE);
                g_signal_emit_by_name(apply, "clicked");
                CHECK(umi_debug_workspace_breakpoint_at(workspace, 0U, &point) == UMI_STATUS_OK &&
                      point.line == 11U);
            }
            CHECK(!UmiStudioDebuggerNativeBusy(debugger));
        }
    }
cleanup:
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);

    if (panel != NULL && !Wait(panel))
        failed = 1;
    /* Retained controls, including editable inputs, must be inert after close. */
    GtkWidget *buttons[] = {save, load, apply, previous};
    for (size_t i = 0U; i < 4U; ++i)
        if (buttons[i] != NULL)
            g_signal_emit_by_name(buttons[i], "clicked");
    if (entry != NULL)
        gtk_editable_set_text(GTK_EDITABLE(entry), "retained input");
    g_clear_object(&panel);
    if (title != NULL)
        gtk_editable_set_text(GTK_EDITABLE(title), "after panel release");
    GtkWidget *retained[] = {entry, title, save, load, apply, previous, approval};
    for (size_t i = 0U; i < 7U; ++i)
        if (retained[i] != NULL)
            g_object_unref(retained[i]);
    UmiDebugSetupDestroy(setup);
    UmiDebugSetupDestroy(loaded);
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
        printf("Retained isolated debug setup files: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(original);
    g_free(originalData);
    return failed;
}
