/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_save_copy_gtk4.c
 *
 * PURPOSE:
 *   Verify native Save Copy commands preserve source state and refuse existing destinations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/copy.h"
#include "umicom/document/format.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
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
  const char *name = argv[1], *cases[] = {"new",      "existing", "same-source", "unicode",
                                          "readonly", "empty",    "retired",     "utf16"};
  bool known = false;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (strcmp(name, cases[i]) == 0)
      known = true;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  int failed = 0;
  bool changed = false;
  char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
  char *directory = NULL, *source = NULL, *destination = NULL, *contents = NULL;
  UmiSettings *settings = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioGtkWorkbench *workbench = NULL;
  GtkApplication *application = NULL;
  GError *error = NULL;
  directory = g_dir_make_tmp("umicom-source-copy-XXXXXX", &error);
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
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  application = gtk_application_new("org.umicom.studio.copytest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  /* The workbench borrows the bootstrap-owned desktop shell. Keep the old
   * incomplete call for review; supply the same owner used by Studio itself. */
#if 0
  CHECK(umi_studio_gtk_workbench_create(application, ui, &workbench) == UMI_STATUS_OK);
#endif
  CHECK(umi_studio_gtk_workbench_create(application, ui,
                                        umi_studio_bootstrap_desktop_shell(bootstrap),
                                        &workbench) == UMI_STATUS_OK);
  source = g_build_filename(directory, "source.c", NULL);
  destination =
      g_build_filename(directory, strcmp(name, "unicode") == 0 ? "caf\xc3\xa9.c" : "copy.c", NULL);
  CHECK(g_file_set_contents(source, "saved\n", -1, &error));
  CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.open", source) == UMI_STATUS_OK);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  UmiDocumentWorkingCopySnapshot before, after;
  CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
  UmiUiDocumentViewSnapshot view;
  CHECK(umi_ui_document_view_model_find(views, before.view_id, &view) == UMI_STATUS_OK);
  const char *draft = strcmp(name, "empty") == 0 ? "" : "draft\n";
  view.dirty = 1;
  view.cursor_offset = 0U;
  view.selection_length = 0U;
  CHECK(UmiUiDocumentViewModelUpsertText(views, &view, draft, strlen(draft)) == UMI_STATUS_OK);
  if (strcmp(name, "readonly") == 0) {
    view.read_only = 1;
    CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
  }
  if (strcmp(name, "utf16") == 0) {
    UmiDocumentFormatOptions format = {UMI_DOCUMENT_ENCODING_UTF16_LE,
                                       UMI_DOCUMENT_LINE_ENDING_NONE, 0};
    CHECK(UmiDocumentCoordinatorSetFormat(documents, &format) == UMI_STATUS_OK);
  }
  CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
  if (strcmp(name, "existing") == 0)
    CHECK(g_file_set_contents(destination, "keep\n", -1, &error));
  if (strcmp(name, "retired") == 0)
    gtk_window_destroy(umi_studio_gtk_workbench_window(workbench));
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "file.save-copy", strcmp(name, "same-source") == 0 ? source : destination);
  if (strcmp(name, "retired") == 0) {
    CHECK(status != UMI_STATUS_OK && !g_file_test(destination, G_FILE_TEST_EXISTS));
    goto cleanup;
  }
  UmiStatus wanted = strcmp(name, "existing") == 0      ? UMI_STATUS_ALREADY_EXISTS
                     : strcmp(name, "same-source") == 0 ? UMI_STATUS_INVALID_ARGUMENT
                                                        : UMI_STATUS_OK;
  CHECK(status == wanted);
  CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK);
  CHECK(before.document_id == after.document_id && before.dirty == after.dirty &&
        before.revision == after.revision && before.undo_count == after.undo_count &&
        before.redo_count == after.redo_count && strcmp(before.path, after.path) == 0);
  gsize length = 0U;
  CHECK(g_file_get_contents(source, &contents, &length, &error) && length == 6U &&
        memcmp(contents, "saved\n", 6U) == 0);
  g_clear_pointer(&contents, g_free);
  if (strcmp(name, "same-source") != 0) {
    CHECK(g_file_get_contents(destination, &contents, &length, &error));
    if (strcmp(name, "existing") == 0)
      CHECK(length == 5U && memcmp(contents, "keep\n", 5U) == 0);
    else if (strcmp(name, "utf16") == 0) {
      const unsigned char expected[] = {0xffU, 0xfeU, 'd', 0U,  'r', 0U,   'a',
                                        0U,    'f',   0U,  't', 0U,  '\n', 0U};
      CHECK(length == sizeof expected && memcmp(contents, expected, length) == 0);
    } else
      CHECK(length == strlen(draft) && memcmp(contents, draft, length) == 0);
  }
cleanup:
  if (workbench != NULL)
    umi_studio_gtk_workbench_destroy(workbench);
  pump();
  if (bootstrap != NULL)
    umi_studio_bootstrap_destroy(bootstrap);
  if (settings != NULL)
    umi_settings_destroy(settings);
  g_clear_object(&application);
  g_clear_error(&error);
  if (changed)
    (void)g_chdir(original);
  if (oldData != NULL)
    (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  g_free(contents);
  g_free(source);
  g_free(destination);
  g_free(directory);
  g_free(oldData);
  g_free(original);
  return failed;
}
