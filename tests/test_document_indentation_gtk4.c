/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_document_indentation_gtk4.c
 *
 * PURPOSE:
 *   Exercise document-specific indentation through real Studio routing while preserving session
 * defaults.
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
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                      \
      return 1;                                                                                    \
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
  for (guint i = 0; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(window)), "document.indentation.review") &&
        UmiGtk4WindowIsOpen(window))
      return window;
    g_object_unref(window);
  }
  return NULL;
}
int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  const char *name = argv[1], *names[] = {"entry",    "apply",       "inherit",  "inactive",
                                          "argument", "no-document", "readonly", "session-default"};
  int known = 0;
  for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i)
    if (!strcmp(name, names[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  /* The former fixed case directory is retained for review. A fresh child now
   * isolates persistent bootstrap data across repeated qualification runs. */
#if 0
  CHECK(g_chdir(argv[2]) == 0);
#endif
  char *directory = g_build_filename(argv[2], "document-indentation-XXXXXX", NULL);
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
      gtk_application_new("org.umicom.StudioDocumentIndentationTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  UmiDocumentId id = 0U;
  char view[UMI_UI_ID_CAPACITY];
  UmiDocumentIndentationReview review;
  UmiEditorIndentation before;
  CHECK(UmiDocumentCoordinatorGetIndentation(documents, &before) == UMI_STATUS_OK);
  if (strcmp(name, "no-document")) {
    CHECK(umi_document_coordinator_new(documents, "review.c", view, sizeof view) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot snapshot;
    CHECK(umi_document_coordinator_active_snapshot(documents, &snapshot) == UMI_STATUS_OK);
    id = snapshot.document_id;
    if (!strcmp(name, "inherit")) {
      UmiEditorIndentation initial = {8U, 0};
      CHECK(UmiDocumentCoordinatorInspectIndentation(documents, id, &review) == UMI_STATUS_OK);
      CHECK(UmiDocumentCoordinatorApplyIndentation(documents, &review, &initial, NULL) ==
            UMI_STATUS_OK);
    }
    if (!strcmp(name, "readonly")) {
      UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
      UmiUiDocumentViewSnapshot snapshot;
      CHECK(umi_ui_document_view_model_find(views, view, &snapshot) == UMI_STATUS_OK);
      snapshot.read_only = 1;
      CHECK(umi_ui_document_view_model_upsert(views, &snapshot) == UMI_STATUS_OK);
    }
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  CHECK(Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)),
             "studio.editor.document-indentation") != NULL);
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "editor.document-indentation", !strcmp(name, "argument") ? "unexpected" : NULL);
  GtkWindow *form = Form();
  if (!strcmp(name, "no-document"))
    CHECK(status == UMI_STATUS_PERMISSION_DENIED && form == NULL);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT && form == NULL);
  else {
    CHECK(status == UMI_STATUS_OK && form != NULL);
    if (strcmp(name, "entry")) {
      UmiDocumentId peer = 0U;
      if (!strcmp(name, "inactive")) {
        char other[UMI_UI_ID_CAPACITY];
        UmiDocumentWorkingCopySnapshot snapshot;
        CHECK(umi_document_coordinator_new(documents, "other.c", other, sizeof other) ==
              UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &snapshot) == UMI_STATUS_OK);
        peer = snapshot.document_id;
      }
      GtkWidget *width = Find(GTK_WIDGET(form), "document.indentation.width"),
                *spaces = Find(GTK_WIDGET(form), "document.indentation.spaces");
      CHECK(width && spaces);
      gtk_editable_set_text(GTK_EDITABLE(width), "2");
      gtk_check_button_set_active(GTK_CHECK_BUTTON(spaces), TRUE);
      GtkWidget *apply =
          Find(GTK_WIDGET(form), !strcmp(name, "inherit") ? "document.indentation.inherit"
                                                          : "document.indentation.apply");
      CHECK(apply);
      g_signal_emit_by_name(apply, "clicked");
      CHECK(UmiDocumentCoordinatorInspectIndentation(documents, id, &review) == UMI_STATUS_OK);
      CHECK(review.overridden == !!strcmp(name, "inherit"));
      CHECK(review.effective.tab_width == (!strcmp(name, "inherit") ? before.tab_width : 2U));
      if (peer) {
        CHECK(UmiDocumentCoordinatorInspectIndentation(documents, peer, &review) == UMI_STATUS_OK &&
              !review.overridden);
      }
      UmiEditorIndentation after;
      CHECK(UmiDocumentCoordinatorGetIndentation(documents, &after) == UMI_STATUS_OK &&
            after.tab_width == before.tab_width && after.insert_spaces == before.insert_spaces);
    }
    gtk_window_destroy(form);
    g_object_unref(form);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_object_unref(app);
  g_free(directory);
  return 0;
}
