/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_gates.c
 *
 * PURPOSE:
 *   Implement the quality gates behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/quality_gates.h"
/*
 * Provide the studio quality set policy operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_set_policy(UmiStudioQualityCentre *centre,const UmiCodeGuardQualityPolicy *policy)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || policy == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    centre->session.policy = *policy;
    centre->session.revision += 1U;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio quality evaluate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_evaluate(UmiStudioQualityCentre *centre,UmiCodeGuardQualityEvaluation *out_evaluation)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_evaluation == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_codeguard_quality_session_evaluate(&centre->session);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { *out_evaluation = centre->session.evaluation; centre->revision += 1U; }
    return status;
}
