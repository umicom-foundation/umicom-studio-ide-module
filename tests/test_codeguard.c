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

#include "umicom/studio/codeguard.h"

/* Verify Studio keeps a thin, safe facade over reusable Framework quality services. */
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
    /* Destruction verifies the normal owner-controlled cleanup path. */
    umi_studio_codeguard_destroy(guard);
    return 0;
}
