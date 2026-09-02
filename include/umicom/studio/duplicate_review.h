/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/duplicate_review.h
 *
 * PURPOSE:
 *   Expose Studio integration for CodeGuard security, duplication and
 *   architecture quality analysis.
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
#ifndef UMICOM_STUDIO_DUPLICATE_REVIEW_H
#define UMICOM_STUDIO_DUPLICATE_REVIEW_H
#include <stddef.h>
#include "umicom/base/status.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio source governance summary data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioSourceGovernanceSummary {
    size_t total_findings;
    size_t duplicate_findings;
    size_t versioned_name_findings;
    size_t batch_name_findings;
} UmiStudioSourceGovernanceSummary;

/* Build a presentation-ready summary from Framework-owned CodeGuard results. */
UmiStatus umi_studio_source_governance_review(
    const char *root,
    UmiStudioSourceGovernanceSummary *out_summary);

/* Preserve the original focused API for existing Studio callers. */
UmiStatus umi_studio_duplicate_review(const char *root,size_t *out_duplicates);
#ifdef __cplusplus
}
#endif
#endif
