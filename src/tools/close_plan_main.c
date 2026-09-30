/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/close_plan_main.c
 *
 * PURPOSE:
 *   Explain the deterministic STU-02 close plan without starting GTK.
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
#include <stdlib.h>
#include <string.h>

static int PrintPlan(unsigned mask)
{
    UmiStudioClosePlanInput input;
    UmiStudioClosePlan plan;
    UmiStatus status;

    UmiStudioClosePlanInputInit(&input);
    input.services_ready = (mask & 1u) != 0u;
    input.snapshot_ready = (mask & 2u) != 0u;
    input.test_run_pending = (mask & 4u) != 0u;
    input.test_discovery_pending = (mask & 8u) != 0u;
    input.build_busy = (mask & 16u) != 0u;
    input.debugger_busy = (mask & 32u) != 0u;
    input.dirty_documents = (mask & 64u) != 0u;
    input.discard_confirmed = (mask & 128u) != 0u;

    status = UmiStudioClosePlanEvaluate(&input, &plan);
    if (status != UMI_STATUS_OK) return 2;

    printf("mask=%u decision=%s actions=%u second-close=%d\n",
        mask, UmiStudioClosePlanDecisionText(plan.decision),
        (unsigned)plan.actions, plan.requires_second_close);
    return 0;
}

static int SelfTest(void)
{
    UmiStudioClosePlanInput input;
    UmiStudioClosePlan plan;

    UmiStudioClosePlanInputInit(&input);
    if (UmiStudioClosePlanEvaluate(&input, &plan) != UMI_STATUS_OK ||
        plan.decision != UMI_STUDIO_CLOSE_PLAN_ALLOW) return 10;

    input.build_busy = 1;
    if (UmiStudioClosePlanEvaluate(&input, &plan) != UMI_STATUS_OK ||
        plan.decision != UMI_STUDIO_CLOSE_PLAN_STOP_OWNED_WORK ||
        (plan.actions & UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_BUILD) == 0u)
        return 11;

    input.build_busy = 0;
    input.dirty_documents = 1;
    if (UmiStudioClosePlanEvaluate(&input, &plan) != UMI_STATUS_OK ||
        plan.decision != UMI_STUDIO_CLOSE_PLAN_CONFIRM_DISCARD) return 12;

    input.discard_confirmed = 1;
    if (UmiStudioClosePlanEvaluate(&input, &plan) != UMI_STATUS_OK ||
        plan.decision != UMI_STUDIO_CLOSE_PLAN_ALLOW) return 13;

    puts("Studio close-plan self-test passed.");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) return SelfTest();
    if (argc == 3 && strcmp(argv[1], "mask") == 0) {
        char *end = NULL;
        unsigned long value = strtoul(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || value > 255UL) return 2;
        return PrintPlan((unsigned)value);
    }

    puts("Umicom Studio close-plan inspector");
    puts("Usage:");
    puts("  umicom-studio-close-plan --self-test");
    puts("  umicom-studio-close-plan mask 255");
    return argc == 1 ? 0 : 2;
}
