/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/delivery_update.c
 *
 * PURPOSE:
 *   Plan Studio updates and rollbacks using Framework generation and compatibility rules.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio can explain an update decision without owning the generic comparison or rollback algorithm.
 */

#include "umicom/studio/delivery_update.h"

/*
 * Provide the studio update plan operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_update_plan(uint64_t current_generation,
                                 uint64_t available_generation,
                                 int compatible,
                                 int policy_allows,
                                 UmiUpdatePlan *out_plan)
{
    UmiUpdateDecision decision;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_plan == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    decision = umi_update_decide(current_generation,
                                 available_generation,
                                 compatible,
                                 policy_allows);
    return umi_update_plan_init(out_plan,
                                current_generation,
                                available_generation,
                                decision,
                                decision == UMI_UPDATE_AVAILABLE
                                    ? "A newer compatible Studio generation is available"
                                    : "No eligible Studio update is available");
}

/*
 * Provide the studio rollback plan operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_rollback_plan(uint64_t current_generation,
                                   uint64_t target_generation,
                                   const char *reason,
                                   UmiRollbackPlan *out_plan)
{
    UmiStatus status = umi_rollback_plan_init(out_plan,
                                              current_generation,
                                              target_generation,
                                              reason);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_rollback_plan_approve(out_plan);
}
