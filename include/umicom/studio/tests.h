/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/tests.h
 *
 * PURPOSE:
 *   Expose test discovery, execution and summaries through the Framework testing model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_TESTS_H
#define UMICOM_STUDIO_TESTS_H

#include <stddef.h>

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio test service data shared with callers of this public contract.
 */
typedef struct UmiStudioTestService UmiStudioTestService;

/**
 * Represent the studio test snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioTestSnapshot {
    char build_directory[UMI_BUILD_PATH_CAPACITY];
    size_t suite_count;
    size_t test_count;
    size_t passed;
    size_t failed;
    size_t skipped;
    size_t cancelled;
    size_t timed_out;
    size_t selected_count;
    size_t retained_result_count;
    size_t retained_attachment_count;
    uint64_t workspace_revision;
    uint64_t explorer_revision;
    int operation_running;
    int stop_requested;
} UmiStudioTestSnapshot;

/**
 * Represent the studio test explorer state data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioTestExplorerState {
    char workspace_root[UMI_BUILD_PATH_CAPACITY];
    char active_project_id[128];
    char active_suite_id[128];
    char search_text[256];
    char label_filter[128];
    int outcome_filter;
    int include_disabled;
    size_t selected_count;
    uint64_t workspace_revision;
    uint64_t revision;
} UmiStudioTestExplorerState;

/**
 * Initialise studio test service from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_test_service_create(UmiStudioTestService **out_service);
/**
 * Release or reset state held by studio test service so the same storage can be reused
 * safely.
 */
void umi_studio_test_service_destroy(UmiStudioTestService *service);
/**
 * Provide the studio test service discover operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_discover(UmiStudioTestService *service,
                                           const char *build_directory,
                                           size_t *out_discovered);
/**
 * Provide the studio test service discover metadata operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_discover_metadata(
    UmiStudioTestService *service,
    const char *workspace_root,
    const char *project_id,
    const char *build_directory,
    const char *configuration,
    UmiTestPlatformCtestImportSummary *out_summary
);
/* Planning selects stable ids without starting child processes. Frontends can
 * inspect the plan before explicitly beginning or executing the operation. */
UmiStatus umi_studio_test_service_run_all(UmiStudioTestService *service,
                                          UmiCancellationToken *cancellation,
                                          UmiTestRunSummary *out_summary);
/**
 * Provide the studio test service plan all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_plan_all(
    UmiStudioTestService *service,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan
);
/**
 * Find studio test service plan while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_test_service_plan_selected(
    UmiStudioTestService *service,
    const char *const *item_ids,
    size_t item_count,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan
);
/**
 * Provide the studio test service plan failed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_plan_failed(
    UmiStudioTestService *service,
    UmiTestPlatformOperationPlan *out_plan
);
/**
 * Provide the studio test service begin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_begin(
    UmiStudioTestService *service,
    const UmiTestPlatformOperationPlan *plan
);
/**
 * Perform studio test service through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_test_service_execute(
    UmiStudioTestService *service,
    const UmiTestPlatformOperationPlan *plan,
    UmiTestPlatformExecutionSummary *out_summary
);
/**
 * Provide the studio test service stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_stop(UmiStudioTestService *service);
/**
 * Provide the studio test service finish operation used by this module and its client
 * applications.
 */
void umi_studio_test_service_finish(UmiStudioTestService *service);
/**
 * Provide the studio test service set filter operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_set_filter(
    UmiStudioTestService *service,
    const char *search_text,
    const char *label,
    int outcome,
    int include_disabled
);
/**
 * Provide the studio test service set workspace operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_set_workspace(
    UmiStudioTestService *service,
    const char *workspace_root,
    const char *project_id,
    uint64_t workspace_revision
);
/**
 * Provide the studio test service explorer state operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_service_explorer_state(
    const UmiStudioTestService *service,
    UmiStudioTestExplorerState *out_state
);
/**
 * Provide the studio test service hierarchy operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_hierarchy(
    UmiStudioTestService *service,
    UmiTestPlatformHierarchyNode *nodes,
    size_t capacity,
    size_t *out_count
);
/**
 * Provide the studio test service snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_test_service_snapshot(
    const UmiStudioTestService *service,
    UmiStudioTestSnapshot *out_snapshot
);
/**
 * Provide the studio test service registry operation used by this module and its client
 * applications.
 */
UmiTestRegistry *umi_studio_test_service_registry(
    UmiStudioTestService *service
);
/**
 * Provide the studio test service platform operation used by this module and its client
 * applications.
 */
UmiTestPlatformService *umi_studio_test_service_platform(
    UmiStudioTestService *service
);
/* Framework owns professional Test Explorer selection and presentation state;
 * Studio only exposes the coordinator bound to its application service. */
UmiTestWorkspace *umi_studio_test_service_workspace(
    UmiStudioTestService *service
);

#ifdef __cplusplus
}
#endif

#endif
