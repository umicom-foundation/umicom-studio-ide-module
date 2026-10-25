/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_kanban_entry_gtk4.c
 * PURPOSE: Verify the welcome entry opens an independently owned Framework board window.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/kanban_project.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)
static GtkWidget *Find(GtkWidget *root)
{
    if (strcmp(gtk_widget_get_name(root), "umicom-kanban-launcher") == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = Find(child); if (found != NULL) return found;
    }
    return NULL;
}
static GtkWindow *FindBoard(GtkApplication *application)
{
    for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next)
        if (strcmp(gtk_widget_get_name(GTK_WIDGET(item->data)), "umicom-kanban-window") == 0) return GTK_WINDOW(item->data);
    return NULL;
}
int main(int argc, char **argv)
{
    REQUIRE(argc == 3);
    if (!gtk_init_check()) { fprintf(stderr, "GTK display unavailable; Studio board entry coverage skipped.\n"); return 77; }
    char *directory = g_build_filename(argv[2], "studio-kanban-XXXXXX", NULL);
    REQUIRE(g_mkdtemp(directory) != NULL && g_chdir(directory) == 0);
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); REQUIRE(g_mkdir_with_parents("config", 0700) == 0);
    UmiSettings *settings = NULL;
    REQUIRE(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    REQUIRE(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioBootstrap *bootstrap = NULL; UmiStudioServicesOptions services = {0};
    REQUIRE(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
    GtkApplication *application = gtk_application_new("org.umicom.StudioKanbanTest", G_APPLICATION_NON_UNIQUE);
    GError *error = NULL; REQUIRE(g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbench *workbench = NULL; UmiStudioGtkWorkbenchOptions options = {0};
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    GtkWidget *button = Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench))); REQUIRE(button != NULL); g_object_ref(button);
    GtkWindow *board = NULL;

    if (strncmp(argv[1], "project", 7U) == 0) {
        UmiStudioServices *project_services = umi_studio_ui_services(umi_studio_bootstrap_ui(bootstrap));
        char *project = g_build_filename(directory, "project", NULL);
        char *other = g_build_filename(directory, "other", NULL);
        REQUIRE(g_mkdir_with_parents(project, 0700) == 0 && g_mkdir_with_parents(other, 0700) == 0);
        if (strcmp(argv[1], "project-none") != 0)
            REQUIRE(umi_studio_workspace_open(project_services, project, strcmp(argv[1], "project-untrusted") != 0, 0) == UMI_STATUS_OK);
        REQUIRE(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        UmiStatus status = umi_studio_gtk_workbench_execute_command(workbench, "project.board",
            strcmp(argv[1], "project-argument") == 0 ? "unexpected" : NULL);
        if (!strcmp(argv[1], "project-none") || !strcmp(argv[1], "project-argument")) {
            REQUIRE(status == (!strcmp(argv[1], "project-none") ? UMI_STATUS_PERMISSION_DENIED : UMI_STATUS_INVALID_ARGUMENT));
            REQUIRE(FindBoard(application) == NULL);
        } else {
            REQUIRE(status == UMI_STATUS_OK);
            board = FindBoard(application); REQUIRE(board != NULL); g_object_ref(board);
            UmiKanbanProjectLocation captured;
            REQUIRE(UmiKanbanGtk4ProjectLocation(board, &captured) == UMI_STATUS_OK &&
                umi_path_equal(captured.project_root, project));
            if (!strcmp(argv[1], "project-reuse")) {
                REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "project.board", NULL) == UMI_STATUS_OK);
                size_t count = 0U;
                for (GList *item = gtk_application_get_windows(application); item; item = item->next)
                    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(item->data)), "umicom-kanban-window")) ++count;
                REQUIRE(count == 1U && FindBoard(application) == board);
            }
            if (!strcmp(argv[1], "project-switch")) {
                REQUIRE(umi_studio_workspace_close(project_services) == UMI_STATUS_OK);
                REQUIRE(umi_studio_workspace_open(project_services, other, 1, 0) == UMI_STATUS_OK);
                REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "project.board", NULL) == UMI_STATUS_OK);
                UmiKanbanProjectLocation unchanged;
                REQUIRE(UmiKanbanGtk4ProjectLocation(board, &unchanged) == UMI_STATUS_OK &&
                    !strcmp(unchanged.project_root, captured.project_root));
                GtkWindow *second = NULL;
                for (GList *item = gtk_application_get_windows(application); item; item = item->next) {
                    UmiKanbanProjectLocation location;
                    if (UmiKanbanGtk4ProjectLocation(GTK_WINDOW(item->data), &location) == UMI_STATUS_OK &&
                        umi_path_equal(location.project_root, other)) second = GTK_WINDOW(item->data);
                }
                REQUIRE(second != NULL && second != board); gtk_window_close(second);
            }
            if (!strcmp(argv[1], "project-independent")) {
                umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
                REQUIRE(gtk_window_get_application(board) == application);
                REQUIRE(UmiKanbanGtk4ProjectLocation(board, &captured) == UMI_STATUS_OK);
            }
            gtk_window_close(board);
        }
        if (workbench != NULL) { umi_studio_gtk_workbench_destroy(workbench); workbench = NULL; }
        g_free(project); g_free(other);
    } else
    if (strcmp(argv[1], "entry") == 0) {
        g_signal_emit_by_name(button, "clicked"); board = FindBoard(application); REQUIRE(board != NULL); g_object_ref(board);
        REQUIRE(gtk_widget_get_visible(GTK_WIDGET(board)));
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        REQUIRE(gtk_window_get_application(board) == application);
        REQUIRE(gtk_widget_get_visible(GTK_WIDGET(board)));
        gtk_window_close(board);
    } else {
        REQUIRE(strcmp(argv[1], "retained") == 0);
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(button, "clicked"); REQUIRE(FindBoard(application) == NULL);
    }
    g_object_unref(button); g_clear_object(&board);
    umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings); g_object_unref(application);
    /* Keep the isolated fixture folder for review; no product files are touched. */
    g_free(directory); return 0;
}
