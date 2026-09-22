/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/test_execution.h
 * PURPOSE: Connect Studio Test Explorer to Framework's queued CTest execution.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TEST_EXECUTION_H
#define UMICOM_STUDIO_TEST_EXECUTION_H
#include "umicom/testing/ctest_job.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioTestService UmiStudioTestService;

/** Copy the currently trusted workspace and active build profile. The GUI
 * supplies this context both when starting and when polling a run. Selected
 * tests must belong to this build root and configuration. */
typedef struct UmiStudioTestRunContext {
    uint64_t workspace_generation;
    int trusted;
    char source_root[1024];
    char build_root[1024];
    char configuration[128];
} UmiStudioTestRunContext;

/** Arm one existing execution-command dispatch; Disarm after dispatch on every
 * path. Direct API/CLI callers that do not arm retain synchronous execution.
 * All service bridge methods belong to the UI/service owner thread. */
UmiStatus UmiStudioTestRunArm(UmiStudioTestService *service, UmiTaskQueue *queue,
    const UmiStudioTestRunContext *context);
void UmiStudioTestRunDisarm(UmiStudioTestService *service);
int UmiStudioTestRunPending(const UmiStudioTestService *service);
UmiStatus UmiStudioTestRunCancel(UmiStudioTestService *service);

/** Poll without waiting; publish completed rows on the service owner thread.
 * BUSY means execution or result publication is still in progress. A changed trust/profile/
 * catalogue/history fence requests Stop and prevents further publication.
 * Completed task results remain available until the next accepted run or
 * service destruction. Cancelled attempts are never a pass. Publish at most
 * 16 completed rows per call; continue polling even after the worker finishes
 * while rows still await publication. */
UmiStatus UmiStudioTestRunPoll(UmiStudioTestService *service,
    const UmiStudioTestRunContext *current, char *message, size_t capacity);
UmiStatus UmiStudioTestRunSnapshot(UmiStudioTestService *service,
    UmiCtestJobSnapshot *outSnapshot);
UmiStatus UmiStudioTestRunResultAt(UmiStudioTestService *service, size_t index,
    UmiTestResult *outResult, uint32_t *outAttempt);
#ifdef __cplusplus
}
#endif
#endif
