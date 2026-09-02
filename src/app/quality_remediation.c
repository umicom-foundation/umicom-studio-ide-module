/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_remediation.c
 *
 * PURPOSE:
 *   Implement the quality remediation behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Quality remediation guidance | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/quality_remediation.h"
/*
 * Provide the studio quality remediation refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_quality_remediation_refresh(UmiStudioQualityCentre *centre)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_codeguard_remediation_plan_build(&centre->session.evidence,&centre->session.remediation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->session.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Find studio quality remediation while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiCodeGuardRemediationAction *umi_studio_quality_remediation_at(const UmiStudioQualityCentre *centre,size_t index)
{
    return centre == NULL ? NULL : umi_codeguard_remediation_plan_at(&centre->session.remediation,index);
}
