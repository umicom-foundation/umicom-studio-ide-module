/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/security.h
 *
 * PURPOSE:
 *   Expose Studio security reports, workspace-trust decisions and permission
 *   checks through the shared Framework security context.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_H
#define UMICOM_STUDIO_SECURITY_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio security report data shared with callers of this public contract.
 */
typedef struct UmiStudioSecurityReport {
    size_t identities;
    size_t roles;
    size_t sessions;
    size_t trusted_workspaces;
    size_t events;
    size_t denied_events;
    size_t policy_rules;
} UmiStudioSecurityReport;

/**
 * Provide the studio security report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_report(UmiStudioServices *services,
                                     UmiStudioSecurityReport *out_report);
/**
 * Provide the studio security set workspace trust operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_set_workspace_trust(
    UmiStudioServices *services,
    const char *path,
    UmiWorkspaceTrustLevel level);
/**
 * Provide the studio security authorise operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_authorise(UmiStudioServices *services,
                                       const char *principal,
                                       const char *role_id,
                                       const char *permission,
                                       const char *resource,
                                       UmiAuthorisationDecision *out_decision);

#ifdef __cplusplus
}
#endif

#endif
