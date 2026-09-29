/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/release_baseline.h
 *
 * PURPOSE:
 *   Define the product-specific Studio stable-release catalogue used to keep
 *   implementation work focused on one accepted IDE release.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_RELEASE_BASELINE_H
#define UMICOM_STUDIO_RELEASE_BASELINE_H

#include <stddef.h>
#include "umicom/base/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_RELEASE_ID_CAPACITY 16U
#define UMI_STUDIO_RELEASE_TITLE_CAPACITY 128U
#define UMI_STUDIO_RELEASE_EXIT_CAPACITY 256U
#define UMI_STUDIO_RELEASE_OWNER_CAPACITY 96U
#define UMI_STUDIO_RELEASE_DETAIL_CAPACITY 192U

typedef enum UmiStudioReleaseEvidenceState {
    UMI_STUDIO_RELEASE_NOT_RUN = 0,
    UMI_STUDIO_RELEASE_PASSED = 1,
    UMI_STUDIO_RELEASE_FAILED = 2,
    UMI_STUDIO_RELEASE_BLOCKED = 3,
    UMI_STUDIO_RELEASE_SKIPPED = 4
} UmiStudioReleaseEvidenceState;

typedef struct UmiStudioReleaseRequirement {
    char id[UMI_STUDIO_RELEASE_ID_CAPACITY];
    char title[UMI_STUDIO_RELEASE_TITLE_CAPACITY];
    char exit_test[UMI_STUDIO_RELEASE_EXIT_CAPACITY];
    char reusable_owner[UMI_STUDIO_RELEASE_OWNER_CAPACITY];
} UmiStudioReleaseRequirement;

typedef struct UmiStudioReleaseEvidence {
    char id[UMI_STUDIO_RELEASE_ID_CAPACITY];
    UmiStudioReleaseEvidenceState state;
    char detail[UMI_STUDIO_RELEASE_DETAIL_CAPACITY];
} UmiStudioReleaseEvidence;

typedef struct UmiStudioReleaseSummary {
    size_t requirement_count;
    size_t passed;
    size_t failed;
    size_t blocked;
    size_t skipped;
    size_t not_run;
    int release_ready;
} UmiStudioReleaseSummary;

size_t UmiStudioReleaseRequirementCount(void);
const UmiStudioReleaseRequirement *UmiStudioReleaseRequirementAt(size_t index);
const UmiStudioReleaseRequirement *UmiStudioReleaseRequirementFind(const char *id);
void UmiStudioReleaseSummaryInit(UmiStudioReleaseSummary *summary);
UmiStatus UmiStudioReleaseAssess(
    const UmiStudioReleaseEvidence *evidence,
    size_t evidence_count,
    UmiStudioReleaseSummary *out_summary);

#ifdef __cplusplus
}
#endif
#endif
