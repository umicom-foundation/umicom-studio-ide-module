/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/project_rename.h
 *
 * PURPOSE:
 *   Compose guarded Framework workspace rename with Studio document and build owners.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_PROJECT_RENAME_H
#define UMICOM_STUDIO_PROJECT_RENAME_H
#include "umicom/platform/workspace_rename.h"
#include "umicom/studio/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioProjectRenameResult {
  UmiWorkspaceRenameResult entry;
  UmiStatus refresh_status;
} UmiStudioProjectRenameResult;
/** Rename a closed workspace entry. Affected open documents, a running build,
 * or a stale workspace reject the operation without writing. Successful disk
 * rename is reported separately from asynchronous index-refresh admission.
 * Do not retry a renamed entry when refresh_status is unavailable or BUSY.
 * No document is saved, closed, retargeted or opened by this operation. */
UmiStatus UmiStudioUiRenameProjectEntry(UmiStudioUi *ui, uint64_t expectedRevision,
                                        const char *relativeSource, const char *newName,
                                        UmiStudioProjectRenameResult *outResult);
#ifdef __cplusplus
}
#endif
#endif
