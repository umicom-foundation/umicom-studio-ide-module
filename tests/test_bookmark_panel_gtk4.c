/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_bookmark_panel_gtk4.c
 *
 * PURPOSE:
 *   Exercise the shared bookmark pane through Studio and reveal a hidden editor without losing
 * drafts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/document.h"
#include "umicom/document/navigation.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/bookmark_panel.h"
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
static GtkWidget *Find(GtkWidget *root, const char *tag) {
  const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
  if (actual && !strcmp(actual, tag))
    return root;
  for (GtkWidget *child = gtk_widget_get_first_child(root); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child, tag);
    if (found)
      return found;
  }
  return NULL;
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *name = argv[1], *cases[] = {"visit",    "hidden-editor", "remove",         "closed",
                                          "retained", "refresh",       "catalogue-bound"};
  int known = 0;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(name, cases[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *original = g_get_current_dir(), *data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
  char *directory = g_build_filename(original, "bookmark-pane-XXXXXX", NULL);
  CHECK(g_mkdtemp(directory) != NULL && g_chdir(directory) == 0);
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
  umi_settings_destroy(settings);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions service = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&service, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  char id[UMI_UI_ID_CAPACITY];
  CHECK(umi_document_coordinator_new(documents, "notes.c", id, sizeof id) == UMI_STATUS_OK);
  UmiUiDocumentViewSnapshot view;
  CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
  view.dirty = 1;
  CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "first\nsecond\nthird\n", 19U) ==
        UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorGoToPosition(documents, 2U, 2U, NULL) == UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorToggleBookmark(documents, NULL) == UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorGoToPosition(documents, 1U, 1U, NULL) == UMI_STATUS_OK);
  GtkApplication *app =
      gtk_application_new("org.umicom.studio.BookmarkPaneTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  if (!strcmp(name, "hidden-editor")) {
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(
              workbench, "umicom.studio.layout.bookmark-only", "Bookmarks") == UMI_STATUS_OK);
    /* Commit after adding the bookmark pane so the blank canvas has a real tool. */
  }
  CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_BOOKMARKS,
                                                        NULL) == UMI_STATUS_OK);
  if (!strcmp(name, "hidden-editor"))
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
  GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
  GtkWidget *refresh = Find(root, "umicom-bookmark-refresh");
  CHECK(GTK_IS_BUTTON(refresh));
  g_signal_emit_by_name(refresh, "clicked");
  GtkWidget *button =
      Find(root, !strcmp(name, "remove") ? "umicom-bookmark-remove" : "umicom-bookmark-visit");
  CHECK(GTK_IS_BUTTON(button));
  g_object_ref(button);
  if (!strcmp(name, "retained")) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    g_signal_emit_by_name(button, "clicked");
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
          view.cursor_offset == 0U);
  } else if (!strcmp(name, "closed")) {
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorClose(documents, active.document_id, 1) == UMI_STATUS_OK);
    g_signal_emit_by_name(refresh, "clicked");
    GtkWidget *closed = Find(root, "umicom-bookmark-visit");
    CHECK(closed && !gtk_widget_get_sensitive(closed));
    g_signal_emit_by_name(Find(root, "umicom-bookmark-remove"), "clicked");
    UmiDocumentBookmarksSnapshot marks;
    CHECK(UmiDocumentCoordinatorBookmarks(documents, &marks) == UMI_STATUS_OK && !marks.count);
  } else if (!strcmp(name, "refresh")) {
    CHECK(UmiDocumentCoordinatorGoToPosition(documents, 3U, 1U, NULL) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorToggleBookmark(documents, NULL) == UMI_STATUS_OK);
    g_signal_emit_by_name(refresh, "clicked");
    GtkWidget *list = Find(root, "umicom-bookmark-list");
    CHECK(gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 1) != NULL);
    g_signal_emit_by_name(button, "clicked");
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
          view.cursor_offset == 13U);
  } else {
    g_signal_emit_by_name(button, "clicked");
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && !strcmp(view.source_text, "first\nsecond\nthird\n"));
    CHECK(view.cursor_offset == (!strcmp(name, "remove") ? 0U : 7U));
    UmiDocumentBookmarksSnapshot marks;
    CHECK(UmiDocumentCoordinatorBookmarks(documents, &marks) == UMI_STATUS_OK);
    if (!strcmp(name, "remove"))
      CHECK(!marks.count);
    else {
      UmiUiWorkspaceLayout *layout = calloc(1, sizeof *layout);
      CHECK(layout);
      CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
      const UmiUiWorkspaceWindow *editor = umi_ui_workspace_layout_find_window(layout, "editor");
      CHECK(editor && editor->visible);
      free(layout);
    }
    if (!strcmp(name, "catalogue-bound")) {
      /* Both new enum identities traverse storage formerly sized for 27.
       * Sanitizer-enabled owner builds expose any stale bound immediately. */
      CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_KANBAN,
                                                            NULL) == UMI_STATUS_OK);
      CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_BOOKMARKS,
                                                            NULL) == UMI_STATUS_OK);
    }
  }
  g_object_unref(button);
  umi_studio_gtk_workbench_destroy(workbench);
  umi_studio_bootstrap_destroy(bootstrap);
  g_object_unref(app);
  CHECK(g_chdir(original) == 0);
  if (data)
    CHECK(g_setenv("UMICOM_STUDIO_DATA_PATH", data, TRUE));
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  printf("Retained Studio bookmark fixture: %s\n", directory);
  g_free(directory);
  g_free(original);
  g_free(data);
  return 0;
}
