/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_language_comment_gtk4.c
 *
 * PURPOSE:
 *   Verify language-aware source comments through Studio controls, shared dispatch and Undo.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/commands.h"
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

int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  const char *name = argv[1],
             *names[] = {"c", "python", "sql", "cmake", "readonly", "unknown", "argument", "undo"};
  int known = 0;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "comments-XXXXXX", NULL);
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
      gtk_application_new("org.umicom.StudioCommentTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);

  const char *filename = !strcmp(name, "python")    ? "review.py"
                         : !strcmp(name, "sql")     ? "review.sql"
                         : !strcmp(name, "cmake")   ? "CMakeLists.txt"
                         : !strcmp(name, "unknown") ? "review.json"
                                                    : "review.c";
  const char *source = "alpha\nbeta", *expected = !strcmp(name, "python") || !strcmp(name, "cmake")
                                                      ? "# alpha\nbeta"
                                                  : !strcmp(name, "sql") ? "-- alpha\nbeta"
                                                                         : "// alpha\nbeta";
  char view_id[UMI_UI_ID_CAPACITY];
  CHECK(umi_document_coordinator_new(documents, filename, view_id, sizeof view_id) ==
        UMI_STATUS_OK);
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  UmiUiDocumentViewSnapshot view;
  CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
  view.dirty = 1;
  view.cursor_offset = 0U;
  view.selection_length = 0U;
  CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
  if (!strcmp(name, "readonly")) {
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.read_only = 1;
    CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
  }
  /* This check observes native controls. Preserve the projection-only
   * refresh for review; explicit workspace synchronization publishes the
   * search pane and command availability without waiting for a timer. */
#if 0
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
#endif
  CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
  GtkWidget *button = Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)),
                           "studio.editor.toggle-language-comment");
  CHECK(GTK_IS_BUTTON(button));
  int refused = !strcmp(name, "readonly") || !strcmp(name, "unknown");
  CHECK(gtk_widget_get_sensitive(button) == !refused);
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "editor.toggle-language-comment", !strcmp(name, "argument") ? "bad" : NULL);
  CHECK(status == (refused                     ? UMI_STATUS_PERMISSION_DENIED
                   : !strcmp(name, "argument") ? UMI_STATUS_INVALID_ARGUMENT
                                               : UMI_STATUS_OK));
  char *text = NULL;
  size_t bytes = 0U;
  CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &bytes) == UMI_STATUS_OK);
  CHECK(!strcmp(text, status == UMI_STATUS_OK ? expected : source));
  UmiUiDocumentViewModelFreeText(text);
  if (!strcmp(name, "undo")) {
    CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
    text = NULL;
    CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &bytes) == UMI_STATUS_OK);
    CHECK(!strcmp(text, source));
    UmiUiDocumentViewModelFreeText(text);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(directory);
  return 0;
}
