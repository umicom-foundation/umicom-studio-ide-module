/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace_profile_session.h
 *
 * PURPOSE:
 *   Persist user-created Framework workspace profiles through Studio's
 *   existing Session Store without introducing another settings backend.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_WORKSPACE_PROFILE_SESSION_H
#define UMICOM_STUDIO_WORKSPACE_PROFILE_SESSION_H

#include "umicom/platform/session_store.h"
#include "umicom/ui/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio workspace profile session restore operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_workspace_profile_session_restore(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session);
/**
 * Write studio workspace profile session in its stable representation and report capacity
 * or input failures to the caller.
 */
UmiStatus umi_studio_workspace_profile_session_save(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session);

#ifdef __cplusplus
}
#endif

#endif
