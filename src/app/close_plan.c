/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/close_plan.c
 *
 * PURPOSE:
 *   Implement Studio's deterministic close orchestration. This product layer
 *   never replaces Framework service ownership: it only orders cancellation,
 *   confirmation and final window close.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/close_plan.h"

#include <string.h>

static int ValidBoolean(int value)
{
    return value == 0 || value == 1;
}

void UmiStudioClosePlanInputInit(UmiStudioClosePlanInput *input)
{
    if (input != NULL) {
        (void)memset(input, 0, sizeof(*input));
        input->services_ready = 1;
        input->snapshot_ready = 1;
    }
}

void UmiStudioClosePlanInit(UmiStudioClosePlan *plan)
{
    if (plan != NULL) {
        (void)memset(plan, 0, sizeof(*plan));
        plan->decision = UMI_STUDIO_CLOSE_PLAN_KEEP_OPEN_ERROR;
    }
}

UmiStatus UmiStudioClosePlanEvaluate(
    const UmiStudioClosePlanInput *input,
    UmiStudioClosePlan *out_plan)
{
    UmiStudioClosePlan candidate;
    uint32_t actions = UMI_STUDIO_CLOSE_PLAN_ACTION_NONE;

    if (input == NULL || out_plan == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!ValidBoolean(input->services_ready) ||
        !ValidBoolean(input->snapshot_ready) ||
        !ValidBoolean(input->test_run_pending) ||
        !ValidBoolean(input->test_discovery_pending) ||
        !ValidBoolean(input->build_busy) ||
        !ValidBoolean(input->debugger_busy) ||
        !ValidBoolean(input->dirty_documents) ||
        !ValidBoolean(input->discard_confirmed))
        return UMI_STATUS_INVALID_ARGUMENT;

    UmiStudioClosePlanInit(&candidate);

    if (!input->services_ready || !input->snapshot_ready) {
        candidate.decision = UMI_STUDIO_CLOSE_PLAN_KEEP_OPEN_ERROR;
        *out_plan = candidate;
        return UMI_STATUS_OK;
    }

    if (input->test_run_pending)
        actions |= UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_TEST_RUN;
    if (input->test_discovery_pending)
        actions |= UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_TEST_DISCOVERY;
    if (input->build_busy)
        actions |= UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_BUILD;
    if (input->debugger_busy)
        actions |= UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_DEBUGGER;

    if (actions != UMI_STUDIO_CLOSE_PLAN_ACTION_NONE) {
        candidate.decision = UMI_STUDIO_CLOSE_PLAN_STOP_OWNED_WORK;
        candidate.actions = actions;
        candidate.requires_second_close = 1;
        *out_plan = candidate;
        return UMI_STATUS_OK;
    }

    if (input->dirty_documents && !input->discard_confirmed) {
        candidate.decision = UMI_STUDIO_CLOSE_PLAN_CONFIRM_DISCARD;
        *out_plan = candidate;
        return UMI_STATUS_OK;
    }

    candidate.decision = UMI_STUDIO_CLOSE_PLAN_ALLOW;
    *out_plan = candidate;
    return UMI_STATUS_OK;
}

const char *UmiStudioClosePlanDecisionText(UmiStudioClosePlanDecision decision)
{
    switch (decision) {
        case UMI_STUDIO_CLOSE_PLAN_KEEP_OPEN_ERROR: return "keep-open-error";
        case UMI_STUDIO_CLOSE_PLAN_STOP_OWNED_WORK: return "stop-owned-work";
        case UMI_STUDIO_CLOSE_PLAN_CONFIRM_DISCARD: return "confirm-discard";
        case UMI_STUDIO_CLOSE_PLAN_ALLOW: return "allow-close";
        default: return "invalid";
    }
}
