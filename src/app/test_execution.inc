/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/test_execution.inc
 * PURPOSE: Publish copied CTest job results through the existing Test Platform.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static int StudioRunContextValid(const UmiStudioTestRunContext *context)
{
    return context != NULL && context->trusted &&
        memchr(context->source_root, '\0', sizeof(context->source_root)) != NULL &&
        memchr(context->build_root, '\0', sizeof(context->build_root)) != NULL &&
        memchr(context->configuration, '\0', sizeof(context->configuration)) != NULL &&
        context->source_root[0] != '\0' && context->build_root[0] != '\0';
}

static int StudioRunContextEqual(const UmiStudioTestRunContext *a,
    const UmiStudioTestRunContext *b)
{
    return StudioRunContextValid(b) && a->workspace_generation == b->workspace_generation &&
        strcmp(a->source_root, b->source_root) == 0 && strcmp(a->build_root, b->build_root) == 0 &&
        strcmp(a->configuration[0] ? a->configuration : "Debug",
               b->configuration[0] ? b->configuration : "Debug") == 0;
}

UmiStatus UmiStudioTestRunArm(UmiStudioTestService *service, UmiTaskQueue *queue,
    const UmiStudioTestRunContext *context)
{
    if (service == NULL || queue == NULL || context == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!StudioRunContextValid(context)) return UMI_STATUS_PERMISSION_DENIED;
    if (UmiStudioTestDiscoveryPending(service) || service->discoveryQueue != NULL ||
        service->executionPending || service->executionQueue != NULL) return UMI_STATUS_BUSY;
    if (service->executionJob != NULL && service->executionOwner != umi_thread_current_id())
        return UMI_STATUS_INVALID_STATE;
    service->executionQueue = queue;
    service->executionContext = *context;
    service->executionOwner = umi_thread_current_id();
    return UMI_STATUS_OK;
}

void UmiStudioTestRunDisarm(UmiStudioTestService *service)
{
    if (service != NULL && service->executionOwner == umi_thread_current_id())
        service->executionQueue = NULL;
}

int UmiStudioTestRunPending(const UmiStudioTestService *service)
{
    return service != NULL && service->executionPending;
}

UmiStatus UmiStudioTestRunCancel(UmiStudioTestService *service)
{
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!service->executionPending) return UMI_STATUS_NOT_FOUND;
    if (service->executionOwner != umi_thread_current_id()) return UMI_STATUS_INVALID_STATE;
    (void)umi_test_platform_service_request_stop(service->platform);
    return UmiCtestJobCancel(service->executionJob);
}

/* Check capacity for the entire plan before a child can run. Existing history
 * is never silently cleared to make room. Identifiers stay unique across runs. */
static UmiStatus UmiStudioExecutionStart(UmiStudioTestService *service,
    const UmiTestPlatformOperationPlan *plan, UmiTestPlatformExecutionSummary *outSummary)
{
    UmiTaskQueue *queue = service->executionQueue;
    service->executionQueue = NULL;
    if (service->executionOwner != umi_thread_current_id()) return UMI_STATUS_INVALID_STATE;
    if (queue == NULL || service->executionPending || UmiStudioTestDiscoveryPending(service))
        return UMI_STATUS_BUSY;
    if (plan->kind < UMI_TEST_PLATFORM_OPERATION_RUN_ALL ||
        plan->kind > UMI_TEST_PLATFORM_OPERATION_REPEAT) return UMI_STATUS_INVALID_ARGUMENT;
    if (plan->selection.source_revision != 0U && plan->selection.source_revision !=
        umi_test_platform_item_registry_revision(umi_test_platform_service_item(service->platform)))
        return UMI_STATUS_INVALID_STATE;
    const size_t count = plan->selection.count;
    const uint32_t repeats = plan->repeat_count ? plan->repeat_count : 1U;
    if (count == 0U) return UMI_STATUS_INVALID_ARGUMENT;
    if (count > sizeof(plan->selection.item_ids) / sizeof(plan->selection.item_ids[0]) ||
        count > UMI_CTEST_JOB_MAX_ATTEMPTS || repeats > UMI_CTEST_JOB_MAX_ATTEMPTS / count)
        return UMI_STATUS_CAPACITY_EXCEEDED;
    const size_t total = count * (size_t)repeats;
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(service->platform);
    UmiTestPlatformOutputRegistry *outputs = umi_test_platform_service_output(service->platform);
    if (total > UMI_TEST_PLATFORM_RESULT_CAPACITY - umi_test_platform_result_registry_count(results) ||
        total > UMI_TEST_PLATFORM_OUTPUT_CAPACITY - umi_test_platform_output_registry_count(outputs))
        return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiCtestJobRequest *requests = calloc(count, sizeof(*requests));
    if (requests == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = UMI_STATUS_OK;
    for (size_t i = 0U; i < count && status == UMI_STATUS_OK; ++i) {
        if (memchr(plan->selection.item_ids[i], '\0', sizeof(plan->selection.item_ids[i])) == NULL) {
            status = UMI_STATUS_INVALID_ARGUMENT; break;
        }
        UmiTestPlatformItemSnapshot item;
        status = umi_test_platform_item_registry_find(
            umi_test_platform_service_item(service->platform), plan->selection.item_ids[i], &item);
        if (status == UMI_STATUS_OK)
            status = UmiTestPlatformCtestMakeRunRequest(&item,
                umi_test_platform_service_discovery(service->platform), &requests[i]);
        if (status == UMI_STATUS_OK &&
            (strcmp(requests[i].build_directory, service->executionContext.build_root) != 0 ||
             strcmp(requests[i].configuration, service->executionContext.configuration[0]
                ? service->executionContext.configuration : "Debug") != 0))
            status = UMI_STATUS_INVALID_STATE;
    }
    uint64_t sequence = umi_test_platform_result_registry_revision(results);
    for (size_t i = 0U; status == UMI_STATUS_OK && i < umi_test_platform_result_registry_count(results); ++i) {
        UmiTestPlatformResultSnapshot result;
        status = umi_test_platform_result_registry_at(results, i, &result);
        if (status == UMI_STATUS_OK && result.sequence > sequence) sequence = result.sequence;
    }
    if (status == UMI_STATUS_OK && (uint64_t)total > UINT64_MAX - sequence)
        status = UMI_STATUS_CAPACITY_EXCEEDED;
    for (size_t i = 0U; status == UMI_STATUS_OK && i < total; ++i) {
        UmiTestPlatformResultSnapshot result;
        UmiTestPlatformOutputSnapshot output;
        char identity[128];
        (void)snprintf(identity, sizeof(identity), "result.job.%llu",
            (unsigned long long)(sequence + (uint64_t)i + 1U));
        if (umi_test_platform_result_registry_find(results, identity, &result) != UMI_STATUS_NOT_FOUND)
            status = UMI_STATUS_INVALID_STATE;
        (void)snprintf(identity, sizeof(identity), "output.job.%llu",
            (unsigned long long)(sequence + (uint64_t)i + 1U));
        if (umi_test_platform_output_registry_find(outputs, identity, &output) != UMI_STATUS_NOT_FOUND)
            status = UMI_STATUS_INVALID_STATE;
    }
    UmiCtestJob *job = NULL;
    UmiCtestJobOptions options = {repeats, plan->stop_on_failure};
    if (status == UMI_STATUS_OK) status = UmiCtestJobCreate(requests, count, &options, &job);
    free(requests);
    int began = 0;
    if (status == UMI_STATUS_OK) {
        status = umi_test_platform_service_begin_operation(service->platform, plan);
        began = status == UMI_STATUS_OK;
    }
    if (status == UMI_STATUS_OK) status = UmiCtestJobSubmit(job, queue);
    if (status != UMI_STATUS_OK) {
        if (began) umi_test_platform_service_finish_operation(service->platform);
        (void)UmiCtestJobDestroy(job);
        return status;
    }
    /* The previous terminal job remains available until another run is accepted. */
    (void)UmiCtestJobDestroy(service->executionJob);
    service->executionJob = job;
    service->executionPending = 1;
    service->executionStale = 0;
    service->executionPublished = 0U;
    service->executionSequence = sequence;
    service->executionItemsRevision = umi_test_platform_item_registry_revision(
        umi_test_platform_service_item(service->platform));
    service->executionDiscoveryRevision = umi_test_platform_discovery_registry_revision(
        umi_test_platform_service_discovery(service->platform));
    service->executionResultsRevision = umi_test_platform_result_registry_revision(results);
    service->executionOutputRevision = umi_test_platform_output_registry_revision(outputs);
    memset(&service->last_summary, 0, sizeof(service->last_summary));
    memset(outSummary, 0, sizeof(*outSummary)); outSummary->planned = total;
    ++service->explorer.revision;
    return UMI_STATUS_OK;
}

/* The compact UI keeps an explicitly labelled tail. The complete adapter
 * capture remains in the job and can be copied through TestRunResultAt. */
static void StudioRunOutputTail(char *output, size_t capacity, const char *text)
{
    static const char prefix[] = "[Earlier output omitted from this compact view]\n";
    size_t length = strlen(text);
    if (length < capacity) { memcpy(output, text, length + 1U); return; }
    if (capacity <= sizeof(prefix)) { copy_text(output, capacity, "Output exceeds view capacity."); return; }
    size_t room = capacity - sizeof(prefix);
    const char *tail = text + length - room;
    /* Do not begin inside a UTF-8 continuation byte. */
    while (*tail && (((unsigned char)*tail & 0xc0U) == 0x80U)) ++tail;
    memcpy(output, prefix, sizeof(prefix) - 1U);
    memcpy(output + sizeof(prefix) - 1U, tail, strlen(tail) + 1U);
}

static UmiStatus StudioRunPublishRow(UmiStudioTestService *service, const UmiTestResult *native)
{
    UmiTestPlatformResultSnapshot result = {0};
    UmiTestPlatformOutputSnapshot output = {0};
    if (memchr(native->test_id, '\0', sizeof(native->test_id)) == NULL ||
        strlen(native->test_id) >= sizeof(result.item_id)) return UMI_STATUS_CAPACITY_EXCEEDED;
    result.sequence = service->executionSequence + (uint64_t)service->executionPublished + 1U;
    (void)snprintf(result.id, sizeof(result.id), "result.job.%llu", (unsigned long long)result.sequence);
    (void)snprintf(output.id, sizeof(output.id), "output.job.%llu", (unsigned long long)result.sequence);
    copy_text(result.item_id, sizeof(result.item_id), native->test_id);
    copy_text(output.item_id, sizeof(output.item_id), native->test_id);
    copy_text(output.stream, sizeof(output.stream), "combined");
    result.outcome = platform_outcome(native->state);
    result.exit_code = native->exit_code;
    result.duration_ms = (double)native->duration_ms;
    copy_text(result.message, sizeof(result.message), native->state == UMI_TEST_STATE_PASSED ?
        "Test passed." : native->state == UMI_TEST_STATE_SKIPPED ? "Test skipped; it did not pass." :
        native->state == UMI_TEST_STATE_CANCELLED ? "Test cancelled." :
        native->state == UMI_TEST_STATE_TIMED_OUT ? "Test timed out." :
        native->state == UMI_TEST_STATE_NOT_RUN ? "Test was not run; inspect its output." : "Test failed.");
    StudioRunOutputTail(result.failure_details, sizeof(result.failure_details), native->output);
    StudioRunOutputTail(output.text, sizeof(output.text), native->output);
    UmiTestPlatformOutputRegistry *outputs = umi_test_platform_service_output(service->platform);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(service->platform);
    UmiStatus status = umi_test_platform_output_registry_upsert(outputs, &output);
    if (status == UMI_STATUS_OK) {
        status = umi_test_platform_result_registry_upsert(results, &result);
        if (status != UMI_STATUS_OK)
            (void)umi_test_platform_output_registry_remove(outputs, output.id);
    }
    if (status != UMI_STATUS_OK) return status;
    service->executionResultsRevision = umi_test_platform_result_registry_revision(results);
    service->executionOutputRevision = umi_test_platform_output_registry_revision(outputs);
    ++service->executionPublished;
    ++service->last_summary.total;
    switch (native->state) {
        case UMI_TEST_STATE_PASSED: ++service->last_summary.passed; break;
        case UMI_TEST_STATE_SKIPPED: ++service->last_summary.skipped; break;
        case UMI_TEST_STATE_CANCELLED: ++service->last_summary.cancelled; break;
        case UMI_TEST_STATE_TIMED_OUT: ++service->last_summary.timed_out; break;
        default: ++service->last_summary.failed; break;
    }
    if (native->duration_ms <= UINT64_MAX - service->last_summary.duration_ms)
        service->last_summary.duration_ms += native->duration_ms;
    else service->last_summary.duration_ms = UINT64_MAX;
    umi_test_platform_operation_mark_completed(umi_test_platform_service_operation(service->platform));
    ++service->explorer.revision;
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioTestRunPoll(UmiStudioTestService *service,
    const UmiStudioTestRunContext *current, char *message, size_t capacity)
{
    if (message != NULL && capacity != 0U) message[0] = '\0';
    if (service == NULL || (message == NULL && capacity != 0U)) return UMI_STATUS_INVALID_ARGUMENT;
    if (!service->executionPending) return UMI_STATUS_NOT_FOUND;
    if (service->executionOwner != umi_thread_current_id()) return UMI_STATUS_INVALID_STATE;
    if (!StudioRunContextEqual(&service->executionContext, current) ||
        service->executionItemsRevision != umi_test_platform_item_registry_revision(
            umi_test_platform_service_item(service->platform)) ||
        service->executionDiscoveryRevision != umi_test_platform_discovery_registry_revision(
            umi_test_platform_service_discovery(service->platform)) ||
        service->executionResultsRevision != umi_test_platform_result_registry_revision(
            umi_test_platform_service_result(service->platform)) ||
        service->executionOutputRevision != umi_test_platform_output_registry_revision(
            umi_test_platform_service_output(service->platform))) {
        service->executionStale = 1;
        (void)UmiStudioTestRunCancel(service);
    }
    UmiCtestJobSnapshot progress;
    UmiStatus status = UmiCtestJobGetSnapshot(service->executionJob, &progress);
    if (status != UMI_STATUS_OK) return status;
    /* Publish a bounded group per UI tick, including after worker completion. */
    size_t publishedThisPoll = 0U;
    while (!service->executionStale && service->executionPublished < progress.completed &&
            publishedThisPoll < 16U) {
        UmiTestResult result;
        status = UmiCtestJobResultAt(service->executionJob, service->executionPublished, &result, NULL);
        if (status == UMI_STATUS_OK) status = StudioRunPublishRow(service, &result);
        if (status == UMI_STATUS_OK) ++publishedThisPoll;
        if (status != UMI_STATUS_OK) {
            service->executionStale = 1;
            (void)UmiStudioTestRunCancel(service);
            break;
        }
    }
    if (!service->executionStale && service->executionPublished < progress.completed)
        return UMI_STATUS_BUSY;
    if (progress.state == UMI_TASK_CREATED || progress.state == UMI_TASK_QUEUED ||
        progress.state == UMI_TASK_RUNNING) return UMI_STATUS_BUSY;
    service->executionPending = 0;
    umi_test_platform_service_finish_operation(service->platform);
    ++service->explorer.revision;
    (void)umi_test_workspace_refresh(service->workspace);
    if (message != NULL && capacity != 0U) {
        if (service->executionStale)
            (void)snprintf(message, capacity,
                "Run stopped after its workspace, catalogue or history changed. No further results were applied.");
        else (void)snprintf(message, capacity,
            "Test run %s: %zu passed, %zu failed, %zu skipped, %zu cancelled, %zu timed out, %zu not run; %zu attempts not started.",
            progress.status == UMI_STATUS_OK ? "complete" :
                progress.status == UMI_STATUS_CANCELLED ? "cancelled" : "finished with errors",
            progress.passed, progress.failed, progress.skipped, progress.cancelled,
            progress.timed_out, progress.not_run, progress.planned - progress.completed);
    }
    return service->executionStale ? UMI_STATUS_INVALID_STATE : progress.status;
}

UmiStatus UmiStudioTestRunSnapshot(UmiStudioTestService *service, UmiCtestJobSnapshot *outSnapshot)
{
    if (service == NULL || outSnapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->executionJob == NULL) return UMI_STATUS_NOT_FOUND;
    if (service->executionOwner != umi_thread_current_id()) return UMI_STATUS_INVALID_STATE;
    return UmiCtestJobGetSnapshot(service->executionJob, outSnapshot);
}

UmiStatus UmiStudioTestRunResultAt(UmiStudioTestService *service, size_t index,
    UmiTestResult *outResult, uint32_t *outAttempt)
{
    if (service == NULL || outResult == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->executionJob == NULL) return UMI_STATUS_NOT_FOUND;
    if (service->executionOwner != umi_thread_current_id()) return UMI_STATUS_INVALID_STATE;
    return UmiCtestJobResultAt(service->executionJob, index, outResult, outAttempt);
}

/* Non-GUI owners must keep the callback payload alive until cancellation is
 * acknowledged. The interactive close guard avoids entering this wait. */
static int UmiStudioExecutionDispose(UmiStudioTestService *service)
{
    if (service->executionJob == NULL) return 1;
    if (service->executionOwner != umi_thread_current_id()) return 0;
    (void)UmiCtestJobCancel(service->executionJob);
    UmiStatus status;
    do {
        status = UmiCtestJobDestroy(service->executionJob);
        if (status == UMI_STATUS_BUSY) umi_thread_sleep_ms(1U);
    } while (status == UMI_STATUS_BUSY);
    if (status != UMI_STATUS_OK) return 0;
    service->executionJob = NULL;
    service->executionPending = 0;
    return 1;
}
