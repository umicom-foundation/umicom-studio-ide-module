/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/release_baseline_main.c
 *
 * PURPOSE:
 *   Inspect the Studio stable-release catalogue from a native C23 command.
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

static int print_catalogue(void)
{
    size_t i;
    for (i = 0U; i < UmiStudioReleaseRequirementCount(); ++i) {
        const UmiStudioReleaseRequirement *item =
            UmiStudioReleaseRequirementAt(i);
        if (item == NULL) return 2;
        printf("%s | %s\n", item->id, item->title);
        printf("  owner: %s\n", item->reusable_owner);
        printf("  exit : %s\n", item->exit_test);
    }
    return 0;
}

static int self_test(void)
{
    UmiStudioReleaseSummary summary;
    UmiStudioReleaseEvidence evidence[20] = {0};
    size_t i;

    if (UmiStudioReleaseRequirementCount() != 20U) return 10;
    if (UmiStudioReleaseAssess(NULL, 0U, &summary) != UMI_STATUS_OK) return 11;
    if (summary.requirement_count != 20U || summary.not_run != 20U ||
        summary.release_ready) return 12;

    for (i = 0U; i < 20U; ++i) {
        const UmiStudioReleaseRequirement *item =
            UmiStudioReleaseRequirementAt(i);
        if (item == NULL) return 13;
        (void)snprintf(evidence[i].id, sizeof evidence[i].id, "%s", item->id);
        evidence[i].state = UMI_STUDIO_RELEASE_PASSED;
        (void)snprintf(evidence[i].detail, sizeof evidence[i].detail,
            "self-test evidence");
    }
    if (UmiStudioReleaseAssess(evidence, 20U, &summary) != UMI_STATUS_OK)
        return 14;
    if (!summary.release_ready || summary.passed != 20U) return 15;
    puts("Studio release-baseline self-test passed.");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) return self_test();
    if (argc == 2 && strcmp(argv[1], "catalogue") == 0) return print_catalogue();
    if (argc == 3 && strcmp(argv[1], "explain") == 0) {
        const UmiStudioReleaseRequirement *item =
            UmiStudioReleaseRequirementFind(argv[2]);
        if (item == NULL) {
            fprintf(stderr, "Unknown Studio release item: %s\n", argv[2]);
            return 2;
        }
        printf("%s — %s\nOwner: %s\nExit test: %s\n",
            item->id, item->title, item->reusable_owner, item->exit_test);
        return 0;
    }

    puts("Umicom Studio IDE release baseline");
    puts("Usage:");
    puts("  umicom-studio-release-baseline catalogue");
    puts("  umicom-studio-release-baseline explain STU-01");
    puts("  umicom-studio-release-baseline --self-test");
    return argc == 1 ? 0 : 2;
}
