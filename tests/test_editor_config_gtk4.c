/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_config_gtk4.c
 *
 * PURPOSE:
 *   Exercise captured project configuration navigation without implicit file reads.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/path.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/editor_config_inspection.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%d: %s\n", __LINE__, #x);                                                   \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)
static GtkWidget *Find(GtkWidget *widget, const char *name) {
  if (!strcmp(gtk_widget_get_name(widget), name))
    return widget;
  for (GtkWidget *child = gtk_widget_get_first_child(widget); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child, name);
    if (found)
      return found;
  }
  return NULL;
}
static GtkWindow *Inspector(GtkApplication *app) {
  for (GList *item = gtk_application_get_windows(app); item; item = item->next)
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(item->data)), "umicom-editor-config-inspection"))
      return GTK_WINDOW(item->data);
  return NULL;
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1];
  const char *names[] = {"entry",       "untrusted",   "no-project",    "argument",
                         "active-file", "independent", "switch-project"};
  int known = 0;
  for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i)
    if (!strcmp(name, names[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "studio-config-XXXXXX", NULL);
  CHECK(g_mkdtemp(directory) && g_chdir(directory) == 0);
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  UmiSettings *settings = NULL;
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions services = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioConfigTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, umi_studio_bootstrap_ui(bootstrap),
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  char *project = g_build_filename(directory, "project", NULL),
       *other = g_build_filename(directory, "other", NULL);
  CHECK(g_mkdir(project, 0700) == 0 && g_mkdir(other, 0700) == 0);
  UmiStudioServices *owner = umi_studio_ui_services(umi_studio_bootstrap_ui(bootstrap));
  if (strcmp(name, "no-project"))
    CHECK(umi_studio_workspace_open(owner, project, strcmp(name, "untrusted") != 0, 0) ==
          UMI_STATUS_OK);
  if (!strcmp(name, "active-file")) {
    char *file = g_build_filename(project, "main.c", NULL);
    CHECK(g_file_set_contents(file, "int main(void) { return 0; }\n", -1, NULL));
    char view[UMI_UI_ID_CAPACITY];
    CHECK(umi_document_coordinator_open(umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap)),
                                        file, view, sizeof view) == UMI_STATUS_OK);
    g_free(file);
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "project.editor-config", !strcmp(name, "argument") ? "unexpected" : NULL);
  GtkWindow *inspection = Inspector(app);
  if (!strcmp(name, "no-project"))
    CHECK(status == UMI_STATUS_PERMISSION_DENIED && !inspection);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT && !inspection);
  else {
    CHECK(status == UMI_STATUS_OK && inspection);
    g_object_ref(inspection);
    CHECK(UmiGtk4EditorConfigInspectionStatus(inspection) == UMI_STATUS_UNAVAILABLE &&
          !UmiGtk4EditorConfigInspectionBusy(inspection));
    GtkWidget *root = Find(GTK_WIDGET(inspection), "editor-config.root"),
              *relative = Find(GTK_WIDGET(inspection), "editor-config.relative");
    CHECK(root && relative);
    CHECK(umi_path_equal(gtk_editable_get_text(GTK_EDITABLE(root)), project));
    CHECK(!strcmp(gtk_editable_get_text(GTK_EDITABLE(relative)),
                  !strcmp(name, "active-file") ? "main.c" : ""));
    if (!strcmp(name, "independent")) {
      umi_studio_gtk_workbench_destroy(workbench);
      workbench = NULL;
      CHECK(gtk_window_get_application(inspection) == app);
    }
    if (!strcmp(name, "switch-project")) {
      CHECK(umi_studio_workspace_open(owner, other, 1, 0) == UMI_STATUS_OK);
      CHECK(umi_path_equal(gtk_editable_get_text(GTK_EDITABLE(root)), project));
    }
    gtk_window_destroy(inspection);
    g_object_unref(inspection);
  }
  if (workbench)
    umi_studio_gtk_workbench_destroy(workbench);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_object_unref(app);
  g_free(project);
  g_free(other);
  g_free(directory);
  return 0;
}
