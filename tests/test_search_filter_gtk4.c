/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_search_filter_gtk4.c
 * PURPOSE: Exercise real Studio filter controls and captured background search scope.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)

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
    GtkWidget *retained = NULL;
    char *directory = NULL, *original_directory = NULL, *original_data = NULL;
    GError *error = NULL;
    char workspace[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY], excluded[UMI_PATH_CAPACITY];
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    original_directory = g_get_current_dir(); original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-search-scope-XXXXXX", &error);
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
    CHECK(umi_fs_write_text(source, "FindMarker\n") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(excluded, "FindMarker\n") == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    CHECK(umi_studio_workspace_open(umi_studio_bootstrap_services(bootstrap), workspace, 0, 0) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.search-scope-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_SEARCH, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *query = Find(root, "umicom-file-search-query");
    GtkWidget *include = Find(root, "umicom-file-search-include");
    GtkWidget *exclude = Find(root, "umicom-file-search-exclude");
    GtkWidget *start = Find(root, "umicom-file-search-start");
    CHECK(GTK_IS_EDITABLE(query) && GTK_IS_EDITABLE(include) && GTK_IS_EDITABLE(exclude) && GTK_IS_BUTTON(start));
    gtk_editable_set_text(GTK_EDITABLE(query), "FindMarker");
    gtk_editable_set_text(GTK_EDITABLE(include), "*.c;*.txt");
    gtk_editable_set_text(GTK_EDITABLE(exclude), "*.txt");
    g_signal_emit_by_name(start, "clicked");
    /* Editing the next request must not change the already captured scope. */
    gtk_editable_set_text(GTK_EDITABLE(include), "*.md");
    UmiFileSearchSnapshot state;
    for (unsigned attempt = 0U; attempt < 10000U; ++attempt) {
        CHECK(UmiStudioUiSearchRead(ui, &state) == UMI_STATUS_OK);
        if (!state.active) break;
        g_usleep(1000UL);
    }
    CHECK(state.ready && state.stats.matches == 1U);
    UmiSearchPathFilter captured;
    CHECK(UmiStudioUiSearchFilterRead(ui, state.requestId, &captured) == UMI_STATUS_OK);
    CHECK(strcmp(captured.include_patterns, "*.c;*.txt") == 0 && strcmp(captured.exclude_patterns, "*.txt") == 0);
    UmiSearchMatch match;
    CHECK(UmiStudioUiSearchMatchAt(ui, state.requestId, 0U, &match) == UMI_STATUS_OK);
    CHECK(umi_path_equal(match.path, source));
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *scope = Find(root, "umicom-file-search-scope");
    CHECK(GTK_IS_LABEL(scope) && strstr(gtk_label_get_text(GTK_LABEL(scope)), "*.c;*.txt") != NULL);
    include = Find(root, "umicom-file-search-include"); start = Find(root, "umicom-file-search-start");
    CHECK(GTK_IS_EDITABLE(include) && GTK_IS_BUTTON(start));
    retained = g_object_ref(start);
    uint64_t previous_request = state.requestId;
    gtk_editable_set_text(GTK_EDITABLE(include), "[unsupported]");
    g_signal_emit_by_name(start, "clicked");
    CHECK(UmiStudioUiSearchRead(ui, &state) == UMI_STATUS_OK && state.ready && state.requestId == previous_request);
    /* Retaining a native button does not retain permission to call a torn-down
     * product host. The Framework search service remains alive for this check. */
    umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
    g_signal_emit_by_name(retained, "clicked");
    CHECK(UmiStudioUiSearchRead(ui, &state) == UMI_STATUS_OK && state.requestId == previous_request);
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
    if (directory != NULL) printf("Retained isolated Studio search fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(original_directory); g_free(original_data);
    return failed;
}
