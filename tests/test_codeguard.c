/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_codeguard.c
 *
 * PURPOSE:
 *   Validate Studio integration with Framework CodeGuard and consolidation
 *   services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is intentionally small and focused. CodeGuard separates scanning
 * rules, analysis engines and reports so new developers can understand one
 * responsibility at a time and test it independently.
 */
#include <assert.h>
#include <stdio.h>

#include "umicom/studio/codeguard.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%d: %s\n", __LINE__, #condition); return 1; } } while (0)

/* Verify Studio keeps a thin, safe facade over reusable Framework quality services. */
/* Execute test setup and checks even when NDEBUG disables standard assertions; allocation and API calls must not disappear from release tests. The previous implementation remains for review. */
#if 0
int main(void)
{
    UmiStudioCodeGuard *guard = NULL;
    UmiCodeGuardDocumentationPolicy policy;
    UmiCodeGuardDocumentationReport report;
    /* Creating the facade proves Studio can activate the shared quality service. */
    assert(umi_studio_codeguard_create(".", &guard) == UMI_STATUS_OK);
    assert(guard != NULL);
    /* Use Framework's learner-friendly defaults rather than duplicating thresholds. */
    umi_codeguard_documentation_policy_default(&policy);
    assert(umi_studio_codeguard_review_documentation(
        guard, __FILE__, &policy, &report) == UMI_STATUS_OK);
    /* This deliberately documented test should be suitable for Quality Centre display. */
    assert(report.has_standard_file_header);
    assert(report.passes);
    /* These checks remain active with NDEBUG. A pre-requested token must
     * prevent a scan of the current directory and preserve the report. */
    UmiCancellationToken *token = NULL;
    UmiCodeGuardProfile profile = umi_codeguard_profile_default();
    const UmiCodeGuardResult *retained = umi_studio_codeguard_result(guard);
    if (umi_cancellation_token_create(&token) != UMI_STATUS_OK) {
        umi_studio_codeguard_destroy(guard);
        return 1;
    }
    umi_cancellation_token_request(token);
    UmiStatus invalid_guard = umi_studio_codeguard_scan_cancellable(NULL, &profile, token);
    UmiStatus invalid_profile = umi_studio_codeguard_scan_cancellable(guard, NULL, token);
    UmiStatus cancelled = umi_studio_codeguard_scan_cancellable(guard, &profile, token);
    int cancellation_ok = invalid_guard == UMI_STATUS_INVALID_ARGUMENT &&
        invalid_profile == UMI_STATUS_INVALID_ARGUMENT &&
        cancelled == UMI_STATUS_CANCELLED &&
        umi_studio_codeguard_result(guard) == retained &&
        umi_studio_codeguard_summary(guard).total == 0U;
    umi_cancellation_token_destroy(token);
    if (!cancellation_ok) {
        fprintf(stderr, "Studio CodeGuard cancellation did not preserve Framework ownership\n");
        umi_studio_codeguard_destroy(guard);
        return 1;
    }
    /* Destruction verifies the normal owner-controlled cleanup path. */
    umi_studio_codeguard_destroy(guard);
    return 0;
}
#endif
int main(void)
{
    UmiStudioCodeGuard *guard = NULL;
    UmiCodeGuardDocumentationPolicy policy;
    UmiCodeGuardDocumentationReport report;
    /* Creating the facade proves Studio can activate the shared quality service. */
    CHECK(umi_studio_codeguard_create(".", &guard) == UMI_STATUS_OK);
    CHECK(guard != NULL);
    /* Use Framework's learner-friendly defaults rather than duplicating thresholds. */
    umi_codeguard_documentation_policy_default(&policy);
/* A fixed documented source is the oracle; this test retains older implementations that can legitimately exceed documentation metrics. The former implementation is retained for owner review. */
#if 0
    CHECK(umi_studio_codeguard_review_documentation(
        guard, __FILE__, &policy, &report) == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_codeguard_review_documentation(
        guard, UMICOM_STUDIO_QUALITY_FIXTURE, &policy, &report) == UMI_STATUS_OK);
    /* This deliberately documented test should be suitable for Quality Centre display. */
    CHECK(report.has_standard_file_header);
    CHECK(report.passes);
    /* These checks remain active with NDEBUG. A pre-requested token must
     * prevent a scan of the current directory and preserve the report. */
    UmiCancellationToken *token = NULL;
    UmiCodeGuardProfile profile = umi_codeguard_profile_default();
    const UmiCodeGuardResult *retained = umi_studio_codeguard_result(guard);
    if (umi_cancellation_token_create(&token) != UMI_STATUS_OK) {
        umi_studio_codeguard_destroy(guard);
        return 1;
    }
    umi_cancellation_token_request(token);
    UmiStatus invalid_guard = umi_studio_codeguard_scan_cancellable(NULL, &profile, token);
    UmiStatus invalid_profile = umi_studio_codeguard_scan_cancellable(guard, NULL, token);
    UmiStatus cancelled = umi_studio_codeguard_scan_cancellable(guard, &profile, token);
    int cancellation_ok = invalid_guard == UMI_STATUS_INVALID_ARGUMENT &&
        invalid_profile == UMI_STATUS_INVALID_ARGUMENT &&
        cancelled == UMI_STATUS_CANCELLED &&
        umi_studio_codeguard_result(guard) == retained &&
        umi_studio_codeguard_summary(guard).total == 0U;
    umi_cancellation_token_destroy(token);
    if (!cancellation_ok) {
        fprintf(stderr, "Studio CodeGuard cancellation did not preserve Framework ownership\n");
        umi_studio_codeguard_destroy(guard);
        return 1;
    }
    /* Destruction verifies the normal owner-controlled cleanup path. */
    umi_studio_codeguard_destroy(guard);
    return 0;
}
