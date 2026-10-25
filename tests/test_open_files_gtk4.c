/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_open_files_gtk4.c
 *
 * PURPOSE:
 *   Follow native queued opening through real documents and retained host state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
 * Follow native queued opening through real documents and retained host state. */
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                                   \
  do {                                                                                             \
    if (!(c)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c);                                      \
      failed = 1;                                                                                  \
      goto cleanup;                                                                                \
    }                                                                                              \
  } while (0)
static void pump(void) {
  for (unsigned i = 0U; i < 128U && g_main_context_pending(NULL); ++i)
    (void)g_main_context_iteration(NULL, FALSE);
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *name = argv[1];
  /* Extend the native queue cases with the real editor drop controller.
   * The original catalogue is retained so its coverage remains visible. */
#if 0
  const char *cases[] = {"all",      "mixed",  "duplicate", "cancel", "busy",
                         "captured", "retire", "empty",     "limit",  "dirty"};
#endif
  const char *cases[] = {"all", "mixed", "duplicate", "cancel", "busy", "captured",
      "retire", "empty", "limit", "dirty", "drop", "drop-mixed", "drop-remote",
      "cancel-command", "results-command"};
  bool known = false;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (strcmp(name, cases[i]) == 0)
      known = true;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  int failed = 0;
  char *original = g_get_current_dir(), *directory = NULL,
       *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
  char *first = NULL, *second = NULL, *missing = NULL;
  bool changed = false;
  UmiSettings *settings = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioGtkWorkbench *workbench = NULL;
  GtkApplication *application = NULL;
  GError *error = NULL;
  directory = g_dir_make_tmp("umicom-open-files-XXXXXX", &error);
  CHECK(directory != NULL && g_chdir(directory) == 0);
  changed = true;
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioServicesOptions services = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  application = gtk_application_new("org.umicom.studio.open-files-test", G_APPLICATION_NON_UNIQUE);
  CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  first = g_build_filename(directory, "one.c", NULL);
  second = g_build_filename(directory, "two.c", NULL);
  missing = g_build_filename(directory, "absent.c", NULL);
  CHECK(g_file_set_contents(first, "int one = 1;\n", -1, &error));
  CHECK(g_file_set_contents(second, "int two = 2;\n", -1, &error));
  const char *paths[] = {first, second};
  size_t before = umi_document_coordinator_count(documents);
  UmiDocumentOpenBatchSummary summary;
  int running = 99;
  CHECK(umi_studio_gtk_workbench_open_files_snapshot(workbench, &summary, &running) ==
            UMI_STATUS_NOT_FOUND &&
        running == 99);
  CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open.cancel", NULL)
      == UMI_STATUS_PERMISSION_DENIED);
  CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open.results", NULL)
      == UMI_STATUS_PERMISSION_DENIED);
  if (strcmp(name, "empty") == 0 || strcmp(name, "limit") == 0) {
    UmiStatus status = umi_studio_gtk_workbench_open_files(
        workbench, paths, strcmp(name, "empty") == 0 ? 0U : UMI_DOCUMENT_OPEN_BATCH_MAXIMUM + 1U);
    CHECK(status == (strcmp(name, "empty") == 0 ? UMI_STATUS_INVALID_ARGUMENT
                                                : UMI_STATUS_CAPACITY_EXCEEDED));
    CHECK(umi_document_coordinator_count(documents) == before);
    goto cleanup;
  }
  if (strcmp(name, "mixed") == 0 || strcmp(name, "drop-mixed") == 0)
    paths[0] = missing;
  if (strcmp(name, "duplicate") == 0 || strcmp(name, "dirty") == 0)
    paths[1] = first;
  char view_id[UMI_UI_ID_CAPACITY] = {0};
  if (strcmp(name, "dirty") == 0) {
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", first) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot current;
    CHECK(umi_document_coordinator_active_snapshot(documents, &current) == UMI_STATUS_OK);
    strcpy(view_id, current.view_id);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "unsaved\n", 8U) == UMI_STATUS_OK);
  }
  /* A native drop must enter the same queue as the chooser and explicit API.
   * The historical direct-only invocation remains for review. */
#if 0
  CHECK(umi_studio_gtk_workbench_open_files(workbench, paths, 2U) == UMI_STATUS_OK);
#endif
  if (g_str_has_prefix(name, "drop")) {
    GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
    gtk_window_present(window);
    GtkWidget *area = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window),
        "studio.files.drop-area");
    CHECK(area != NULL);
    gint64 ready = g_get_monotonic_time() + 5 * G_TIME_SPAN_SECOND;
    while (!gtk_widget_get_mapped(area) && g_get_monotonic_time() < ready) {
      pump();
      g_usleep(1000U);
    }
    CHECK(gtk_widget_get_mapped(area));
    GListModel *controllers = gtk_widget_observe_controllers(area);
    GtkDropTarget *drop = NULL;
    for (guint i = 0U; i < g_list_model_get_n_items(controllers); ++i) {
      GtkEventController *controller = g_list_model_get_item(controllers, i);
      if (GTK_IS_DROP_TARGET(controller) &&
          g_strcmp0(gtk_event_controller_get_name(controller), "umicom.local-files") == 0) {
        drop = GTK_DROP_TARGET(controller);
        break;
      }
      g_object_unref(controller);
    }
    g_object_unref(controllers);
    CHECK(drop != NULL);
    GFile *files[] = {g_file_new_for_path(paths[0]),
        strcmp(name, "drop-remote") == 0 ? g_file_new_for_uri("https://example.invalid/file.c")
                                       : g_file_new_for_path(paths[1])};
    GValue value = G_VALUE_INIT;
    g_value_init(&value, GDK_TYPE_FILE_LIST);
    g_value_take_boxed(&value, gdk_file_list_new_from_array(files, 2U));
    gboolean accepted = FALSE;
    g_signal_emit_by_name(drop, "drop", &value, 0.0, 0.0, &accepted);
    g_value_unset(&value);
    g_object_unref(files[0]);
    g_object_unref(files[1]);
    g_object_unref(drop);
    if (strcmp(name, "drop-remote") == 0) {
      CHECK(!accepted && umi_document_coordinator_count(documents) == before);
      CHECK(umi_studio_gtk_workbench_open_files_snapshot(workbench, &summary, &running)
          == UMI_STATUS_NOT_FOUND);
      goto cleanup;
    }
    CHECK(accepted);
  } else {
    CHECK(umi_studio_gtk_workbench_open_files(workbench, paths, 2U) == UMI_STATUS_OK);
  }
  CHECK(umi_studio_gtk_workbench_open_files_snapshot(workbench, &summary, &running) ==
            UMI_STATUS_OK &&
        running && summary.pending == 2U);
  if (strcmp(name, "busy") == 0)
    CHECK(umi_studio_gtk_workbench_open_files(workbench, paths, 2U) == UMI_STATUS_BUSY);
  if (strcmp(name, "captured") == 0) {
    memset(first, 'x', strlen(first));
    memset(second, 'x', strlen(second));
  }
  if (strcmp(name, "cancel") == 0)
    CHECK(umi_studio_gtk_workbench_cancel_open_files(workbench) == UMI_STATUS_OK);
  if (strcmp(name, "cancel-command") == 0)
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open.cancel", NULL)
        == UMI_STATUS_OK);
  if (strcmp(name, "retire") == 0) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    pump();
    CHECK(umi_document_coordinator_count(documents) == before);
    goto cleanup;
  }
  gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
  do {
    pump();
    CHECK(umi_studio_gtk_workbench_open_files_snapshot(workbench, &summary, &running) ==
          UMI_STATUS_OK);
    if (!running)
      break;
    g_usleep(1000U);
  } while (g_get_monotonic_time() < deadline);
  CHECK(!running && summary.pending == 0U);
  if (strcmp(name, "cancel") == 0 || strcmp(name, "cancel-command") == 0) {
    CHECK(summary.cancelled == 2U && summary.succeeded == 0U);
    CHECK(umi_document_coordinator_count(documents) == before);
  } else if (strcmp(name, "mixed") == 0 || strcmp(name, "drop-mixed") == 0) {
    CHECK(summary.failed == 1U && summary.succeeded == 1U &&
          umi_document_coordinator_count(documents) == before + 1U);
    UmiDocumentOpenItem item;
    CHECK(umi_studio_gtk_workbench_open_file_result(workbench, 0U, &item) == UMI_STATUS_OK);
    CHECK(item.state == UMI_DOCUMENT_OPEN_FAILED && item.status != UMI_STATUS_OK);
    bool report = false;
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
      GtkWidget *window = g_list_model_get_item(windows, i);
      if (umi_gtk4_automation_find_tagged_widget(window, "studio.file.open-results") != NULL)
        report = true;
      g_object_unref(window);
    }
    CHECK(report);
  } else {
    CHECK(summary.succeeded == 2U && summary.failed == 0U);
    CHECK(umi_document_coordinator_count(documents) ==
          before + ((strcmp(name, "duplicate") == 0 || strcmp(name, "dirty") == 0) ? 1U : 2U));
  }
  if (strcmp(name, "dirty") == 0) {
    char *text = NULL;
    size_t length = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &length) == UMI_STATUS_OK);
    bool same = length == 8U && strcmp(text, "unsaved\n") == 0;
    UmiUiDocumentViewModelFreeText(text);
    CHECK(same);
  }
  if (strcmp(name, "results-command") == 0) {
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open.results", NULL)
        == UMI_STATUS_OK);
    bool found = false;
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
      GtkWidget *window = g_list_model_get_item(windows, i);
      if (umi_gtk4_automation_find_tagged_widget(window, "studio.file.open-results") != NULL)
        found = true;
      g_object_unref(window);
    }
    CHECK(found);
  }
  CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open.cancel", NULL)
      == UMI_STATUS_PERMISSION_DENIED);
  CHECK(umi_studio_gtk_workbench_cancel_open_files(workbench) == UMI_STATUS_NOT_FOUND);
cleanup:
  if (workbench != NULL)
    umi_studio_gtk_workbench_destroy(workbench);
  /* Retired idle work resolves its weak host before touching borrowed services. */
  pump();
  if (bootstrap != NULL)
    umi_studio_bootstrap_destroy(bootstrap);
  if (settings != NULL)
    umi_settings_destroy(settings);
  g_clear_object(&application);
  g_clear_error(&error);
  if (changed)
    (void)g_chdir(original);
  if (old_data != NULL)
    (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  g_free(first);
  g_free(second);
  g_free(missing);
  g_free(old_data);
  g_free(directory);
  g_free(original);
  return failed;
}
