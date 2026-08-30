/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_audit_view.h
 *
 * PURPOSE:
 *   Publish Studio's public security audit view contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_AUDIT_VIEW_H
#define UMICOM_STUDIO_SECURITY_AUDIT_VIEW_H
#include "umicom/studio/security_centre.h"
const UmiSecurityAuditEntry *umi_studio_security_audit_at(const UmiStudioSecurityCentre *centre,size_t index);
size_t umi_studio_security_audit_denied(const UmiStudioSecurityCentre *centre);
#endif
