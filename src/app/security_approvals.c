/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_approvals.c
 *
 * PURPOSE:
 *   Implement the security approvals behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Security approval administration v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_approvals.h"
/*
 * Provide the studio security approval request operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_approval_request(UmiStudioSecurityCentre *centre,const UmiSecurityApproval *approval)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_security_approval_request(&centre->governance.approvals,approval);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->governance.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Provide the studio security approval decide operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_approval_decide(UmiStudioSecurityCentre *centre,const char *approval_id,const char *decided_by,UmiSecurityApprovalState state,const char *reason)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_security_approval_decide(&centre->governance.approvals,approval_id,decided_by,state,reason);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->governance.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Find studio security approval while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiSecurityApproval *umi_studio_security_approval_find(const UmiStudioSecurityCentre *centre,const char *approval_id,uint64_t now_ns) { return centre == NULL ? NULL : umi_security_approval_find(&centre->governance.approvals,approval_id,now_ns); }
