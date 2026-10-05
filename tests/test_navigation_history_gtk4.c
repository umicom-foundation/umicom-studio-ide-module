/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_navigation_history_gtk4.c
 * PURPOSE: Exercise Studio history shortcuts and retained-controller teardown.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/document/navigation_history.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)

/* Find the actual Studio shortcut handler; retain it across teardown to check
 * that the existing host cleanup disconnects callbacks borrowing its runtime. */
static GtkEventController *Keyboard(GtkWidget *window)
{
    GListModel *controllers = gtk_widget_observe_controllers(window);
    GtkEventController *found = NULL;
    for (guint index = 0U; index < g_list_model_get_n_items(controllers); ++index) {
        GtkEventController *controller = g_list_model_get_item(controllers, index);
        if (g_strcmp0(gtk_event_controller_get_name(controller), "umicom-studio-keyboard") == 0) {
            found = controller; break;
        }
        g_object_unref(controller);
    }
    g_object_unref(controllers);
    return found;
}

/* The unpresented workbench uses a private temporary configuration. No tool
 * discovery, build process, remote AI or personal session restore is enabled. */
int main(void)
{
    int failed = 0, changed_directory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions services_options = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkEventController *retained = NULL;
    char *directory = NULL, *original_directory = NULL, *original_data = NULL;
    GError *error = NULL;
    char workspace[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY], excluded[UMI_PATH_CAPACITY];
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    original_directory = g_get_current_dir(); original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-navigation-XXXXXX", &error);
    CHECK(original_directory != NULL && directory != NULL && g_chdir(directory) == 0); changed_directory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_path_join(directory, "workspace", workspace, sizeof(workspace)) == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(workspace) == UMI_STATUS_OK);
    CHECK(umi_path_join(workspace, "notes.c", source, sizeof(source)) == UMI_STATUS_OK);
    CHECK(umi_path_join(workspace, "notes.txt", excluded, sizeof(excluded)) == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(source, "first\nsecond\nthird\n") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(excluded, "FindMarker\n") == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    CHECK(umi_studio_workspace_open(umi_studio_bootstrap_services(bootstrap), workspace, 0, 0) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.navigation-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);

    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    CHECK(umi_document_coordinator_open(documents, source, NULL, 0U) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorGoToPosition(documents, 2U, 1U, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    retained = Keyboard(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)));
    CHECK(retained != NULL);
    gboolean handled = FALSE;
    g_signal_emit_by_name(retained, "key-pressed", GDK_KEY_Left, 0U, GDK_ALT_MASK, &handled);
    CHECK(handled);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(umi_studio_ui_workbench(ui)), active.view_id, &view) == UMI_STATUS_OK);
    CHECK(view.cursor_offset == 0U && strcmp(view.source_text, "first\nsecond\nthird\n") == 0);
    handled = FALSE;
    g_signal_emit_by_name(retained, "key-pressed", GDK_KEY_Right, 0U, GDK_ALT_MASK, &handled);
    CHECK(handled);
    CHECK(umi_ui_document_view_model_find(umi_ui_workbench_documents(umi_studio_ui_workbench(ui)), active.view_id, &view) == UMI_STATUS_OK);
    CHECK(view.cursor_offset == 6U);
    UmiDocumentNavigationHistorySnapshot history;
    CHECK(UmiDocumentCoordinatorNavigationSnapshot(documents, &history) == UMI_STATUS_OK);
    uint64_t revision = history.revision;
    umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
    handled = FALSE;
    g_signal_emit_by_name(retained, "key-pressed", GDK_KEY_Left, 0U, GDK_ALT_MASK, &handled);
    CHECK(!handled);
    CHECK(UmiDocumentCoordinatorNavigationSnapshot(documents, &history) == UMI_STATUS_OK && history.revision == revision);
cleanup:
    g_clear_object(&retained);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed_directory && g_chdir(original_directory) != 0) failed = 1;
    if (isolated) {
        if (original_data != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated Studio navigation fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(original_directory); g_free(original_data);
    return failed;
}
