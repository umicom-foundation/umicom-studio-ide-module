/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/quick_open.c
 *
 * PURPOSE:
 *   Implement fuzzy subsequence scoring and deterministic ranked quick-open results.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#include "umicom/studio/quick_open.h"

#include <ctype.h>
#include <string.h>

/*
 * Provide the studio quick open score operation used by this module and its client
 * applications.
 */
int umi_studio_quick_open_score(const char *query, const char *candidate)
{
    size_t query_index = 0U;
    size_t candidate_index;
    int score = 0;
    int contiguous = 0;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (query == NULL || candidate == NULL) {
        return -1;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (query[0] == '\0') {
        return 1;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (candidate_index = 0U;
         candidate[candidate_index] != '\0' && query[query_index] != '\0';
         ++candidate_index) {
        const unsigned char query_char =
            (unsigned char)tolower((unsigned char)query[query_index]);
        const unsigned char candidate_char =
            (unsigned char)tolower((unsigned char)candidate[candidate_index]);

        /* Apply this branch only when its contract condition is satisfied. */
        if (query_char == candidate_char) {
            score += 10;
            /* Apply this branch only when its contract condition is satisfied. */
            if (candidate_index == 0U) {
                score += 8;
            }
            /* Apply this branch only when its contract condition is satisfied. */
            if (contiguous != 0) {
                score += 5;
            }
            contiguous = 1;
            ++query_index;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            contiguous = 0;
            score -= 1;
        }
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (query[query_index] != '\0') {
        return -1;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (strstr(candidate, query) != NULL) {
        score += 25;
    }

    return score;
}

/* Provide the insert result operation used by this module and its client applications. */
static void insert_result(UmiStudioQuickOpenResults *results,
                          const UmiStudioQuickOpenCandidate *candidate)
{
    size_t position;
    size_t move_count;

    /* Apply this branch only when its contract condition is satisfied. */
    if (candidate->score < 0) {
        return;
    }

    position = 0U;
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (position < results->count &&
           results->items[position].score >= candidate->score) {
        ++position;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (position >= UMI_STUDIO_QUICK_OPEN_MAX_RESULTS) {
        return;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (results->count < UMI_STUDIO_QUICK_OPEN_MAX_RESULTS) {
        ++results->count;
    }

    move_count = results->count - position - 1U;
    /* Apply this branch only when its contract condition is satisfied. */
    if (move_count > 0U) {
        (void)memmove(&results->items[position + 1U],
                      &results->items[position],
                      move_count * sizeof(results->items[0]));
    }

    results->items[position] = *candidate;
}

/*
 * Provide the studio quick open search operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quick_open_search(
    const char *query,
    const UmiStudioQuickOpenCandidate *candidates,
    size_t candidate_count,
    UmiStudioQuickOpenResults *results)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (query == NULL || candidates == NULL || results == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (candidate_count > UMI_STUDIO_QUICK_OPEN_MAX_CANDIDATES) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)memset(results, 0, sizeof(*results));

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < candidate_count; ++index) {
        UmiStudioQuickOpenCandidate scored = candidates[index];
        int label_score = umi_studio_quick_open_score(query, scored.label);
        int detail_score = umi_studio_quick_open_score(query, scored.detail);

        scored.score = label_score > detail_score ? label_score : detail_score;
        insert_result(results, &scored);
    }

    return UMI_STATUS_OK;
}
