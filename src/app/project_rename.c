/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/project_rename.c
 *
 * PURPOSE:
 *   Protect open drafts and running builds while using Framework workspace rename.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/project_rename.h"
#include "umicom/document/path_usage.h"
#include "umicom/studio/build.h"
#include <string.h>
UmiStatus UmiStudioUiRenameProjectEntry(UmiStudioUi *ui, uint64_t expectedRevision,
                                        const char *relativeSource, const char *newName,
                                        UmiStudioProjectRenameResult *outResult) {
  if (outResult == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  memset(outResult, 0, sizeof(*outResult));
  outResult->refresh_status = UMI_STATUS_NOT_FOUND;
  if (ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiStudioServices *services = umi_studio_ui_services(ui);
  if (services == NULL)
    return UMI_STATUS_INVALID_STATE;
  if (UmiStudioBuildBusy(umi_studio_services_build(services)))
    return UMI_STATUS_BUSY;
  UmiWorkspaceGraph *workspace = umi_studio_services_workspace(services);
  UmiWorkspaceRenameResult preview;
  UmiStatus status =
      UmiWorkspaceRenamePreview(workspace, expectedRevision, relativeSource, newName, &preview);
  if (status != UMI_STATUS_OK)
    return status;
  int inUse = 0;
  status = UmiDocumentCoordinatorPathInUse(umi_studio_ui_documents(ui), preview.source,
                                           preview.destination, &inUse);
  if (status != UMI_STATUS_OK)
    return status;
  if (inUse)
    return UMI_STATUS_BUSY;
  /* No owner callback occurs between these checks and the synchronous
   * filesystem operation. Refresh admission cannot undo a completed rename. */
  status = UmiWorkspaceRenameEntry(workspace, expectedRevision, relativeSource, newName,
                                   &outResult->entry);
  if (outResult->entry.renamed)
    outResult->refresh_status = UmiStudioUiRefreshProjectFiles(ui);
  return status;
}
