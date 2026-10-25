/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_preference_storage_gtk4.c
 *
 * PURPOSE:
 *   Verify asynchronous preference persistence with isolated paths, stale settings and native owner
 * retirement.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/indentation_storage.h"
#include "umicom/platform/local_replace.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/editor_preferences.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%d: %s\n", __LINE__, #x);                                                   \
      failed = 1;                                                                                  \
      goto cleanup;                                                                                \
    }                                                                                              \
  } while (0)
static char preferenceRoot[UMI_PATH_CAPACITY];
/* The native test target redirects only default path selection; it still
 * executes the real codec, file operations, GTK workers and completion path. */
UmiStatus FixtureEditorPreferenceStoragePath(const char *application, const char *base, char *out,
                                             size_t capacity) {
  (void)application;
  (void)base;
  return umi_path_join(preferenceRoot, "preferences.txt", out, capacity);
}
static GtkWidget *Find(GtkWidget *widget, const char *id) {
  if (widget == NULL)
    return NULL;
  const char *tag = g_object_get_data(G_OBJECT(widget), "umicom-automation-id");
  if (tag != NULL && !strcmp(tag, id))
    return widget;
  for (GtkWidget *child = gtk_widget_get_first_child(widget); child != NULL;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child, id);
    if (found != NULL)
      return found;
  }
  return NULL;
}
static GtkWindow *Form(void) {
  GListModel *windows = gtk_window_get_toplevels();
  for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (Find(GTK_WIDGET(window), "studio.editor-preferences"))
      return window;
    g_object_unref(window);
  }
  return NULL;
}
static int Wait(GObject *binding) {
  gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
  while (g_object_get_data(binding, "studio-editor-preference-io") != NULL &&
         g_get_monotonic_time() < deadline) {
    (void)g_main_context_iteration(NULL, FALSE);
    g_usleep(1000U);
  }
  return g_object_get_data(binding, "studio-editor-preference-io") == NULL;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *name = argv[1], *cases[] = {"load",
                                          "save",
                                          "save-captured",
                                          "stale-load",
                                          "busy",
                                          "missing",
                                          "malformed",
                                          "retired",
                                          "default-policy",
                                          "startup",
                                          "button-save",
                                          "draft-not-applied",
                                          "close-after-save",
                                          "relative"};
  int known = 0;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(name, cases[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  int failed = 0, changed = 0;
  GError *error = NULL;
  char *cwd = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH")),
       *directory = NULL, *path = NULL;
  UmiSettings *settings = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioGtkWorkbench *workbench = NULL;
  GtkApplication *application = NULL;
  GtkWindow *form = NULL;
  GObject *binding = NULL;
  directory = g_dir_make_tmp("umicom-editor-storage-XXXXXX", &error);
  CHECK(directory != NULL && g_chdir(directory) == 0);
  changed = 1;
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_strlcpy(preferenceRoot, directory, sizeof preferenceRoot) < sizeof preferenceRoot);
  path = g_build_filename(directory, "preferences.txt", NULL);
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiEditorIndentation stored = {2, 0}, reviewed, current;
  if (strcmp(name, "missing") && strcmp(name, "button-save") && strcmp(name, "draft-not-applied") &&
      strcmp(name, "close-after-save"))
    CHECK(UmiDocumentIndentationStorageSave(path, &stored) == UMI_STATUS_OK);
  if (!strcmp(name, "malformed"))
    CHECK(UmiLocalFileReplace(path, "bad", 3U) == UMI_STATUS_OK);
  UmiStudioServicesOptions services = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  application =
      gtk_application_new("org.umicom.studio.preference-storage-test", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioGtkWorkbenchOptions options = {0};
  options.load_editor_preferences = !strcmp(name, "startup");
  CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
  if (!strcmp(name, "default-policy")) {
    UmiStudioGtkWorkbenchOptions defaults = umi_studio_gtk_workbench_options_default();
    CHECK(defaults.load_editor_preferences == 1);
    UmiStudioEditorPreferencesStorageSnapshot untouched;
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage_snapshot(workbench, &untouched) ==
          UMI_STATUS_NOT_FOUND);
    CHECK(UmiStudioUiReadEditorPreferences(ui, &current) == UMI_STATUS_OK &&
          current.tab_width == 4U);
    goto cleanup;
  }
  if (!strcmp(name, "relative")) {
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage(workbench, "relative.txt", 0) ==
          UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage(workbench, path, 2) ==
          UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage(NULL, path, 0) ==
          UMI_STATUS_INVALID_ARGUMENT);
    goto cleanup;
  }
  CHECK(UmiStudioUiReadEditorPreferences(ui, &reviewed) == UMI_STATUS_OK);
  int save = !strcmp(name, "save") || !strcmp(name, "save-captured");
  int buttonSave = !strcmp(name, "button-save") || !strcmp(name, "draft-not-applied") ||
                   !strcmp(name, "close-after-save");
  if (buttonSave) {
    GtkWidget *button = Find(GTK_WIDGET(window), "studio.editor.preferences");
    CHECK(button != NULL);
    g_signal_emit_by_name(button, "clicked");
    form = Form();
    CHECK(form != NULL);
    GtkWidget *width = Find(GTK_WIDGET(form), "studio.editor-preferences.width");
    GtkWidget *apply = Find(GTK_WIDGET(form), "studio.editor-preferences.apply");
    GtkWidget *saveButton = Find(GTK_WIDGET(form), "studio.editor-preferences.save");
    CHECK(width && apply && saveButton);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(width), 2.0);
    if (strcmp(name, "draft-not-applied"))
      g_signal_emit_by_name(apply, "clicked");
    g_signal_emit_by_name(saveButton, "clicked");
    if (!strcmp(name, "draft-not-applied")) {
      CHECK(!g_file_test(path, G_FILE_TEST_EXISTS));
      goto cleanup;
    }
    if (!strcmp(name, "close-after-save"))
      gtk_window_destroy(form);
  } else if (strcmp(name, "startup")) {
    if (save)
      CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, &stored) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage(workbench, path, save) ==
          UMI_STATUS_OK);
  }
  binding = g_object_get_data(G_OBJECT(window), "umicom-studio-file-dialog-binding");
  CHECK(binding != NULL);
  g_object_ref(binding);
  if (!strcmp(name, "busy"))
    CHECK(umi_studio_gtk_workbench_editor_preferences_storage(workbench, path, 0) ==
          UMI_STATUS_BUSY);
  if (!strcmp(name, "stale-load") || !strcmp(name, "save-captured")) {
    CHECK(UmiStudioUiReadEditorPreferences(ui, &reviewed) == UMI_STATUS_OK);
    UmiEditorIndentation newer = {8, 1};
    CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, &newer) == UMI_STATUS_OK);
  }
  if (!strcmp(name, "retired"))
    gtk_window_destroy(window);
  CHECK(Wait(binding));
  UmiStudioEditorPreferencesStorageSnapshot *result =
      g_object_get_data(binding, "studio-editor-preference-result");
  CHECK(result != NULL && !result->running);
  UmiStatus expected = !strcmp(name, "missing")      ? UMI_STATUS_NOT_FOUND
                       : !strcmp(name, "malformed")  ? UMI_STATUS_PARSE_ERROR
                       : !strcmp(name, "stale-load") ? UMI_STATUS_INVALID_STATE
                       : !strcmp(name, "retired")    ? UMI_STATUS_CANCELLED
                                                     : UMI_STATUS_OK;
  CHECK(result->status == expected);
  CHECK(UmiStudioUiReadEditorPreferences(ui, &current) == UMI_STATUS_OK);
  if (save || buttonSave) {
    CHECK(UmiDocumentIndentationStorageLoad(path, &stored) == UMI_STATUS_OK);
    CHECK(stored.tab_width == 2U && stored.insert_spaces == (buttonSave ? 1 : 0));
    CHECK(current.tab_width == (!strcmp(name, "save-captured") ? 8U : 2U));
  } else
    CHECK(current.tab_width == (!strcmp(name, "stale-load") ? 8U
                                : expected == UMI_STATUS_OK ? 2U
                                                            : 4U));
cleanup:
  if (form != NULL && UmiGtk4WindowIsOpen(form))
    gtk_window_destroy(form);
  g_clear_object(&form);
  if (workbench != NULL)
    umi_studio_gtk_workbench_destroy(workbench);
  if (binding != NULL) {
    (void)Wait(binding);
    g_clear_object(&binding);
  }
  if (bootstrap != NULL)
    umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_clear_object(&application);
  g_clear_error(&error);
  if (changed && g_chdir(cwd) != 0)
    failed = 1;
  if (oldData != NULL)
    g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  g_free(cwd);
  g_free(oldData);
  g_free(directory);
  g_free(path);
  return failed;
}
