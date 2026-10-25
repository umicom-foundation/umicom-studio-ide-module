/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/search_pages_fixture.h
 *
 * PURPOSE:
 *   Create isolated real Studio search workflows below the test working directory.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_SEARCH_PAGES_FIXTURE_H
#define UMICOM_STUDIO_SEARCH_PAGES_FIXTURE_H
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
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
typedef struct SearchPagesFixture {
  GtkApplication *app;
  UmiStudioBootstrap *bootstrap;
  UmiStudioGtkWorkbench *workbench;
  UmiStudioUi *ui;
  GtkWidget *root;
  char *original_directory, *original_data, *directory;
  char workspace[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY];
  unsigned callbacks;
  const char *action;
} SearchPagesFixture;
static inline GtkWidget *SearchFind(GtkWidget *root, const char *tag) {
  if (root == NULL)
    return NULL;
  const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
  if (actual != NULL && strcmp(actual, tag) == 0)
    return root;
  for (GtkWidget *child = gtk_widget_get_first_child(root); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = SearchFind(child, tag);
    if (found)
      return found;
  }
  return NULL;
}
static inline GtkWidget *SearchControl(SearchPagesFixture *f, const char *part) {
  char tag[100];
  (void)snprintf(tag, sizeof tag, "umicom-file-search-%s", part);
  GtkWidget *control = SearchFind(f->root, tag);
  CHECK(control != NULL);
  return control;
}
static inline void SearchClick(SearchPagesFixture *f, const char *part) {
  g_signal_emit_by_name(SearchControl(f, part), "clicked");
}
static inline void SearchWait(SearchPagesFixture *f) {
  UmiFileSearchSnapshot state;
  for (unsigned i = 0U; i < 10000U; ++i) {
    CHECK(UmiStudioUiSearchRead(f->ui, &state) == UMI_STATUS_OK);
    if (!state.active) {
      CHECK(state.ready && !state.stale);
      return;
    }
    g_usleep(1000UL);
  }
  CHECK(0);
}
static inline void SearchPagesCreate(SearchPagesFixture *f) {
  memset(f, 0, sizeof *f);
  f->original_directory = g_get_current_dir();
  f->original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
  /* Keep fixture evidence below CTest's working directory, never in a user
   * profile. A fresh directory also isolates parallel test processes. */
  for (unsigned i = 0U; i < 1000U; ++i) {
    f->directory = g_strdup_printf("%s/search-page-%" G_GINT64_FORMAT "-%u", f->original_directory,
                                   g_get_monotonic_time(), i);
    if (g_mkdir(f->directory, 0700) == 0)
      break;
    g_clear_pointer(&f->directory, g_free);
  }
  CHECK(f->directory != NULL && g_chdir(f->directory) == 0);
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
  CHECK(umi_path_join(f->directory, "workspace", f->workspace, sizeof f->workspace) ==
        UMI_STATUS_OK);
  CHECK(umi_fs_make_directories(f->workspace) == UMI_STATUS_OK);
  CHECK(umi_path_join(f->workspace, "many.c", f->source, sizeof f->source) == UMI_STATUS_OK);
  GString *text = g_string_new(NULL);
  for (unsigned i = 0U; i < 150U; ++i)
    g_string_append(text, "PageMarker\n");
  CHECK(umi_fs_write_text(f->source, text->str) == UMI_STATUS_OK);
  g_string_free(text, TRUE);
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &f->bootstrap) == UMI_STATUS_OK);
  f->ui = umi_studio_bootstrap_ui(f->bootstrap);
  CHECK(umi_studio_workspace_open(umi_studio_bootstrap_services(f->bootstrap), f->workspace, 0,
                                  0) == UMI_STATUS_OK);
  f->app = gtk_application_new("org.umicom.studio.search-pages-test", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(f->app), NULL, NULL));
  UmiStudioGtkWorkbenchOptions native = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(
            f->app, f->ui, umi_studio_bootstrap_desktop_shell(f->bootstrap), &native,
            &f->workbench) == UMI_STATUS_OK);
  f->root = GTK_WIDGET(umi_studio_gtk_workbench_window(f->workbench));
  CHECK(umi_studio_gtk_workbench_workspace_open_surface(f->workbench, UMI_STUDIO_SURFACE_SEARCH,
                                                        NULL) == UMI_STATUS_OK);
  CHECK(umi_studio_gtk_workbench_refresh(f->workbench) == UMI_STATUS_OK);
  gtk_editable_set_text(GTK_EDITABLE(SearchControl(f, "query")), "PageMarker");
  SearchClick(f, "start");
  SearchWait(f);
  CHECK(umi_studio_gtk_workbench_refresh(f->workbench) == UMI_STATUS_OK);
}
static inline size_t SearchRowCount(SearchPagesFixture *f) {
  size_t count = 0U;
  for (GtkWidget *row = gtk_widget_get_first_child(SearchControl(f, "results")); row;
       row = gtk_widget_get_next_sibling(row))
    ++count;
  return count;
}
static inline GtkWidget *SearchFirstButton(SearchPagesFixture *f) {
  GtkWidget *row = gtk_widget_get_first_child(SearchControl(f, "results"));
  CHECK(GTK_IS_LIST_BOX_ROW(row));
  GtkWidget *button = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(row));
  CHECK(GTK_IS_BUTTON(button));
  return button;
}
static inline void SearchPagesDestroy(SearchPagesFixture *f) {
  umi_studio_gtk_workbench_destroy(f->workbench);
  umi_studio_bootstrap_destroy(f->bootstrap);
  g_clear_object(&f->app);
  CHECK(g_chdir(f->original_directory) == 0);
  if (f->original_data)
    CHECK(g_setenv("UMICOM_STUDIO_DATA_PATH", f->original_data, TRUE));
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  printf("Retained Studio search fixture: %s\n", f->directory);
  g_free(f->directory);
  g_free(f->original_data);
  g_free(f->original_directory);
}
#endif
