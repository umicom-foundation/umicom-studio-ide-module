/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_compatibility.c
 *
 * PURPOSE:
 *   Implement the quality compatibility behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | API compatibility inspection | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/quality_compatibility.h"
/*
 * Provide the studio quality compare api operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_compare_api(UmiStudioQualityCentre *centre,const UmiCodeGuardApiBaseline *expected,const UmiCodeGuardApiBaseline *current)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_codeguard_api_baseline_compare(expected,current,&centre->session.api);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->session.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Find studio quality api change while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiCodeGuardApiChange *umi_studio_quality_api_change_at(const UmiStudioQualityCentre *centre,size_t index)
{
    return centre != NULL && index < centre->session.api.count ? &centre->session.api.changes[index] : NULL;
}
