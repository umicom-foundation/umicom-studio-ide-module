/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/duplicates_main.c
 *
 * PURPOSE:
 *   Provide a native Studio command for CodeGuard security and consolidation
 *   inspection.
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
#include "umicom/studio/duplicate_review.h"
#include <stdio.h>
int main(int argc, char **argv)
{
    UmiStudioSourceGovernanceSummary summary;
    const char *root = argc > 1 ? argv[1] : ".";

    if (umi_studio_source_governance_review(root, &summary) != UMI_STATUS_OK) {
        return 2;
    }

    (void)printf("Duplicate source findings: %zu\n",
                 summary.duplicate_findings);
    (void)printf("Version-labelled filenames: %zu\n",
                 summary.versioned_name_findings);
    (void)printf("Batch-labelled filenames: %zu\n",
                 summary.batch_name_findings);
    return summary.duplicate_findings == 0U &&
                   summary.versioned_name_findings == 0U &&
                   summary.batch_name_findings == 0U
               ? 0
               : 1;
}
