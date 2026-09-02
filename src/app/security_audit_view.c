/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_audit_view.c
 *
 * PURPOSE:
 *   Implement the security audit view behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Security audit view v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_audit_view.h"

/*
 * Find studio security audit while leaving the underlying catalogue or model owned by this
 * module.
 */
const UmiSecurityAuditEntry *umi_studio_security_audit_at(
    const UmiStudioSecurityCentre *centre,
    size_t index)
{
    return centre == NULL ? NULL : umi_security_audit_at(&centre->governance.audit,index);
}

/*
 * Provide the studio security audit denied operation used by this module and its client
 * applications.
 */
size_t umi_studio_security_audit_denied(const UmiStudioSecurityCentre *centre)
{
    return centre == NULL ? 0U : umi_security_audit_denied_count(&centre->governance.audit);
}
