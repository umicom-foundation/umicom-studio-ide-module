/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_duplicate_review.c
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
#include "umicom/studio/duplicate_review.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioSourceGovernanceSummary summary;
    size_t duplicates = 0U;
    UmiStatus status;

    assert(umi_studio_source_governance_review(NULL, &summary) ==
           UMI_STATUS_INVALID_ARGUMENT);
    assert(umi_studio_source_governance_review(".", NULL) ==
           UMI_STATUS_INVALID_ARGUMENT);

    status = umi_studio_source_governance_review(".", &summary);
    assert(status == UMI_STATUS_OK || status == UMI_STATUS_IO_ERROR);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        assert(summary.total_findings >= summary.duplicate_findings);
        assert(summary.total_findings >= summary.versioned_name_findings);
        assert(summary.total_findings >= summary.batch_name_findings);
    }

    status = umi_studio_duplicate_review(".", &duplicates);
    assert(status == UMI_STATUS_OK || status == UMI_STATUS_IO_ERROR);
    return 0;
}
