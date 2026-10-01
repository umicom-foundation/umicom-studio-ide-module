/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_scope_inspection_gtk4.c
 * PURPOSE: Verify scopes stay unloaded until clicked and retain safe owned child branches.
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


int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *name = argv[2]; int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0}; UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *retained = NULL, *retainedChild = NULL; UmiDebugRuntimePlatform *platform = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL; GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    originalDirectory = g_get_current_dir(); originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-scope-inspection-XXXXXX", &error);
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

    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(debugger);
    UmiDebugService *model = umi_studio_debugger_service_model(debugger);
    platform = UmiStudioDebuggerNativePlatform(debugger);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
    (void)snprintf(profile.arguments, sizeof profile.arguments, "scope-inspection-%s", name);
    CHECK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile) == UMI_STATUS_OK);
    CHECK(umi_debug_runtime_platform_start(platform, profile.id, "session", "launch", "{}", 0, NULL, 2000U) == UMI_STATUS_OK);
    UmiDebugRuntimePlatformSnapshot state;
    for (unsigned i = 0U; i < 80U; ++i) {
        CHECK(umi_debug_runtime_platform_snapshot(platform, &state) == UMI_STATUS_OK);
        if (state.paused) break;
        int handled = 0; UmiStatus status = umi_debug_runtime_platform_pump_event(platform, 25U, &handled);
        CHECK(status == UMI_STATUS_OK || status == UMI_STATUS_NOT_FOUND);
    }
    CHECK(state.paused);
    CHECK(UmiStudioDebuggerPollNative(debugger, umi_studio_services_build(umi_studio_bootstrap_services(bootstrap))) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.scope-inspection-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));

    const int only = strcmp(name, "only-expensive") == 0;
    const char *scopePath = only ? "scope.0" : "scope.1";
    char tag[128]; (void)snprintf(tag, sizeof tag, "studio.debug.variable.inspect.%s", scopePath);
    GtkWidget *inspect = Find(root, tag);
    (void)snprintf(tag, sizeof tag, "studio.debug.variable.value.%s", scopePath);
    GtkWidget *value = Find(root, tag);
    (void)snprintf(tag, sizeof tag, "studio.debug.variable.collapse.%s", scopePath);
    GtkWidget *collapse = Find(root, tag);
    CHECK(GTK_IS_BUTTON(inspect) && GTK_IS_LABEL(value) && GTK_IS_BUTTON(collapse));
    CHECK(strcmp(gtk_button_get_label(GTK_BUTTON(inspect)), "Inspect scope") == 0);
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(value)), "Globals") != NULL);
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(value)), "expensive scope") != NULL);
    CHECK(gtk_widget_get_sensitive(inspect) && !gtk_widget_get_sensitive(collapse));
    CHECK(umi_debug_variable_registry_count(umi_debug_service_variable(model)) == (only ? 0U : 1U));
    UmiDebugViewStamp before, after;
    CHECK(UmiDebugWorkspaceViewStamp(workspace, &before) == UMI_STATUS_OK);
    retained = g_object_ref(inspect);
    if (strcmp(name, "display") == 0) {
        UmiDebugRuntimePlatformSnapshot start, finish;
        CHECK(umi_debug_runtime_platform_snapshot(platform, &start) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(umi_debug_runtime_platform_snapshot(platform, &finish) == UMI_STATUS_OK);
        CHECK(start.adapter.messages_sent == finish.adapter.messages_sent);
        CHECK(Find(root, "studio.debug.variable.inspect.scope.1") == retained);
        CHECK(Find(root, "studio.debug.variable.value.scope.1.0") == NULL);
    } else if (strcmp(name, "stale") == 0 || strcmp(name, "detached") == 0) {
        UmiDebugScopeSnapshot scope; CHECK(umi_debug_workspace_scope_at(workspace, 1U, &scope) == UMI_STATUS_OK);
        CHECK(umi_debug_scope_registry_upsert(umi_debug_service_scope(model), &scope) == UMI_STATUS_OK);
        if (strcmp(name, "detached") == 0) CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        UmiDebugRuntimePlatformSnapshot start, finish;
        CHECK(umi_debug_runtime_platform_snapshot(platform, &start) == UMI_STATUS_OK);
        g_signal_emit_by_name(retained, "clicked");
        CHECK(umi_debug_runtime_platform_snapshot(platform, &finish) == UMI_STATUS_OK);
        CHECK(start.adapter.messages_sent == finish.adapter.messages_sent);
        CHECK(Find(root, "studio.debug.variable.value.scope.1.0") == NULL);
    } else {
        g_signal_emit_by_name(inspect, "clicked");
        CHECK(UmiDebugWorkspaceViewStamp(workspace, &after) == UMI_STATUS_OK);
        CHECK(UmiDebugViewStampEqual(&before, &after));
        (void)snprintf(tag, sizeof tag, "studio.debug.variable.inspect.%s.0", scopePath);
        GtkWidget *child = Find(root, tag);
        (void)snprintf(tag, sizeof tag, "studio.debug.variable.value.%s.0", scopePath);
        GtkWidget *childValue = Find(root, tag);
        if (strcmp(name, "empty") == 0 || strcmp(name, "rejected") == 0) {
            CHECK(child == NULL && childValue == NULL);
            CHECK(gtk_widget_get_sensitive(collapse) == (strcmp(name, "empty") == 0));
        } else {
            CHECK(GTK_IS_BUTTON(child) && GTK_IS_LABEL(childValue)); retainedChild = g_object_ref(child);
            if (strcmp(name, "close") == 0) {
                umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
                umi_studio_bootstrap_destroy(bootstrap); bootstrap = NULL; platform = NULL;
                g_signal_emit_by_name(retained, "clicked"); g_signal_emit_by_name(retainedChild, "clicked");
            } else if (strcmp(name, "cycle") == 0) CHECK(!gtk_widget_get_sensitive(child));
            else if (strcmp(name, "collapse") == 0) {
                g_signal_emit_by_name(collapse, "clicked"); CHECK(Find(root, "studio.debug.variable.inspect.scope.1.0") == NULL);
                UmiDebugRuntimePlatformSnapshot start, finish;
                CHECK(umi_debug_runtime_platform_snapshot(platform, &start) == UMI_STATUS_OK);
                g_signal_emit_by_name(retainedChild, "clicked");
                CHECK(umi_debug_runtime_platform_snapshot(platform, &finish) == UMI_STATUS_OK);
                CHECK(start.adapter.messages_sent == finish.adapter.messages_sent);
            } else if (strcmp(name, "nested") == 0) {
                g_signal_emit_by_name(child, "clicked"); GtkWidget *leaf = Find(root, "studio.debug.variable.value.scope.1.0.0");
                CHECK(GTK_IS_LABEL(leaf) && strstr(gtk_label_get_text(GTK_LABEL(leaf)), "leaf = 7") != NULL);
            } else if (strcmp(name, "selection") == 0) {
                UmiDebugScopeSnapshot scope; CHECK(umi_debug_workspace_scope_at(workspace, 2U, &scope) == UMI_STATUS_OK);
                CHECK(umi_debug_workspace_select_scope(workspace, scope.id) == UMI_STATUS_OK);
                CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
                CHECK(Find(root, "studio.debug.variable.value.scope.1.0") == NULL);
                UmiDebugRuntimePlatformSnapshot start, finish;
                CHECK(umi_debug_runtime_platform_snapshot(platform, &start) == UMI_STATUS_OK);
                g_signal_emit_by_name(retainedChild, "clicked");
                CHECK(umi_debug_runtime_platform_snapshot(platform, &finish) == UMI_STATUS_OK);
                CHECK(start.adapter.messages_sent == finish.adapter.messages_sent);
            } else if (strcmp(name, "refresh") == 0 || strcmp(name, "failed-refresh") == 0) {
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(childValue)), "capture 1") != NULL);
                g_signal_emit_by_name(inspect, "clicked");
                CHECK((Find(root, "studio.debug.variable.inspect.scope.1.0") == retainedChild) == (strcmp(name, "failed-refresh") == 0));
                childValue = Find(root, "studio.debug.variable.value.scope.1.0"); CHECK(GTK_IS_LABEL(childValue));
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(childValue)), strcmp(name, "failed-refresh") == 0 ? "capture 1" : "capture 2") != NULL);
            } else if (strcmp(name, "budget") == 0) {
                for (size_t i = 0U; i < 7U; ++i) {
                    (void)snprintf(tag, sizeof tag, "studio.debug.variable.inspect.scope.1.%zu", i);
                    GtkWidget *branch = Find(root, tag); CHECK(GTK_IS_BUTTON(branch)); g_signal_emit_by_name(branch, "clicked");
                    (void)snprintf(tag, sizeof tag, "studio.debug.variable.value.scope.1.%zu.127", i); CHECK(Find(root, tag) != NULL);
                }
                GtkWidget *other = Find(root, "studio.debug.variable.inspect.scope.2"); CHECK(GTK_IS_BUTTON(other));
                UmiDebugRuntimePlatformSnapshot start, finish;
                CHECK(umi_debug_runtime_platform_snapshot(platform, &start) == UMI_STATUS_OK);
                g_signal_emit_by_name(other, "clicked");
                CHECK(umi_debug_runtime_platform_snapshot(platform, &finish) == UMI_STATUS_OK);
                CHECK(start.adapter.messages_sent == finish.adapter.messages_sent);
                CHECK(Find(root, "studio.debug.variable.value.scope.2.0") == NULL);
                GtkWidget *release = Find(root, "studio.debug.variable.collapse.scope.1.0"); CHECK(GTK_IS_BUTTON(release));
                g_signal_emit_by_name(release, "clicked"); g_signal_emit_by_name(other, "clicked");
                CHECK(Find(root, "studio.debug.variable.value.scope.2.0") != NULL);
            } else if (strcmp(name, "same-name") == 0) {
                GtkWidget *other = Find(root, "studio.debug.variable.inspect.scope.2"); CHECK(GTK_IS_BUTTON(other));
                g_signal_emit_by_name(other, "clicked"); GtkWidget *otherValue = Find(root, "studio.debug.variable.value.scope.2.0");
                CHECK(GTK_IS_LABEL(otherValue) && strstr(gtk_label_get_text(GTK_LABEL(otherValue)), "other = 99") != NULL);
                CHECK(Find(root, "studio.debug.variable.inspect.scope.1.0") == retainedChild);
            } else {
                CHECK(strcmp(name, "expand") == 0 || only);
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(childValue)), "capture 1") != NULL);
                CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
                CHECK(Find(root, only ? "studio.debug.variable.inspect.scope.0.0" : "studio.debug.variable.inspect.scope.1.0") == retainedChild);
            }
        }
    }
cleanup:
    if (retained != NULL) g_object_unref(retained);
    if (retainedChild != NULL) g_object_unref(retainedChild);
    if (platform != NULL) (void)umi_debug_runtime_platform_stop(platform, 1, 2000U);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0) failed = 1;
    if (isolated) {
        if (originalData != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated scope fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(originalDirectory); g_free(originalData);
    return failed;
}
