/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_preferences.c
 *
 * PURPOSE:
 *   Verify reviewed preference updates preserve settings, document drafts and file persistence
 * boundaries.
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
#include "umicom/studio/editor_preferences.h"
#include "umicom/studio/settings.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%d: %s\n", __LINE__, #x);                                                   \
      failed = 1;                                                                                  \
      goto done;                                                                                   \
    }                                                                                              \
  } while (0)
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *name = argv[1],
             *cases[] = {"read",         "spaces",       "tabs",      "stale-width",
                         "stale-spaces", "invalid",      "arguments", "repeat",
                         "unrelated",    "session-only", "draft"};
  int known = 0;
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(name, cases[i]))
      known = 1;
  if (!known)
    return 2;
  int failed = 0;
  UmiSettings *seed = NULL, *disk = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  CHECK(umi_fs_make_directories("config") == UMI_STATUS_OK);
  CHECK(umi_studio_settings_create(&seed) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(seed, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(seed, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiSettings *settings = umi_studio_services_settings(umi_studio_bootstrap_services(bootstrap));
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiEditorIndentation reviewed, candidate = {2, 1}, after, policy;
  CHECK(UmiStudioUiReadEditorPreferences(ui, &reviewed) == UMI_STATUS_OK);
  CHECK(reviewed.tab_width == 4U && reviewed.insert_spaces == 1);
  if (!strcmp(name, "read"))
    goto done;
  if (!strcmp(name, "arguments")) {
    after = (UmiEditorIndentation){8, 0};
    CHECK(UmiStudioUiReadEditorPreferences(NULL, &after) == UMI_STATUS_INVALID_ARGUMENT &&
          after.tab_width == 8U);
    CHECK(UmiStudioUiReadEditorPreferences(ui, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioUiUpdateEditorPreferences(NULL, &reviewed, &candidate) ==
          UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioUiUpdateEditorPreferences(ui, NULL, &candidate) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    goto done;
  }
  char id[UMI_UI_ID_CAPACITY] = {0};
  if (!strcmp(name, "draft")) {
    CHECK(umi_document_coordinator_new(documents, "preferences.c", id, sizeof id) == UMI_STATUS_OK);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 3U;
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "abc", 3U) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
  }
  UmiStatus expected = UMI_STATUS_OK;
  if (!strcmp(name, "tabs"))
    candidate = (UmiEditorIndentation){8, 0};
  if (!strcmp(name, "stale-width")) {
    CHECK(umi_settings_set_integer(settings, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, 8) ==
          UMI_STATUS_OK);
    expected = UMI_STATUS_INVALID_STATE;
  }
  if (!strcmp(name, "stale-spaces")) {
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES, 0) ==
          UMI_STATUS_OK);
    expected = UMI_STATUS_INVALID_STATE;
  }
  if (!strcmp(name, "invalid")) {
    candidate.tab_width = 0U;
    expected = UMI_STATUS_INVALID_ARGUMENT;
  }
  if (!strcmp(name, "unrelated"))
    CHECK(umi_settings_set_integer(settings, UMI_STUDIO_SETTING_BUILD_PARALLEL_JOBS, 2) ==
          UMI_STATUS_OK);
  CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, &candidate) == expected);
  CHECK(UmiStudioUiReadEditorPreferences(ui, &after) == UMI_STATUS_OK);
  CHECK(UmiDocumentCoordinatorGetIndentation(documents, &policy) == UMI_STATUS_OK);
  if (expected == UMI_STATUS_OK) {
    CHECK(after.tab_width == candidate.tab_width && after.insert_spaces == candidate.insert_spaces);
    CHECK(policy.tab_width == candidate.tab_width &&
          policy.insert_spaces == candidate.insert_spaces);
  } else {
    CHECK(policy.tab_width == 4U && policy.insert_spaces == 1);
    CHECK(after.tab_width == (!strcmp(name, "stale-width") ? 8U : 4U));
    CHECK(after.insert_spaces == (!strcmp(name, "stale-spaces") ? 0 : 1));
  }
  if (!strcmp(name, "repeat")) {
    CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, &candidate) ==
          UMI_STATUS_INVALID_STATE);
    reviewed = candidate;
    candidate.tab_width = 8U;
    CHECK(UmiStudioUiUpdateEditorPreferences(ui, &reviewed, &candidate) == UMI_STATUS_OK);
  }
  if (!strcmp(name, "session-only")) {
    CHECK(umi_studio_settings_create(&disk) == UMI_STATUS_OK);
    int loaded = 0;
    int64_t width = 0;
    CHECK(umi_studio_settings_load_if_present(disk, umi_studio_settings_default_path(), &loaded) ==
              UMI_STATUS_OK &&
          loaded);
    CHECK(umi_settings_get_integer(disk, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, &width) ==
              UMI_STATUS_OK &&
          width == 4);
  }
  if (!strcmp(name, "draft")) {
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char *text = NULL;
    size_t bytes = 0;
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &text, &bytes) == UMI_STATUS_OK);
    int unchanged = bytes == 3U && !strcmp(text, "abc");
    UmiUiDocumentViewModelFreeText(text);
    CHECK(unchanged);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorInsertIndentation(documents, active.document_id) == UMI_STATUS_OK);
    CHECK(UmiDocumentCoordinatorUndo(documents, active.document_id) == UMI_STATUS_OK);
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &text, &bytes) == UMI_STATUS_OK);
    unchanged = bytes == 3U && !strcmp(text, "abc");
    UmiUiDocumentViewModelFreeText(text);
    CHECK(unchanged);
  }
done:
  umi_settings_destroy(disk);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(seed);
  return failed;
}
