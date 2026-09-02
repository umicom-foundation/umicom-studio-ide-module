/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/duplicate_review.c
 *
 * PURPOSE:
 *   Integrate Framework CodeGuard analysis into Studio security and
 *   consolidation workflows.
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
#include "umicom/codeguard/codeguard.h"

#include <string.h>

/* Count stable rule identifiers so Studio remains independent of CodeGuard's
 * internal scanner implementation and filesystem traversal details. */
static void umi_studio_source_governance_count_finding(
    const UmiCodeGuardFinding *finding,
    UmiStudioSourceGovernanceSummary *summary)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (finding == NULL || summary == NULL) {
        return;
    }

    ++summary->total_findings;
    /* Apply this branch only when its contract condition is satisfied. */
    if (finding->category == UMI_CODEGUARD_CATEGORY_DUPLICATION) {
        ++summary->duplicate_findings;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(finding->rule_id, "CODEGUARD-NAME-VERSION-001") == 0) {
        ++summary->versioned_name_findings;
    } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(finding->rule_id, "CODEGUARD-NAME-BATCH-001") == 0) {
        ++summary->batch_name_findings;
    }
}

/*
 * Provide the studio source governance review operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_source_governance_review(
    const char *root,
    UmiStudioSourceGovernanceSummary *out_summary)
{
    UmiCodeGuardResult *result = NULL;
    UmiCodeGuardConfig config;
    UmiStatus status;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL || out_summary == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(out_summary, 0, sizeof(*out_summary));
    status = umi_codeguard_result_create(256U, &result);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        /* Select only the checks represented by this focused Studio view. */
        config = umi_codeguard_config_default(root);
        config.profile.scan_architecture = 0;
        config.profile.scan_duplicates = 1;
        config.profile.scan_source_names = 1;
        status = umi_codeguard_scan(&config, result);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < umi_codeguard_result_count(result); ++index) {
            umi_studio_source_governance_count_finding(
                umi_codeguard_result_at(result, index), out_summary);
        }
    }

    umi_codeguard_result_destroy(result);
    return status;
}

/*
 * Provide the studio duplicate review operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_duplicate_review(const char *root,
                                      size_t *out_duplicates)
{
    UmiStudioSourceGovernanceSummary summary;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL || out_duplicates == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    *out_duplicates = 0U;
    status = umi_studio_source_governance_review(root, &summary);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        *out_duplicates = summary.duplicate_findings;
    }
    return status;
}
