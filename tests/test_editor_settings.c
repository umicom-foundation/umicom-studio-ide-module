/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_settings.c
 *
 * PURPOSE:
 *   Exercise stored and changed Studio indentation preferences through the shared document
 * coordinator.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/edit.h"
#include "umicom/document/indentation.h"
#include "umicom/platform/filesystem.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/editor_settings.h"
#include "umicom/studio/settings.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                                   \
  do {                                                                                             \
    if (!(c)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c);                                      \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)
int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *name = argv[1], *cases[] = {"defaults", "startup", "spaces",   "tabs",
                                          "refresh",  "invalid", "arguments"};
  int known = 0;
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(name, cases[i]))
      known = 1;
  CHECK(known);
  CHECK(umi_fs_make_directories("config") == UMI_STATUS_OK);
  UmiSettings *seed = NULL;
  CHECK(umi_studio_settings_create(&seed) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  if (!strcmp(name, "startup"))
    CHECK(umi_settings_set_integer(seed, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, 2) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(seed, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  umi_settings_destroy(seed);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiSettings *settings = umi_studio_services_settings(umi_studio_bootstrap_services(bootstrap));
  UmiEditorIndentation policy;
  CHECK(UmiDocumentCoordinatorGetIndentation(documents, &policy) == UMI_STATUS_OK);
  CHECK(policy.tab_width == (!strcmp(name, "startup") ? 2U : 4U) && policy.insert_spaces == 1);
  if (!strcmp(name, "arguments")) {
    CHECK(UmiStudioUiApplyEditorSettings(NULL) == UMI_STATUS_INVALID_ARGUMENT);
    umi_studio_bootstrap_destroy(bootstrap);
    return 0;
  }
  if (!strcmp(name, "invalid")) {
    CHECK(umi_settings_set_integer(settings, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, 0) !=
          UMI_STATUS_OK);
    CHECK(UmiStudioUiApplyEditorSettings(ui) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorGetIndentation(documents, &policy) == UMI_STATUS_OK &&
          policy.tab_width == 4U);
    umi_studio_bootstrap_destroy(bootstrap);
    return 0;
  }
  if (!strcmp(name, "spaces") || !strcmp(name, "refresh"))
    CHECK(umi_settings_set_integer(settings, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, 2) ==
          UMI_STATUS_OK);
  if (!strcmp(name, "tabs"))
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES, 0) ==
          UMI_STATUS_OK);
  if (!strcmp(name, "refresh"))
    CHECK(umi_studio_ui_refresh(ui) == UMI_STATUS_OK);
  else
    CHECK(UmiStudioUiApplyEditorSettings(ui) == UMI_STATUS_OK);
  char id[UMI_UI_ID_CAPACITY];
  CHECK(umi_document_coordinator_new(documents, "settings.c", id, sizeof id) == UMI_STATUS_OK);
  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  UmiUiDocumentViewSnapshot view;
  CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
  view.cursor_offset = 1U;
  view.dirty = 1;
  CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "x", 1U) == UMI_STATUS_OK);
  CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
  UmiDocumentWorkingCopySnapshot active;
  CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorInsertIndentation(documents, active.document_id) == UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorGetIndentation(documents, &policy) == UMI_STATUS_OK);
  const char *expected = !policy.insert_spaces ? "x\t" : policy.tab_width == 2U ? "x " : "x   ";
  char *output = NULL;
  size_t bytes = 0U;
  CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &bytes) == UMI_STATUS_OK &&
        bytes == strlen(expected) && !strcmp(output, expected));
  UmiUiDocumentViewModelFreeText(output);
  output = NULL;
  CHECK(UmiDocumentCoordinatorUndo(documents, active.document_id) == UMI_STATUS_OK);
  CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &bytes) == UMI_STATUS_OK &&
        bytes == 1U && !strcmp(output, "x"));
  UmiUiDocumentViewModelFreeText(output);
  umi_studio_bootstrap_destroy(bootstrap);
  return 0;
}
