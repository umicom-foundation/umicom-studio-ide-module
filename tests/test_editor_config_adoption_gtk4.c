/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_config_adoption_gtk4.c
 *
 * PURPOSE:
 *   Verify reviewed project indentation through Studio with captured files and explicit project
 * boundaries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/indentation_override.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/editor_config_adoption.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                      \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)
static GtkWidget *Find(GtkWidget *root, const char *name) {
  const char *tag = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
  if (!strcmp(gtk_widget_get_name(root), name) || (tag && !strcmp(tag, name)))
    return root;
  for (GtkWidget *child = gtk_widget_get_first_child(root); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child, name);
    if (found)
      return found;
  }
  return NULL;
}
static GtkWindow *Form(void) {
  GListModel *windows = gtk_window_get_toplevels();
  for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(window)), "document.editor-config.review") &&
        UmiGtk4WindowIsOpen(window))
      return window;
    g_object_unref(window);
  }
  return NULL;
}
static void Drain(GtkWindow *window) {
  gint64 deadline = g_get_monotonic_time() + 15 * G_TIME_SPAN_SECOND;
  while (UmiGtk4EditorConfigAdoptionBusy(window) && g_get_monotonic_time() < deadline) {
    while (g_main_context_pending(NULL))
      g_main_context_iteration(NULL, FALSE);
    g_usleep(1000);
  }
  CHECK(!UmiGtk4EditorConfigAdoptionBusy(window));
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1],
             *names[] = {"entry",    "apply",           "untrusted", "no-project", "no-document",
                         "untitled", "outside-project", "argument",  "inactive"};
  int known = 0;
  for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i)
    if (!strcmp(name, names[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "studio-adoption-XXXXXX", NULL);
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
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions services = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioConfigAdoptionTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  char *project = g_build_filename(directory, "project", NULL);
  CHECK(g_mkdir(project, 0700) == 0);
  char *config = g_build_filename(project, ".editorconfig", NULL),
       *file =
           g_build_filename(!strcmp(name, "outside-project") ? directory : project, "main.c", NULL);
  CHECK(g_file_set_contents(config, "[*]\nindent_size=2\n", -1, NULL));
  CHECK(g_file_set_contents(file, "ab\nc", -1, NULL));
  if (strcmp(name, "no-project"))
    CHECK(umi_studio_workspace_open(umi_studio_ui_services(ui), project,
                                    strcmp(name, "untrusted") != 0, 0) == UMI_STATUS_OK);
  UmiDocumentId id = 0U;
  if (strcmp(name, "no-document")) {
    char view[UMI_UI_ID_CAPACITY];
    UmiDocumentWorkingCopySnapshot snapshot;
    CHECK((!strcmp(name, "untitled")
               ? umi_document_coordinator_new(documents, "main.c", view, sizeof view)
               : umi_document_coordinator_open(documents, file, view, sizeof view)) ==
          UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &snapshot) == UMI_STATUS_OK);
    id = snapshot.document_id;
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  CHECK(Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)),
             "studio.editor.project-indentation") != NULL);
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "editor.project-indentation", !strcmp(name, "argument") ? "unexpected" : NULL);
  GtkWindow *form = Form();
  int unavailable = !strcmp(name, "no-project") || !strcmp(name, "no-document") ||
                    !strcmp(name, "untitled") || !strcmp(name, "outside-project");
  if (unavailable)
    CHECK(status == UMI_STATUS_PERMISSION_DENIED && !form);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT && !form);
  else {
    CHECK(status == UMI_STATUS_OK && form);
    CHECK(UmiGtk4EditorConfigAdoptionStatus(form) == UMI_STATUS_UNAVAILABLE);
    if (strcmp(name, "entry")) {
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.editor-config.inspect"), "clicked");
      Drain(form);
      CHECK(UmiGtk4EditorConfigAdoptionStatus(form) == UMI_STATUS_OK);
      UmiDocumentId peer = 0U;
      if (!strcmp(name, "inactive")) {
        char view[UMI_UI_ID_CAPACITY];
        UmiDocumentWorkingCopySnapshot snapshot;
        CHECK(umi_document_coordinator_new(documents, "other.c", view, sizeof view) ==
              UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &snapshot) == UMI_STATUS_OK);
        peer = snapshot.document_id;
      }
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.editor-config.apply"), "clicked");
      Drain(form);
      CHECK(UmiGtk4EditorConfigAdoptionStatus(form) == UMI_STATUS_OK);
      UmiDocumentIndentationReview review;
      CHECK(UmiDocumentCoordinatorInspectIndentation(documents, id, &review) == UMI_STATUS_OK &&
            review.overridden && review.effective.tab_width == 2U);
      if (peer)
        CHECK(UmiDocumentCoordinatorInspectIndentation(documents, peer, &review) == UMI_STATUS_OK &&
              !review.overridden);
    }
    gtk_window_destroy(form);
    g_object_unref(form);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_object_unref(app);
  g_free(config);
  g_free(file);
  g_free(project);
  g_free(directory);
  return 0;
}
