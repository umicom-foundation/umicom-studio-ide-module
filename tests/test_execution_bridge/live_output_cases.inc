/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_execution_bridge/live_output_cases.inc
 * PURPOSE: Check Studio output ownership across accepted jobs and service disposal.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

typedef struct OutputWrongThread
{
    UmiStudioTestService *service;
    UmiStatus result;
} OutputWrongThread;
static int ReadOutputOnWrongThread(void *data)
{
    OutputWrongThread *request = data;
    UmiCtestOutputSnapshot *output = calloc(1U, sizeof(*output));
    CHECK(output != NULL);
    request->result = UmiStudioTestRunReadOutput(request->service, output);
    free(output);
    return 0;
}
static void LiveOutputCase(const char *mode, UmiStudioTestService *service, UmiTaskQueue *queue,
                           UmiStudioTestRunContext *context, UmiTestPlatformOperationPlan *plan)
{
    UmiCtestOutputSnapshot *output = calloc(1U, sizeof(*output));
    CHECK(output != NULL);
    output->task_id = 123U;
    CHECK(UmiStudioTestRunReadOutput(service, output) == UMI_STATUS_NOT_FOUND && output->task_id == 123U);
    CHECK(UmiStudioTestRunReadOutput(NULL, output) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioTestRunReadOutput(service, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    if (strcmp(mode, "output-empty") == 0)
    {
        free(output);
        return;
    }
    CHECK(Start(service, queue, context, plan) == UMI_STATUS_OK);
    if (strcmp(mode, "output-wrong-thread") == 0)
    {
        OutputWrongThread request = {service, UMI_STATUS_OK};
        UmiThread *thread = NULL;
        int code = -1;
        CHECK(umi_thread_start(ReadOutputOnWrongThread, &request, &thread) == UMI_STATUS_OK);
        CHECK(umi_thread_join(thread, &code) == UMI_STATUS_OK);
        umi_thread_destroy(thread);
        CHECK(code == 0 && request.result == UMI_STATUS_INVALID_STATE);
    }
    char message[512];
    CHECK(Finish(service, context, message) == UMI_STATUS_OK);
    CHECK(UmiStudioTestRunReadOutput(service, output) == UMI_STATUS_OK);
    CHECK(output->task_id != 0U && output->attempt_complete && output->result_state == UMI_TEST_STATE_PASSED);
    CHECK(strcmp(output->name, "notes.pass") == 0 && strstr(output->bytes, "pass") != NULL);
    uint64_t identity = output->task_id;
    if (strcmp(mode, "output-next-run") == 0)
    {
        CHECK(Start(service, queue, context, plan) == UMI_STATUS_OK);
        CHECK(Finish(service, context, message) == UMI_STATUS_OK);
        CHECK(UmiStudioTestRunReadOutput(service, output) == UMI_STATUS_OK);
        CHECK(output->task_id > identity && output->invocation == 1U && output->attempt == 1U);
    }
    if (strcmp(mode, "output-copy") == 0)
    {
        CHECK(UmiStudioExecutionDispose(service));
        CHECK(output->task_id == identity && strcmp(output->name, "notes.pass") == 0);
        CHECK(UmiStudioTestRunReadOutput(service, output) == UMI_STATUS_NOT_FOUND);
        CHECK(output->task_id == identity);
    }
    free(output);
}
