/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_approvals.h
 *
 * PURPOSE:
 *   Publish Studio's public security approvals contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_APPROVALS_H
#define UMICOM_STUDIO_SECURITY_APPROVALS_H
#include "umicom/studio/security_centre.h"
/**
 * Provide the studio security approval request operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_approval_request(UmiStudioSecurityCentre *centre,const UmiSecurityApproval *approval);
/**
 * Provide the studio security approval decide operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_approval_decide(UmiStudioSecurityCentre *centre,const char *approval_id,const char *decided_by,UmiSecurityApprovalState state,const char *reason);
/**
 * Find studio security approval while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiSecurityApproval *umi_studio_security_approval_find(const UmiStudioSecurityCentre *centre,const char *approval_id,uint64_t now_ns);
#endif
