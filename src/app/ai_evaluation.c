/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/ai_evaluation.c
 *
 * PURPOSE:
 *   Calculate and report a simple Studio AI quality score.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio records evaluation evidence as Framework data so Helix can consume the same result without parsing UI text.
 */

#include "umicom/studio/ai_evaluation.h"

#include <stdio.h>

/*
 * Provide the studio ai evaluation operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_evaluation(double score,
                                   double threshold,
                                   UmiAiEvaluation *evaluation)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (evaluation == NULL || score < 0.0 || score > 1.0 ||
        threshold < 0.0 || threshold > 1.0) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)umi_ai_evaluation_init(evaluation);
    (void)snprintf(evaluation->evaluation_id,
                   sizeof(evaluation->evaluation_id), "%s", "studio-evaluation");
    (void)snprintf(evaluation->metric,
                   sizeof(evaluation->metric), "%s", "quality");
    evaluation->score = score;
    evaluation->threshold = threshold;
    evaluation->passed = score >= threshold;
    return UMI_STATUS_OK;
}
