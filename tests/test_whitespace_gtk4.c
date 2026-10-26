/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_whitespace_gtk4.c
 *
 * PURPOSE:
 *   Exercise shared whitespace menu actions through real Studio source documents.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/document_whitespace.h"
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
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(window)), "document.whitespace.review") &&
        UmiGtk4WindowIsOpen(window))
      return window;
    g_object_unref(window);
  }
  return NULL;
}
int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  const char *name = argv[1],
             *names[] = {"trim-document",    "trim-lines",    "expand-document", "expand-lines",
                         "compact-document", "compact-lines", "inactive",        "readonly",
                         "no-document",      "argument",      "cancel",          "stale"};
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
  char *directory = g_build_filename(argv[2], "whitespace-XXXXXX", NULL);
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
      gtk_application_new("org.umicom.StudioWhitespaceTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);

  const char *source = "a \nb \n", *expected = "a\nb\n", *command = "editor.trim-trailing-document";
  if (!strcmp(name, "trim-lines")) {
    command = "editor.trim-trailing-lines";
    expected = "a\nb \n";
  }
  if (!strcmp(name, "expand-document")) {
    command = "editor.expand-tabs-document";
    source = "\ta\n\tb";
    expected = "    a\n    b";
  }
  if (!strcmp(name, "expand-lines")) {
    command = "editor.expand-tabs-lines";
    source = "\ta\n\tb";
    expected = "    a\n\tb";
  }
  if (!strcmp(name, "compact-document")) {
    command = "editor.compact-spaces-document";
    source = "    a\n    b";
    expected = "\ta\n\tb";
  }
  if (!strcmp(name, "compact-lines")) {
    command = "editor.compact-spaces-lines";
    source = "    a\n    b";
    expected = "\ta\n    b";
  }
  UmiDocumentId id = 0U;
  char view_id[UMI_UI_ID_CAPACITY] = {0};
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  if (strcmp(name, "no-document")) {
    CHECK(umi_document_coordinator_new(documents, "whitespace.c", view_id, sizeof view_id) ==
          UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    id = active.document_id;
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    view.cursor_offset = 0U;
    view.selection_length = 0U;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    if (!strcmp(name, "readonly")) {
      CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
      view.read_only = 1;
      CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
    }
    UmiEditorIndentation policy = {4U, 1};
    CHECK(UmiDocumentCoordinatorSetIndentation(documents, &policy) == UMI_STATUS_OK);
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  CHECK(Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.whitespace"));
  /* Startup includes a real welcome document. A negative admission case must
   * close its fixture-owned documents explicitly rather than assume startup
   * is empty. Check progress on each close so lifecycle defects cannot hang. */
  if (!strcmp(name, "no-document")) {
    size_t remaining = umi_document_coordinator_count(documents);
    while (remaining != 0U) {
      CHECK(umi_document_coordinator_close_active(documents, 1) == UMI_STATUS_OK);
      size_t afterClose = umi_document_coordinator_count(documents);
      CHECK(afterClose < remaining);
      remaining = afterClose;
    }
    UmiDocumentWorkingCopySnapshot absent;
    CHECK(umi_document_coordinator_count(documents) == 0U);
    CHECK(umi_document_coordinator_active_snapshot(documents, &absent) == UMI_STATUS_NOT_FOUND);
  }
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, command, !strcmp(name, "argument") ? "invalid" : NULL);
  GtkWindow *form = Form();
  if (!strcmp(name, "readonly") || !strcmp(name, "no-document"))
    CHECK(status == UMI_STATUS_PERMISSION_DENIED && form == NULL);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT && form == NULL);
  else {
    CHECK(status == UMI_STATUS_OK && form);
    if (!strcmp(name, "inactive")) {
      char other[UMI_UI_ID_CAPACITY];
      CHECK(umi_document_coordinator_new(documents, "other.c", other, sizeof other) ==
            UMI_STATUS_OK);
    }
    if (!strcmp(name, "stale")) {
      UmiUiDocumentViewSnapshot v;
      CHECK(umi_ui_document_view_model_find(views, view_id, &v) == UMI_STATUS_OK);
      CHECK(UmiUiDocumentViewModelUpsertText(views, &v, "changed", 7U) == UMI_STATUS_OK);
    }
    if (!strcmp(name, "cancel"))
      gtk_window_destroy(form);
    else
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.whitespace.apply"), "clicked");
    int applied = 0;
    status = UmiGtk4WhitespaceReviewStatus(form, &applied);
    if (!strcmp(name, "stale"))
      CHECK(status == UMI_STATUS_INVALID_STATE && !applied);
    else if (!strcmp(name, "cancel"))
      CHECK(!applied);
    else
      CHECK(applied);
    char *text = NULL;
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &bytes) == UMI_STATUS_OK);
    const char *wanted = !strcmp(name, "stale")    ? "changed"
                         : !strcmp(name, "cancel") ? source
                                                   : expected;
    CHECK(bytes == strlen(wanted) && !strcmp(text, wanted));
    UmiUiDocumentViewModelFreeText(text);
    if (applied)
      CHECK(UmiDocumentCoordinatorUndo(documents, id) == UMI_STATUS_OK);
    if (UmiGtk4WindowIsOpen(form))
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
