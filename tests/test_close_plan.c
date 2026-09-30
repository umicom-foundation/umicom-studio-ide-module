/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_close_plan.c
 *
 * PURPOSE:
 *   Verify STU-02 close orchestration and output-preservation behaviour.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/close_plan.h"

#include <stdio.h>

#define CHECK(X) do { if (!(X)) { \
    fprintf(stderr, "check failed at line %d: %s\n", __LINE__, #X); return 1; \
} } while (0)

int main(void)
{
    UmiStudioClosePlanInput input;
    UmiStudioClosePlan plan = {
        UMI_STUDIO_CLOSE_PLAN_ALLOW, UINT32_C(77), 1
    };

    UmiStudioClosePlanInputInit(&input);
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) == UMI_STATUS_OK);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_ALLOW);
    CHECK(plan.actions == 0U);

    input.test_run_pending = 1;
    input.test_discovery_pending = 1;
    input.build_busy = 1;
    input.debugger_busy = 1;
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) == UMI_STATUS_OK);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_STOP_OWNED_WORK);
    CHECK(plan.actions ==
        (UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_TEST_RUN |
         UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_TEST_DISCOVERY |
         UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_BUILD |
         UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_DEBUGGER));
    CHECK(plan.requires_second_close == 1);

    UmiStudioClosePlanInputInit(&input);
    input.dirty_documents = 1;
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) == UMI_STATUS_OK);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_CONFIRM_DISCARD);

    input.discard_confirmed = 1;
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) == UMI_STATUS_OK);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_ALLOW);

    UmiStudioClosePlanInputInit(&input);
    input.services_ready = 0;
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) == UMI_STATUS_OK);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_KEEP_OPEN_ERROR);

    UmiStudioClosePlanInputInit(&input);
    input.build_busy = 2;
    plan.decision = UMI_STUDIO_CLOSE_PLAN_ALLOW;
    plan.actions = UINT32_C(77);
    plan.requires_second_close = 1;
    CHECK(UmiStudioClosePlanEvaluate(&input, &plan) ==
        UMI_STATUS_INVALID_ARGUMENT);
    CHECK(plan.decision == UMI_STUDIO_CLOSE_PLAN_ALLOW);
    CHECK(plan.actions == UINT32_C(77));
    CHECK(plan.requires_second_close == 1);

    CHECK(UmiStudioClosePlanEvaluate(NULL, &plan) ==
        UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioClosePlanEvaluate(&input, NULL) ==
        UMI_STATUS_INVALID_ARGUMENT);

    puts("Studio close-plan tests passed.");
    return 0;
}
