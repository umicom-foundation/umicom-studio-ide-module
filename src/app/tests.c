/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/tests.c
 *
 * PURPOSE:
 *   Implement Studio test discovery, execution and aggregate test summaries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioTestService {
    UmiTestRegistry *registry;
    UmiTestSuite *ctest_suite;
    UmiTestPlatformService *platform;
    UmiTestWorkspace *workspace;
    UmiTestPlatformFilter filter;
    UmiStudioTestExplorerState explorer;
    char build_directory[UMI_BUILD_PATH_CAPACITY];
    UmiTestRunSummary last_summary;
};

/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char *destination, size_t capacity, const char *source)
{
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U) return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source == NULL) source = "";
    length = strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length >= capacity) length = capacity - 1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length > 0U) (void)memcpy(destination, source, length);
    destination[length] = '\0';
}

/*
 * Initialise studio test service from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_test_service_create(UmiStudioTestService **out_service)
{
    UmiStudioTestService *service;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_service = NULL;
    service = (UmiStudioTestService *)calloc(1U, sizeof(*service));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_test_registry_create(&service->registry);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_suite_create("studio.ctest", "Studio CTest",
                                       &service->ctest_suite);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_registry_add(service->registry,
                                       service->ctest_suite);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_platform_service_create(&service->platform);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_workspace_create(service->platform,
                                           &service->workspace);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_test_service_destroy(service);
        return status;
    }
    umi_test_platform_filter_init(&service->filter);
    service->explorer.outcome_filter = -1;
    service->explorer.revision = 1U;
    *out_service = service;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio test service so the same storage can be reused
 * safely.
 */
void umi_studio_test_service_destroy(UmiStudioTestService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_test_workspace_destroy(service->workspace);
    umi_test_platform_service_destroy(service->platform);
    umi_test_registry_destroy(service->registry);
    free(service);
}

/*
 * Provide the studio test service discover metadata operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_discover_metadata(
    UmiStudioTestService *service,
    const char *workspace_root,
    const char *project_id,
    const char *build_directory,
    const char *configuration,
    UmiTestPlatformCtestImportSummary *out_summary)
{
    UmiTestPlatformCtestImportOptions options;
    char diagnostics[UMI_PROCESS_OUTPUT_CAPACITY];
    int written;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || project_id == NULL || project_id[0] == '\0' ||
        build_directory == NULL || build_directory[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(&options, 0, sizeof(options));
    copy_text(options.project_id, sizeof(options.project_id), project_id);
    written = snprintf(options.suite_id, sizeof(options.suite_id), "%s.ctest",
                       project_id);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(options.suite_id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    copy_text(options.configuration, sizeof(options.configuration),
              configuration != NULL ? configuration : "Debug");
    copy_text(options.build_directory, sizeof(options.build_directory),
              build_directory);
    status = umi_test_platform_service_discover_ctest(
        service->platform, &options, out_summary, diagnostics,
        sizeof(diagnostics));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    copy_text(service->build_directory, sizeof(service->build_directory),
              build_directory);
    copy_text(service->explorer.workspace_root,
              sizeof(service->explorer.workspace_root), workspace_root);
    copy_text(service->explorer.active_project_id,
              sizeof(service->explorer.active_project_id), project_id);
    copy_text(service->explorer.active_suite_id,
              sizeof(service->explorer.active_suite_id), options.suite_id);
    service->explorer.revision += 1U;
    (void)umi_test_workspace_refresh(service->workspace);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service discover operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_discover(UmiStudioTestService *service,
                                           const char *build_directory,
                                           size_t *out_discovered)
{
    size_t length;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || build_directory == NULL || build_directory[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    length = strlen(build_directory);
    /* Create this optional product surface only when its build option is enabled. */
    if (length + 1U > sizeof(service->build_directory)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    umi_test_registry_remove(service->registry, "studio.ctest");
    service->ctest_suite = NULL;
    status = umi_test_suite_create("studio.ctest", "Studio CTest",
                                   &service->ctest_suite);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ctest_discover(build_directory,
                                    service->ctest_suite,
                                    out_discovered);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_registry_add(service->registry,
                                       service->ctest_suite);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)memcpy(service->build_directory, build_directory, length + 1U);
        (void)umi_test_workspace_refresh(service->workspace);
    }
    return status;
}

/*
 * Provide the studio test service run all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_run_all(UmiStudioTestService *service,
                                          UmiCancellationToken *cancellation,
                                          UmiTestRunSummary *out_summary)
{
    UmiTestResult *results;
    size_t count;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_summary == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    count = umi_test_suite_count(service->ctest_suite);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (count == 0U) {
        (void)memset(out_summary, 0, sizeof(*out_summary));
        return UMI_STATUS_NOT_FOUND;
    }
    results = (UmiTestResult *)calloc(count, sizeof(*results));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (results == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_test_runner_run_suite(service->ctest_suite,
                                       cancellation,
                                       results,
                                       count,
                                       out_summary);
    service->last_summary = *out_summary;
    free(results);
    return status;
}

/*
 * Provide the studio test service plan all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_plan_all(
    UmiStudioTestService *service,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_plan == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_test_platform_operation_plan_init(
        out_plan, repeat_count > 1U ? UMI_TEST_PLATFORM_OPERATION_REPEAT
                                   : UMI_TEST_PLATFORM_OPERATION_RUN_ALL);
    status = umi_test_platform_operation_plan_all(
        out_plan, umi_test_platform_service_item(service->platform),
        umi_test_platform_service_result(service->platform), &service->filter);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    out_plan->kind = repeat_count > 1U ? UMI_TEST_PLATFORM_OPERATION_REPEAT
                                      : UMI_TEST_PLATFORM_OPERATION_RUN_ALL;
    out_plan->repeat_count = repeat_count == 0U ? 1U : repeat_count;
    out_plan->stop_on_failure = stop_on_failure != 0;
    service->explorer.selected_count = out_plan->selection.count;
    service->explorer.revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Find studio test service plan while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_test_service_plan_selected(
    UmiStudioTestService *service,
    const char *const *item_ids,
    size_t item_count,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan)
{
    size_t index;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || item_ids == NULL || item_count == 0U ||
        out_plan == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    umi_test_platform_operation_plan_init(
        out_plan, UMI_TEST_PLATFORM_OPERATION_RUN_SELECTED);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < item_count; ++index) {
        UmiTestPlatformItemSnapshot item;
        status = umi_test_platform_item_registry_find(
            umi_test_platform_service_item(service->platform), item_ids[index],
            &item);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        status = umi_test_platform_operation_plan_add(out_plan,
                                                       item_ids[index]);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    out_plan->kind = repeat_count > 1U ? UMI_TEST_PLATFORM_OPERATION_REPEAT
                                      : UMI_TEST_PLATFORM_OPERATION_RUN_SELECTED;
    out_plan->repeat_count = repeat_count == 0U ? 1U : repeat_count;
    out_plan->stop_on_failure = stop_on_failure != 0;
    service->explorer.selected_count = out_plan->selection.count;
    service->explorer.revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service plan failed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_plan_failed(
    UmiStudioTestService *service,
    UmiTestPlatformOperationPlan *out_plan)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_plan == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_test_platform_operation_plan_init(
        out_plan, UMI_TEST_PLATFORM_OPERATION_RERUN_FAILED);
    status = umi_test_platform_operation_plan_failed(
        out_plan, umi_test_platform_service_item(service->platform),
        umi_test_platform_service_result(service->platform));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        service->explorer.selected_count = out_plan->selection.count;
        service->explorer.revision += 1U;
    }
    return status;
}

/*
 * Provide the studio test service begin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_begin(
    UmiStudioTestService *service,
    const UmiTestPlatformOperationPlan *plan)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_test_platform_service_begin_operation(service->platform, plan);
}

/* Provide the platform outcome operation used by this module and its client applications. */
static int platform_outcome(UmiTestState state)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (state) {
        case UMI_TEST_STATE_PASSED: return UMI_TEST_PLATFORM_OUTCOME_PASSED;
        case UMI_TEST_STATE_FAILED: return UMI_TEST_PLATFORM_OUTCOME_FAILED;
        case UMI_TEST_STATE_SKIPPED: return UMI_TEST_PLATFORM_OUTCOME_SKIPPED;
        case UMI_TEST_STATE_CANCELLED:
            return UMI_TEST_PLATFORM_OUTCOME_CANCELLED;
        case UMI_TEST_STATE_TIMED_OUT:
            return UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT;
        default: return UMI_TEST_PLATFORM_OUTCOME_NOT_RUN;
    }
}

/*
 * Provide the execute ctest item operation used by this module and its client
 * applications.
 */
static UmiStatus execute_ctest_item(
    const UmiTestPlatformItemSnapshot *item,
    uint32_t attempt,
    void *user_data,
    UmiTestPlatformResultSnapshot *out_result)
{
    UmiStudioTestService *service = (UmiStudioTestService *)user_data;
    UmiTestResult result;
    UmiTestPlatformOutputSnapshot output;
    UmiStatus status;
    int written;
    const char *build_directory;
    build_directory = item->working_directory[0] != '\0'
                          ? item->working_directory
                          : service->build_directory;
    status = umi_ctest_run(build_directory, item->name, &result);
    written = snprintf(out_result->id, sizeof(out_result->id),
                       "result.%llu.%u",
                       (unsigned long long)out_result->sequence,
                       (unsigned)attempt);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(out_result->id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    copy_text(out_result->item_id, sizeof(out_result->item_id), item->id);
    copy_text(out_result->message, sizeof(out_result->message),
              result.state == UMI_TEST_STATE_PASSED
                  ? "Test passed."
                  : "Test did not pass; inspect failure details and output.");
    copy_text(out_result->failure_details,
              sizeof(out_result->failure_details), result.output);
    out_result->duration_ms = (double)result.duration_ms;
    out_result->outcome = platform_outcome(result.state);
    out_result->exit_code = result.exit_code;
    (void)memset(&output, 0, sizeof(output));
    written = snprintf(output.id, sizeof(output.id), "output.%llu.%u",
                       (unsigned long long)out_result->sequence,
                       (unsigned)attempt);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(output.id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    copy_text(output.item_id, sizeof(output.item_id), item->id);
    copy_text(output.stream, sizeof(output.stream), "combined");
    copy_text(output.text, sizeof(output.text), result.output);
    (void)umi_test_platform_output_registry_upsert(
        umi_test_platform_service_output(service->platform), &output);
    return status;
}

/*
 * Perform studio test service through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_test_service_execute(
    UmiStudioTestService *service,
    const UmiTestPlatformOperationPlan *plan,
    UmiTestPlatformExecutionSummary *out_summary)
{
    UmiTestPlatformOperationController *controller;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || plan == NULL || out_summary == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    controller = umi_test_platform_service_operation(service->platform);
    status = umi_test_platform_execute(
        umi_test_platform_service_item(service->platform),
        umi_test_platform_service_result(service->platform), plan, controller,
        execute_ctest_item, service, out_summary);
    service->last_summary.total = out_summary->executed;
    service->last_summary.passed = out_summary->passed;
    service->last_summary.failed = out_summary->failed;
    service->last_summary.skipped = out_summary->skipped;
    service->last_summary.cancelled = out_summary->cancelled;
    service->last_summary.timed_out = out_summary->timed_out;
    service->last_summary.duration_ms = out_summary->duration_ms;
    service->explorer.revision += 1U;
    return status;
}

/*
 * Provide the studio test service stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_stop(UmiStudioTestService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_test_platform_service_request_stop(service->platform);
}

/*
 * Provide the studio test service finish operation used by this module and its client
 * applications.
 */
void umi_studio_test_service_finish(UmiStudioTestService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_test_platform_service_finish_operation(service->platform);
}

/*
 * Provide the studio test service set filter operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_set_filter(
    UmiStudioTestService *service,
    const char *search_text,
    const char *label,
    int outcome,
    int include_disabled)
{
    UmiTestPlatformSelection selection;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || outcome < -1 ||
        outcome > UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    copy_text(service->filter.text, sizeof(service->filter.text), search_text);
    copy_text(service->filter.label, sizeof(service->filter.label), label);
    copy_text(service->filter.suite_id, sizeof(service->filter.suite_id),
              service->explorer.active_suite_id);
    service->filter.outcome = outcome;
    service->filter.include_disabled = include_disabled != 0;
    status = umi_test_workspace_set_filter(
        service->workspace, search_text != NULL ? search_text : "",
        service->explorer.active_suite_id, label != NULL ? label : "",
        outcome, include_disabled, 0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_test_platform_service_select(service->platform,
                                              &service->filter, &selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    copy_text(service->explorer.search_text,
              sizeof(service->explorer.search_text), search_text);
    copy_text(service->explorer.label_filter,
              sizeof(service->explorer.label_filter), label);
    service->explorer.outcome_filter = outcome;
    service->explorer.include_disabled = include_disabled != 0;
    service->explorer.selected_count = selection.count;
    service->explorer.revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service set workspace operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_set_workspace(
    UmiStudioTestService *service,
    const char *workspace_root,
    const char *project_id,
    uint64_t workspace_revision)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || workspace_root == NULL || project_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    copy_text(service->explorer.workspace_root,
              sizeof(service->explorer.workspace_root), workspace_root);
    copy_text(service->explorer.active_project_id,
              sizeof(service->explorer.active_project_id), project_id);
    service->explorer.workspace_revision = workspace_revision;
    service->explorer.revision += 1U;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service explorer state operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_explorer_state(
    const UmiStudioTestService *service,
    UmiStudioTestExplorerState *out_state)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_state == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_state = service->explorer;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service hierarchy operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_hierarchy(
    UmiStudioTestService *service,
    UmiTestPlatformHierarchyNode *nodes,
    size_t capacity,
    size_t *out_count)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_test_platform_service_hierarchy(service->platform, nodes,
                                               capacity, out_count);
}

/*
 * Provide the studio test service snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_snapshot(
    const UmiStudioTestService *service,
    UmiStudioTestSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    (void)snprintf(out_snapshot->build_directory,
                   sizeof(out_snapshot->build_directory),
                   "%s", service->build_directory);
    out_snapshot->suite_count = umi_test_registry_count(service->registry);
    out_snapshot->test_count = umi_test_suite_count(service->ctest_suite);
    out_snapshot->passed = service->last_summary.passed;
    out_snapshot->failed = service->last_summary.failed;
    out_snapshot->skipped = service->last_summary.skipped;
    out_snapshot->cancelled = service->last_summary.cancelled;
    out_snapshot->timed_out = service->last_summary.timed_out;
    {
        UmiTestPlatformServiceSnapshot platform_snapshot;
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_test_platform_service_snapshot(service->platform,
                                               &platform_snapshot) ==
            UMI_STATUS_OK) {
            out_snapshot->selected_count = platform_snapshot.selected_count;
            out_snapshot->retained_result_count =
                platform_snapshot.result_count;
            out_snapshot->retained_attachment_count =
                platform_snapshot.attachment_count;
            out_snapshot->operation_running =
                platform_snapshot.operation_running;
            out_snapshot->stop_requested = platform_snapshot.stop_requested;
        }
    }
    out_snapshot->workspace_revision = service->explorer.workspace_revision;
    out_snapshot->explorer_revision = service->explorer.revision;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test service platform operation used by this module and its client
 * applications.
 */
UmiTestPlatformService *umi_studio_test_service_platform(
    UmiStudioTestService *service)
{
    return service != NULL ? service->platform : NULL;
}

/*
 * Provide the studio test service registry operation used by this module and its client
 * applications.
 */
UmiTestRegistry *umi_studio_test_service_registry(UmiStudioTestService *service)
{
    return service != NULL ? service->registry : NULL;
}

/*
 * Provide the studio test service workspace operation used by this module and its client
 * applications.
 */
UmiTestWorkspace *umi_studio_test_service_workspace(
    UmiStudioTestService *service)
{
    return service != NULL ? service->workspace : NULL;
}
