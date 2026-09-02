/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_layout_session.h
 *
 * PURPOSE:
 *   Persist Framework editor-group ordering through Studio's existing
 *   workspace-local Session Store.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_EDITOR_LAYOUT_SESSION_H
#define UMICOM_STUDIO_EDITOR_LAYOUT_SESSION_H

#include "umicom/platform/session_store.h"
#include "umicom/ui/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio editor layout session restore operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_editor_layout_session_restore(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session,
    UmiUiDocumentLayoutApplyResult *out_result);
/**
 * Write studio editor layout session in its stable representation and report capacity or
 * input failures to the caller.
 */
UmiStatus umi_studio_editor_layout_session_save(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session);

#ifdef __cplusplus
}
#endif

#endif
