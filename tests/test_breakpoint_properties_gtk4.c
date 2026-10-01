/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_breakpoint_properties_gtk4.c
 * PURPOSE: Exercise actual property row callbacks, draft retention and native lifetime guards.
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
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)
/* Find semantic controls without depending on child positions or labels. */
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    if (root == NULL) return NULL;
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = Find(child, tag);
        if (found != NULL) return found;
    }
    return NULL;
}


/* Each case uses an isolated product graph and unpresented owner window.
 * No adapter, compiler, user project, session restore or remote AI is used. */
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *name = argv[1]; int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0}; UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *retained = NULL, *retainedRemove = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL; GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    originalDirectory = g_get_current_dir(); originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-breakpoint-properties-XXXXXX", &error);
    CHECK(originalDirectory != NULL && directory != NULL && g_chdir(directory) == 0); changedDirectory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiStudioDebuggerService *debugger = umi_studio_services_debugger(umi_studio_bootstrap_services(bootstrap));
    CHECK(umi_studio_debugger_service_add_breakpoint(debugger, "main.c", 13, 1) == UMI_STATUS_OK);
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(debugger);
    UmiDebugService *model = umi_studio_debugger_service_model(debugger);
    application = gtk_application_new("org.umicom.studio.breakpoint-properties-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *condition = Find(root, "studio.debug.breakpoint.condition.0"), *log = Find(root, "studio.debug.breakpoint.log.0");
    GtkWidget *enabled = Find(root, "studio.debug.breakpoint.enabled.0"), *apply = Find(root, "studio.debug.breakpoint.apply.0");
    GtkWidget *remove = Find(root, "studio.debug.breakpoint.remove.0"), *reset = Find(root, "studio.debug.breakpoint.reset.0");
    CHECK(GTK_IS_ENTRY(condition) && GTK_IS_ENTRY(log) && GTK_IS_CHECK_BUTTON(enabled) && GTK_IS_BUTTON(apply) && GTK_IS_BUTTON(remove) && GTK_IS_BUTTON(reset));
    retained = g_object_ref(apply); retainedRemove = g_object_ref(remove);
    gtk_editable_set_text(GTK_EDITABLE(condition), "count > 3"); gtk_editable_set_text(GTK_EDITABLE(log), "count={count}");
    UmiDebugBreakpointSnapshot before, after; CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &before) == UMI_STATUS_OK);
    CHECK(before.condition[0] == '\0' && before.log_message[0] == '\0');
    if (strcmp(name, "draft-refresh") == 0) {
        UmiDebugWatchSnapshot watch = {0}; strcpy(watch.id, "watch");
        CHECK(umi_debug_watch_registry_upsert(umi_debug_service_watch(model), &watch) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(Find(root, "studio.debug.breakpoint.apply.0") == retained);
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(condition)), "count > 3") == 0);
    } else if (strcmp(name, "reset") == 0) {
        g_signal_emit_by_name(reset, "clicked");
        CHECK(gtk_editable_get_text(GTK_EDITABLE(condition))[0] == '\0' && gtk_editable_get_text(GTK_EDITABLE(log))[0] == '\0');
        CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_OK && after.revision == before.revision);
    } else if (strcmp(name, "close") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap); bootstrap = NULL;
        g_signal_emit_by_name(retained, "clicked"); g_signal_emit_by_name(retainedRemove, "clicked");
    } else if (strcmp(name, "stale") == 0 || strcmp(name, "detached") == 0) {
        after = before; after.line = 99;
        CHECK(umi_debug_breakpoint_registry_upsert(umi_debug_service_breakpoint(model), &after) == UMI_STATUS_OK);
        if (strcmp(name, "detached") == 0) CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        g_signal_emit_by_name(retained, "clicked"); g_signal_emit_by_name(retainedRemove, "clicked");
        CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_OK && after.line == 99 && after.condition[0] == '\0');
    } else if (strcmp(name, "long-text") == 0) {
        char text[513]; memset(text, 'x', sizeof text - 1U); text[sizeof text - 1U] = '\0';
        gtk_editable_set_text(GTK_EDITABLE(condition), text); g_signal_emit_by_name(apply, "clicked");
        CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_OK && after.revision == before.revision);
    } else if (strcmp(name, "remove") == 0) {
        g_signal_emit_by_name(remove, "clicked"); CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_NOT_FOUND);
        g_signal_emit_by_name(retained, "clicked");
    } else {
        CHECK(strcmp(name, "apply") == 0 || strcmp(name, "disable") == 0);
        if (strcmp(name, "disable") == 0) gtk_check_button_set_active(GTK_CHECK_BUTTON(enabled), FALSE);
        g_signal_emit_by_name(apply, "clicked");
        CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_OK);
        CHECK(strcmp(after.condition, "count > 3") == 0 && strcmp(after.log_message, "count={count}") == 0 && !after.verified);
        CHECK(after.enabled == (strcmp(name, "disable") != 0));
        g_signal_emit_by_name(retainedRemove, "clicked");
        CHECK(umi_debug_workspace_breakpoint_at(workspace, 0, &after) == UMI_STATUS_OK);
    }
cleanup:
    if (retained != NULL) g_object_unref(retained);
    if (retainedRemove != NULL) g_object_unref(retainedRemove);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0) failed = 1;
    if (isolated) {
        if (originalData != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated breakpoint fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(originalDirectory); g_free(originalData);
    return failed;
}
