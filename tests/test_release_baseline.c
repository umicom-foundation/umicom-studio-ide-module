/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_release_baseline.c
 *
 * PURPOSE:
 *   Verify the product release catalogue without starting GTK or another
 *   application.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/release_baseline.h"

#include <stdio.h>
#include <string.h>

#define CHECK(X) do { if (!(X)) { \
    fprintf(stderr, "check failed at line %d: %s\n", __LINE__, #X); return 1; \
} } while (0)

static void make_pass(UmiStudioReleaseEvidence *e, size_t index)
{
    const UmiStudioReleaseRequirement *item = UmiStudioReleaseRequirementAt(index);
    memset(e, 0, sizeof(*e));
    (void)snprintf(e->id, sizeof e->id, "%s", item->id);
    e->state = UMI_STUDIO_RELEASE_PASSED;
    (void)snprintf(e->detail, sizeof e->detail, "verified");
}

int main(void)
{
    UmiStudioReleaseSummary summary = {99U,99U,99U,99U,99U,99U,1};
    UmiStudioReleaseEvidence evidence[20];
    UmiStudioReleaseEvidence invalid;
    size_t i;

    CHECK(UmiStudioReleaseRequirementCount() == 20U);
    for (i = 0U; i < 20U; ++i) {
        const UmiStudioReleaseRequirement *item = UmiStudioReleaseRequirementAt(i);
        char expected[16];
        CHECK(item != NULL);
        (void)snprintf(expected, sizeof expected, "STU-%02u", (unsigned)(i + 1U));
        CHECK(strcmp(item->id, expected) == 0);
        CHECK(item->title[0] != '\0');
        CHECK(item->exit_test[0] != '\0');
        CHECK(item->reusable_owner[0] != '\0');
        CHECK(UmiStudioReleaseRequirementFind(expected) == item);
    }
    CHECK(UmiStudioReleaseRequirementAt(20U) == NULL);
    CHECK(UmiStudioReleaseRequirementFind("STU-21") == NULL);

    CHECK(UmiStudioReleaseAssess(NULL, 0U, &summary) == UMI_STATUS_OK);
    CHECK(summary.not_run == 20U && summary.passed == 0U && !summary.release_ready);

    for (i = 0U; i < 20U; ++i) make_pass(&evidence[i], i);
    CHECK(UmiStudioReleaseAssess(evidence, 20U, &summary) == UMI_STATUS_OK);
    CHECK(summary.passed == 20U && summary.release_ready);

    evidence[7].state = UMI_STUDIO_RELEASE_FAILED;
    CHECK(UmiStudioReleaseAssess(evidence, 20U, &summary) == UMI_STATUS_OK);
    CHECK(summary.failed == 1U && summary.passed == 19U && !summary.release_ready);
    evidence[7].state = UMI_STUDIO_RELEASE_BLOCKED;
    CHECK(UmiStudioReleaseAssess(evidence, 20U, &summary) == UMI_STATUS_OK);
    CHECK(summary.blocked == 1U && !summary.release_ready);
    evidence[7].state = UMI_STUDIO_RELEASE_SKIPPED;
    CHECK(UmiStudioReleaseAssess(evidence, 20U, &summary) == UMI_STATUS_OK);
    CHECK(summary.skipped == 1U && !summary.release_ready);
    evidence[7].state = UMI_STUDIO_RELEASE_PASSED;

    invalid = evidence[0];
    (void)snprintf(invalid.id, sizeof invalid.id, "STU-99");
    summary.requirement_count = 777U;
    CHECK(UmiStudioReleaseAssess(&invalid, 1U, &summary) == UMI_STATUS_NOT_FOUND);
    CHECK(summary.requirement_count == 777U);

    invalid = evidence[0];
    invalid.state = (UmiStudioReleaseEvidenceState)99;
    CHECK(UmiStudioReleaseAssess(&invalid, 1U, &summary) == UMI_STATUS_INVALID_ARGUMENT);

    UmiStudioReleaseEvidence duplicate[2] = {evidence[0], evidence[0]};
    CHECK(UmiStudioReleaseAssess(duplicate, 2U, &summary) == UMI_STATUS_ALREADY_EXISTS);

    puts("Studio release-baseline tests passed.");
    return 0;
}
