/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/close_plan.h
 *
 * PURPOSE:
 *   Define Studio's bounded close orchestration for product-owned background
 *   work. Framework remains the authority for documents, build execution,
 *   tests and debugger services; this contract only decides which existing
 *   Studio-owned operations must finish before a window can close.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_CLOSE_PLAN_H
#define UMICOM_STUDIO_CLOSE_PLAN_H

#include <stdint.h>
#include "umicom/base/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum UmiStudioClosePlanDecision {
    UMI_STUDIO_CLOSE_PLAN_KEEP_OPEN_ERROR = 0,
    UMI_STUDIO_CLOSE_PLAN_STOP_OWNED_WORK = 1,
    UMI_STUDIO_CLOSE_PLAN_CONFIRM_DISCARD = 2,
    UMI_STUDIO_CLOSE_PLAN_ALLOW = 3
} UmiStudioClosePlanDecision;

typedef enum UmiStudioClosePlanAction {
    UMI_STUDIO_CLOSE_PLAN_ACTION_NONE = 0,
    UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_TEST_RUN = 1u << 0,
    UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_TEST_DISCOVERY = 1u << 1,
    UMI_STUDIO_CLOSE_PLAN_ACTION_CANCEL_BUILD = 1u << 2,
    UMI_STUDIO_CLOSE_PLAN_ACTION_STOP_DEBUGGER = 1u << 3
} UmiStudioClosePlanAction;

typedef struct UmiStudioClosePlanInput {
    int services_ready;
    int snapshot_ready;
    int test_run_pending;
    int test_discovery_pending;
    int build_busy;
    int debugger_busy;
    int dirty_documents;
    int discard_confirmed;
} UmiStudioClosePlanInput;

typedef struct UmiStudioClosePlan {
    UmiStudioClosePlanDecision decision;
    uint32_t actions;
    int requires_second_close;
} UmiStudioClosePlan;

void UmiStudioClosePlanInputInit(UmiStudioClosePlanInput *input);
void UmiStudioClosePlanInit(UmiStudioClosePlan *plan);

UmiStatus UmiStudioClosePlanEvaluate(
    const UmiStudioClosePlanInput *input,
    UmiStudioClosePlan *out_plan);

const char *UmiStudioClosePlanDecisionText(UmiStudioClosePlanDecision decision);

#ifdef __cplusplus
}
#endif
#endif
