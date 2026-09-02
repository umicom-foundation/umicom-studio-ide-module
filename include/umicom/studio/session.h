/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/session.h
 *
 * PURPOSE:
 *   Define Studio-specific session keys and narrow helpers over the Framework
 *   session and recovery services without making GTK widgets authoritative.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SESSION_H
#define UMICOM_STUDIO_SESSION_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_SESSION_DEFAULT_PATH ".umicom/studio.session"
#define UMI_STUDIO_RECOVERY_DEFAULT_ROOT ".umicom/recovery"

/**
 * Provide the studio session default path operation used by this module and its client
 * applications.
 */
const char *umi_studio_session_default_path(void);
/**
 * Provide the studio recovery default root operation used by this module and its client
 * applications.
 */
const char *umi_studio_recovery_default_root(void);
/**
 * Provide the studio session set workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_set_workspace(UmiStudioServices *services,
                                           const char *workspace_root);
/**
 * Provide the studio session set active document operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_set_active_document(
    UmiStudioServices *services,
    UmiDocumentId document_id
);
/**
 * Provide the studio session remember document operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_remember_document(
    UmiStudioServices *services,
    UmiDocumentId document_id,
    const char *path
);
/**
 * Provide the studio session forget document operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_forget_document(
    UmiStudioServices *services,
    UmiDocumentId document_id
);
/**
 * Write studio session in its stable representation and report capacity or input failures
 * to the caller.
 */
UmiStatus umi_studio_session_save(UmiStudioServices *services);
/**
 * Provide the studio session reload operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_reload(UmiStudioServices *services,
                                    int *out_loaded);

#ifdef __cplusplus
}
#endif

#endif
