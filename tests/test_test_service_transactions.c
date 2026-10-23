/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_service_transactions.c
 * PURPOSE: Verify shared filter state, scoped reruns and complete plans through Studio public services.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/tests.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/services.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto done; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *cases[] = {"large-filter", "text-too-long", "label-too-long", "typed-invalid", "null-filter", "null-output", "empty-filter", "failed-rerun", "timeout-rerun", "suite-rerun", "label-rerun", "disabled-rerun", "passed-rerun", "canonical-plan", "all-capacity", "selected-missing", "selected-duplicate", "selected-null", "selected-capacity", "repeat-selected", "command-search", "command-failed", "command-long"};
    const char *mode = argv[1]; int known = 0, failed = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(mode, cases[i]) == 0) known = 1;
    if (!known) return 2;
    UmiStudioTestService *service = NULL;
    UmiStudioServices *services = NULL;
    UmiCommandRegistry *commands = NULL;
    UmiTestPlatformOperationPlan *plan = malloc(sizeof *plan), *saved_plan = malloc(sizeof *saved_plan);
    UmiStudioTestExplorerState before, after;
    UmiTestWorkspaceSnapshot workspace_before, workspace_after;
    CHECK(plan != NULL && saved_plan != NULL);
    const int command = strncmp(mode, "command-", 8U) == 0;
    if (command) {
        UmiStudioServicesOptions options = {0};
        CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
        service = umi_studio_services_tests(services);
        CHECK(umi_command_registry_create(&commands) == UMI_STATUS_OK);
        CHECK(umi_studio_commands_register(commands, services) == UMI_STATUS_OK);
    } else CHECK(umi_studio_test_service_create(&service) == UMI_STATUS_OK);
    CHECK(service != NULL);
    UmiTestPlatformService *platform = umi_studio_test_service_platform(service);
    UmiTestWorkspace *workspace = umi_studio_test_service_workspace(service);
    UmiTestPlatformItemRegistry *items = umi_test_platform_service_item(platform);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    const size_t count = strcmp(mode, "large-filter") == 0 ? 26032U : strcmp(mode, "all-capacity") == 0 ? 1026U : 6U;
    for (size_t i = 0U; i < count; ++i) {
        UmiTestPlatformItemSnapshot item = {0};
        snprintf(item.id, sizeof item.id, "item.%05zu", i);
        snprintf(item.name, sizeof item.name, "Example %05zu", i);
        strcpy(item.kind, "test"); strcpy(item.suite_id, i < 4U ? "suite.a" : "suite.b");
        strcpy(item.labels, i == 1U ? "slow" : i == 4U ? "foreign" : "fast"); item.enabled = i != 3U;
        CHECK(umi_test_platform_item_registry_upsert(items, &item) == UMI_STATUS_OK);
    }
    for (size_t i = 0U; i < 5U; ++i) {
        UmiTestPlatformResultSnapshot result = {0};
        snprintf(result.id, sizeof result.id, "result.%zu", i);
        snprintf(result.item_id, sizeof result.item_id, "item.%05zu", i);
        result.outcome = i == 1U ? UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT : i == 2U ? UMI_TEST_PLATFORM_OUTCOME_PASSED : UMI_TEST_PLATFORM_OUTCOME_FAILED;
        CHECK(umi_test_platform_result_registry_upsert(results, &result) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_test_service_set_filter(service, "00000", NULL, -1, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_test_service_explorer_state(service, &before) == UMI_STATUS_OK);
    CHECK(umi_test_workspace_snapshot(workspace, &workspace_before) == UMI_STATUS_OK);
    umi_test_platform_operation_plan_init(plan, UMI_TEST_PLATFORM_OPERATION_RUN_SELECTED);
    CHECK(umi_test_platform_operation_plan_add(plan, "kept-plan") == UMI_STATUS_OK);
    *saved_plan = *plan;
    UmiTestPlatformFilter filter; umi_test_platform_filter_init(&filter);
    UmiTestPlatformFilterSummary summary = {0}, saved_summary;
    summary.matched_count = 333U; saved_summary = summary;
    char long_text[257], long_label[129];
    memset(long_text, 'x', sizeof long_text - 1U); long_text[256] = 0;
    memset(long_label, 'x', sizeof long_label - 1U); long_label[128] = 0;
    int refused = 0;
    if (strcmp(mode, "text-too-long") == 0 || strcmp(mode, "label-too-long") == 0) {
        CHECK(umi_studio_test_service_set_filter(service, strcmp(mode, "text-too-long") == 0 ? long_text : "00001",
            strcmp(mode, "label-too-long") == 0 ? long_label : "", -1, 0) == UMI_STATUS_CAPACITY_EXCEEDED);
        refused = 1;
    } else if (strcmp(mode, "typed-invalid") == 0 || strncmp(mode, "null-", 5U) == 0) {
        if (strcmp(mode, "typed-invalid") == 0) memset(filter.label, 'x', sizeof filter.label);
        CHECK(UmiStudioTestServiceApplyFilter(service, strcmp(mode, "null-filter") == 0 ? NULL : &filter,
            strcmp(mode, "null-output") == 0 ? NULL : &summary) == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(memcmp(&summary, &saved_summary, sizeof summary) == 0); refused = 1;
    } else if (strcmp(mode, "large-filter") == 0 || strcmp(mode, "empty-filter") == 0) {
        const int empty = strcmp(mode, "empty-filter") == 0;
        CHECK(umi_studio_test_service_set_filter(service, empty ? "absent" : NULL, NULL, -1, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_explorer_state(service, &after) == UMI_STATUS_OK);
        CHECK(umi_test_workspace_snapshot(workspace, &workspace_after) == UMI_STATUS_OK);
        CHECK(after.selected_count == (empty ? 0U : count - 1U) && after.selected_count == workspace_after.visible_item_count);
        CHECK(after.revision == before.revision + 1U);
    } else if (strstr(mode, "-rerun") != NULL) {
        const char *expected_id = "item.00000"; size_t expected_count = 1U;
        if (strcmp(mode, "failed-rerun") == 0) strcpy(filter.text, "00000");
        if (strcmp(mode, "timeout-rerun") == 0) { strcpy(filter.text, "00001"); expected_id = "item.00001"; }
        if (strcmp(mode, "suite-rerun") == 0) { strcpy(filter.suite_id, "suite.a"); expected_count = 2U; }
        if (strcmp(mode, "label-rerun") == 0) strcpy(filter.label, "fast");
        if (strcmp(mode, "disabled-rerun") == 0) { strcpy(filter.text, "00003"); filter.include_disabled = 1; expected_id = "item.00003"; }
        if (strcmp(mode, "passed-rerun") == 0) { filter.outcome = UMI_TEST_PLATFORM_OUTCOME_PASSED; expected_count = 0U; }
        CHECK(UmiStudioTestServiceApplyFilter(service, &filter, &summary) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_plan_failed(service, plan) == UMI_STATUS_OK);
        CHECK(plan->kind == UMI_TEST_PLATFORM_OPERATION_RERUN_FAILED && plan->selection.count == expected_count);
        if (expected_count != 0U) CHECK(strcmp(plan->selection.item_ids[0], expected_id) == 0);
        if (expected_count == 2U) CHECK(strcmp(plan->selection.item_ids[1], "item.00001") == 0);
    } else if (strcmp(mode, "canonical-plan") == 0) {
        CHECK(umi_test_workspace_set_filter(workspace, "00002", "", "", -1, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_plan_all(service, 0U, 0, plan) == UMI_STATUS_OK);
        CHECK(plan->selection.count == 1U && strcmp(plan->selection.item_ids[0], "item.00002") == 0);
    } else if (strcmp(mode, "all-capacity") == 0) {
        CHECK(umi_studio_test_service_set_filter(service, NULL, NULL, -1, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_explorer_state(service, &before) == UMI_STATUS_OK);
        CHECK(umi_studio_test_service_plan_all(service, 1U, 0, plan) == UMI_STATUS_CAPACITY_EXCEEDED);
        CHECK(memcmp(plan, saved_plan, sizeof *plan) == 0);
        CHECK(umi_studio_test_service_explorer_state(service, &after) == UMI_STATUS_OK);
        CHECK(memcmp(&before, &after, sizeof before) == 0);
    } else if (strncmp(mode, "selected-", 9U) == 0 || strcmp(mode, "repeat-selected") == 0) {
        const char *ids[] = {"item.00000", "item.00001"}; size_t requested = 2U;
        UmiStatus expected = UMI_STATUS_OK;
        if (strcmp(mode, "selected-missing") == 0) { ids[1] = "missing"; expected = UMI_STATUS_NOT_FOUND; }
        if (strcmp(mode, "selected-duplicate") == 0) { ids[1] = ids[0]; expected = UMI_STATUS_ALREADY_EXISTS; }
        if (strcmp(mode, "selected-null") == 0) { ids[1] = NULL; expected = UMI_STATUS_INVALID_ARGUMENT; }
        if (strcmp(mode, "selected-capacity") == 0) { requested = 1025U; expected = UMI_STATUS_CAPACITY_EXCEEDED; }
        CHECK(umi_studio_test_service_plan_selected(service, ids, requested, 3U, 1, plan) == expected);
        if (expected != UMI_STATUS_OK) {
            CHECK(memcmp(plan, saved_plan, sizeof *plan) == 0); refused = 1;
        } else CHECK(plan->selection.count == 2U && plan->repeat_count == 3U && plan->stop_on_failure &&
            plan->kind == UMI_TEST_PLATFORM_OPERATION_REPEAT && umi_test_platform_operation_execution_count(plan) == 6U);
    } else if (command) {
        char message[512]; const int long_query = strcmp(mode, "command-long") == 0;
        const int failures = strcmp(mode, "command-failed") == 0;
        CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_TESTS_FILTER,
            long_query ? long_text : failures ? "failed" : "00002", message, sizeof message) ==
            (long_query ? UMI_STATUS_CAPACITY_EXCEEDED : UMI_STATUS_OK));
        if (long_query) refused = 1;
        else {
            CHECK(umi_studio_test_service_explorer_state(service, &after) == UMI_STATUS_OK);
            CHECK(umi_test_workspace_snapshot(workspace, &workspace_after) == UMI_STATUS_OK);
            CHECK(after.selected_count == (failures ? 3U : 1U) && after.selected_count == workspace_after.visible_item_count);
            CHECK(strcmp(after.search_text, failures ? "" : "00002") == 0 && workspace_after.filter.failed_only == failures);
            CHECK(strstr(message, failures ? "3 test(s)" : "1 test(s)") != NULL);
            CHECK(umi_studio_test_service_plan_all(service, 1U, 0, plan) == UMI_STATUS_OK);
            CHECK(plan->selection.count == after.selected_count);
        }
    }
    if (refused) {
        CHECK(umi_studio_test_service_explorer_state(service, &after) == UMI_STATUS_OK);
        CHECK(umi_test_workspace_snapshot(workspace, &workspace_after) == UMI_STATUS_OK);
        CHECK(memcmp(&before, &after, sizeof before) == 0);
        CHECK(workspace_after.revision == workspace_before.revision &&
            memcmp(&workspace_after.filter, &workspace_before.filter, sizeof workspace_before.filter) == 0);
        CHECK(strcmp(workspace_after.selected_item_id, workspace_before.selected_item_id) == 0);
    }
done:
    free(plan); free(saved_plan); umi_command_registry_destroy(commands);
    if (services != NULL) umi_studio_services_destroy(services); else umi_studio_test_service_destroy(service);
    return failed;
}
