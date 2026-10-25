/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_project_rename.c
 *
 * PURPOSE:
 *   Exercise Studio rename blockers and preserve source drafts while requesting index refresh.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/filesystem.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/project_rename.h"
#include "umicom/studio/workspace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                      \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)
int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *test = argv[1];
  const char *cases[] = {"closed",     "open",     "dirty",     "inactive", "destination",
                         "descendant", "existing", "untrusted", "stale",    "arguments"};
  int known = 0;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(cases[i], test))
      known = 1;
  CHECK(known);
  char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY],
      destination[UMI_PATH_CAPACITY], id[UMI_UI_ID_CAPACITY];
  CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
  CHECK(umi_path_join(cwd, "studio-rename", root, sizeof root) == UMI_STATUS_OK);
  CHECK(!umi_fs_exists(root) && umi_fs_make_directories(root) == UMI_STATUS_OK);
  const char *relative = !strcmp(test, "descendant") ? "src/old.c" : "old.c";
  if (!strcmp(test, "descendant")) {
    char folder[UMI_PATH_CAPACITY];
    CHECK(umi_path_join(root, "src", folder, sizeof folder) == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(folder) == UMI_STATUS_OK);
  }
  CHECK(umi_path_join(root, relative, source, sizeof source) == UMI_STATUS_OK);
  CHECK(umi_path_join(root, "new.c", destination, sizeof destination) == UMI_STATUS_OK);
  CHECK(umi_fs_write_text(source, "original") == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  CHECK(umi_studio_workspace_open(services, root, 1, 0) == UMI_STATUS_OK);
  UmiWorkspaceGraphSnapshot workspace;
  CHECK(umi_workspace_graph_snapshot(umi_studio_services_workspace(services), &workspace) ==
        UMI_STATUS_OK);
  uint64_t revision = workspace.revision;
  UmiStatus expected = UMI_STATUS_OK;
  if (!strcmp(test, "existing") || !strcmp(test, "destination"))
    CHECK(umi_fs_write_text(destination, "destination") == UMI_STATUS_OK);
  if (!strcmp(test, "open") || !strcmp(test, "dirty") || !strcmp(test, "inactive") ||
      !strcmp(test, "destination") || !strcmp(test, "descendant")) {
    CHECK(umi_document_coordinator_open(documents,
                                        !strcmp(test, "destination") ? destination : source, id,
                                        sizeof id) == UMI_STATUS_OK);
    expected = UMI_STATUS_BUSY;
  }
  if (!strcmp(test, "dirty")) {
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "unsaved", 7U) == UMI_STATUS_OK);
  }
  if (!strcmp(test, "inactive"))
    CHECK(umi_document_coordinator_new(documents, "another.c", id, sizeof id) == UMI_STATUS_OK);
  if (!strcmp(test, "existing"))
    expected = UMI_STATUS_ALREADY_EXISTS;
  if (!strcmp(test, "stale")) {
    ++revision;
    expected = UMI_STATUS_BUSY;
  }
  if (!strcmp(test, "untrusted")) {
    CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    revision = 0U;
    expected = UMI_STATUS_PERMISSION_DENIED;
  }
  UmiStudioProjectRenameResult result;
  if (!strcmp(test, "arguments")) {
    CHECK(UmiStudioUiRenameProjectEntry(NULL, 0U, relative, "new.c", &result) ==
          UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioUiRenameProjectEntry(ui, 0U, relative, "new.c", NULL) ==
          UMI_STATUS_INVALID_ARGUMENT);
  }
  size_t count = umi_document_coordinator_count(documents);
  UmiStatus status = UmiStudioUiRenameProjectEntry(
      ui, revision, !strcmp(test, "descendant") ? "src" : relative, "new.c", &result);
  int unavailable = status == UMI_STATUS_NOT_IMPLEMENTED;
  if (!unavailable)
    CHECK(status == expected);
  CHECK(umi_document_coordinator_count(documents) == count);
  CHECK(result.entry.renamed == (!unavailable && expected == UMI_STATUS_OK));
  CHECK(umi_fs_exists(source) == !result.entry.renamed);
  char *text = NULL;
  CHECK(umi_fs_read_text(result.entry.renamed ? destination : source, &text, NULL) ==
        UMI_STATUS_OK);
  CHECK(!strcmp(text, "original"));
  umi_fs_free_text(text);
  if (!strcmp(test, "dirty")) {
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(umi_ui_workbench_documents(umi_studio_ui_workbench(ui)),
                                         id, &text, &bytes) == UMI_STATUS_OK);
    CHECK(bytes == 7U && !strcmp(text, "unsaved"));
    UmiUiDocumentViewModelFreeText(text);
  }
  umi_studio_bootstrap_destroy(bootstrap);
  CHECK(umi_fs_remove_tree(root) == UMI_STATUS_OK);
  return unavailable ? 77 : 0;
}
