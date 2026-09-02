/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_workspace.h
 *
 * PURPOSE:
 *   Define Studio editor-workspace composition over the reusable Framework editor session.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This is a product-neutral C23 model. The registry owns snapshot copies by
 * value; callers own external resources and coordinate cross-thread mutation.
 */
#ifndef UMICOM_STUDIO_EDITOR_WORKSPACE_H
#define UMICOM_STUDIO_EDITOR_WORKSPACE_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio editor workspace data shared with callers of this public contract.
 */
typedef struct UmiStudioEditorWorkspace UmiStudioEditorWorkspace;
/**
 * Represent the studio editor workspace snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioEditorWorkspaceSnapshot{uint32_t struct_size;uint32_t api_version;UmiEditorSessionSnapshot editor;uint64_t revision;int available;}UmiStudioEditorWorkspaceSnapshot;
/**
 * Initialise studio editor workspace from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_editor_workspace_create(UmiStudioServices *services,UmiStudioEditorWorkspace **out_workspace);
/**
 * Release or reset state held by studio editor workspace so the same storage can be reused
 * safely.
 */
void umi_studio_editor_workspace_destroy(UmiStudioEditorWorkspace *workspace);
/**
 * Provide the studio editor workspace snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_editor_workspace_snapshot(UmiStudioEditorWorkspace *workspace,UmiStudioEditorWorkspaceSnapshot *out_snapshot);
/**
 * Provide the studio editor workspace session operation used by this module and its client
 * applications.
 */
UmiEditorSession *umi_studio_editor_workspace_session(UmiStudioEditorWorkspace *workspace);
#ifdef __cplusplus
}
#endif
#endif
