/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_archive_job.h
 * PURPOSE: Produce an accepted disabled-test run using the real Studio and Framework services.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_TEST_ARCHIVE_JOB_H
#define UMICOM_STUDIO_TEST_ARCHIVE_JOB_H
#include "umicom/studio/tests.h"
#include "umicom/studio/test_execution.h"
#include "umicom/studio/test_archive.h"
#include <stdatomic.h>
#include "umicom/test_platform/ctest.h"
#include "umicom/platform/threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Disabled catalogue entries exercise task ownership, result publication and
 * storage without starting CTest, compiling a project, or invoking a broker. */
static inline UmiStatus ArchiveDisabledRun(UmiStudioTestService *service, UmiTaskQueue *queue)
{
    UmiTestPlatformService *platform = umi_studio_test_service_platform(service);
    UmiTestPlatformCtestImportOptions options = {0};
    strcpy(options.project_id, "archive");
    strcpy(options.suite_id, "archive.ctest");
    strcpy(options.build_directory, "/archive-fixture/build");
    strcpy(options.configuration, "Debug");
    UmiTestPlatformCtestImportSummary summary = {0};
    UmiStatus status = umi_test_platform_ctest_parse_json(
        "{\"tests\":[{\"name\":\"archive.disabled\",\"properties\":[{\"name\":\"DISABLED\",\"value\":true}]}]"
        "}",
        &options, umi_test_platform_service_item(platform), umi_test_platform_service_suite(platform),
        umi_test_platform_service_discovery(platform), &summary);
    UmiTestPlatformOperationPlan *plan = calloc(1, sizeof(*plan));
    if (plan == NULL)
        return UMI_STATUS_OUT_OF_MEMORY;
    umi_test_platform_operation_plan_init(plan, UMI_TEST_PLATFORM_OPERATION_RUN_SELECTED);
    UmiTestPlatformItemSnapshot item = {0};
    if (status == UMI_STATUS_OK)
        status = umi_test_platform_item_registry_at(umi_test_platform_service_item(platform), 0, &item);
    if (status == UMI_STATUS_OK)
    {
        strcpy(plan->selection.item_ids[0], item.id);
        plan->selection.count = 1;
        UmiStudioTestRunContext context = {0};
        context.trusted = 1;
        context.workspace_generation = 17;
        strcpy(context.source_root, "/archive-fixture/source");
        strcpy(context.build_root, options.build_directory);
        strcpy(context.configuration, options.configuration);
        status = UmiStudioTestRunArm(service, queue, &context);
        UmiTestPlatformExecutionSummary execution = {0};
        if (status == UMI_STATUS_OK)
            status = umi_studio_test_service_execute(service, plan, &execution);
        UmiStudioTestRunDisarm(service);
        if (status == UMI_STATUS_OK)
        {
            char message[512];
            for (unsigned i = 0; i < 5000U; ++i)
            {
                status = UmiStudioTestRunPoll(service, &context, message, sizeof(message));
                if (status != UMI_STATUS_BUSY)
                    break;
                umi_thread_sleep_ms(1);
            }
        }
    }
    free(plan);
    return status;
}
static inline UmiStatus ArchiveAwaitSave(UmiStudioTestService *service, UmiStudioTestArchiveState *state)
{
    for (unsigned i = 0; i < 5000U; ++i)
    {
        UmiStatus status = UmiStudioTestArchiveStateRead(service, state);
        if (status != UMI_STATUS_OK)
            return status;
        if (!state->pending)
            return state->write.status;
        umi_thread_sleep_ms(1);
    }
    return UMI_STATUS_TIMEOUT;
}

/* Share deterministic worker control across service and native archive tests.
 * The blocking task holds the sole worker before comparison is submitted. */
typedef struct ArchiveComparisonGate
{
    atomic_int entered, released;
} ArchiveComparisonGate;
static inline UmiStatus ArchiveComparisonWaitGate(UmiTaskContext *context, void *data)
{
    (void)context;
    ArchiveComparisonGate *gate = data;
    atomic_store(&gate->entered, 1);
    while (!atomic_load(&gate->released))
        umi_thread_sleep_ms(1U);
    return UMI_STATUS_OK;
}
static inline UmiStatus ArchiveAwaitComparison(UmiStudioTestService *service,
                                               UmiStudioTestArchiveState *state)
{
    for (unsigned i = 0; i < 5000U; ++i)
    {
        UmiStatus status = UmiStudioTestArchiveStateRead(service, state);
        if (status != UMI_STATUS_OK)
            return status;
        if (state->comparison.state == UMI_TASK_SUCCEEDED || state->comparison.state == UMI_TASK_FAILED ||
            state->comparison.state == UMI_TASK_CANCELLED)
            return state->comparison.status;
        umi_thread_sleep_ms(1U);
    }
    return UMI_STATUS_TIMEOUT;
}
#endif
