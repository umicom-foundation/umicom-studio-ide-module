/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/release_baseline.c
 *
 * PURPOSE:
 *   Provide the bounded Studio release catalogue and evidence assessment.
 *   This is product composition state; reusable IDE mechanisms remain owned
 *   by Umicom Framework.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/release_baseline.h"

#include <string.h>

#define REQUIREMENT(ID, TITLE, EXIT_TEST, OWNER) \
    { ID, TITLE, EXIT_TEST, OWNER }

static const UmiStudioReleaseRequirement requirements[] = {
    REQUIREMENT("STU-01", "Release contract and existing feature inventory",
        "Every in-scope feature has a real UI entry point and an acceptance test.",
        "Framework contracts + Studio composition"),
    REQUIREMENT("STU-02", "Workbench shell and window lifecycle",
        "Busy workspaces open and close without stale callbacks, blank overlays or dead actions.",
        "Framework UI/lifecycle + Studio workbench"),
    REQUIREMENT("STU-03", "Projects and repository workspaces",
        "Commands use the selected project and never borrow another project's build root.",
        "Framework project/repository services"),
    REQUIREMENT("STU-04", "Document editing and text correctness",
        "Editing fixtures preserve exact text, Unicode, cursor and selection semantics.",
        "Framework document/editor services"),
    REQUIREMENT("STU-05", "Save, external changes and recovery",
        "Failed saves retain dirty state and recovery never overwrites newer external content.",
        "Framework document persistence"),
    REQUIREMENT("STU-06", "Search and navigation",
        "Every result opens the intended revision and cancelled replacement changes nothing.",
        "Framework search/navigation"),
    REQUIREMENT("STU-07", "Language-server integration",
        "Real language servers use the active compile configuration and recover after termination.",
        "Framework language runtime"),
    REQUIREMENT("STU-08", "Build profiles and toolchain discovery",
        "Compiler output and exit state remain attached to the selected project/profile.",
        "Framework process/toolchain/build"),
    REQUIREMENT("STU-09", "Test Explorer execution integrity",
        "Exact selected tests run; stale XML, empty selections and failed builds cannot report pass.",
        "Framework test execution"),
    REQUIREMENT("STU-10", "Problems and build history",
        "Diagnostics preserve Windows paths, positions and immutable operation evidence.",
        "Framework diagnostics/build history"),
    REQUIREMENT("STU-11", "Debugger launch and stepping",
        "A real native defect can be launched, paused, stepped and terminated through DAP.",
        "Framework debugger protocols"),
    REQUIREMENT("STU-12", "Debugger inspection and memory views",
        "Displayed values belong to the selected stopped frame and stale values are invalidated.",
        "Framework debugger inspection"),
    REQUIREMENT("STU-13", "Terminal and process supervision",
        "Closing Studio-owned terminals cannot orphan owned children or kill unrelated processes.",
        "Framework process supervision"),
    REQUIREMENT("STU-14", "Git staging, history and comparison",
        "Only reviewed files are published and conflict review preserves both sides.",
        "Framework VCS/diff services"),
    REQUIREMENT("STU-15", "Visual designer to runnable project",
        "Generated GUI source builds and regeneration preserves handwritten source/comments.",
        "Framework designer/generation"),
    REQUIREMENT("STU-16", "Extensions and optional AI assistance",
        "Denied extensions remain inactive and AI never silently modifies or publishes source.",
        "Framework extension/AI policy"),
    REQUIREMENT("STU-17", "Profiling, accessibility and performance",
        "Declared keyboard, DPI, accessibility and responsiveness budgets pass.",
        "Framework observability/UI"),
    REQUIREMENT("STU-18", "Installer, updates and standalone projects",
        "A clean installation builds an independent project without developer PATH assumptions.",
        "Framework distribution/runtime"),
    REQUIREMENT("STU-19", "Daily-use release candidate",
        "The frozen candidate passes sustained author/build/test/debug/Git/designer journeys.",
        "Studio release acceptance"),
    REQUIREMENT("STU-20", "Stable release and teaching pack",
        "A new developer completes the installed Notes capstone and matches documented results.",
        "Studio product release")
};

static int valid_state(UmiStudioReleaseEvidenceState state)
{
    return state >= UMI_STUDIO_RELEASE_NOT_RUN &&
           state <= UMI_STUDIO_RELEASE_SKIPPED;
}

size_t UmiStudioReleaseRequirementCount(void)
{
    return sizeof(requirements) / sizeof(requirements[0]);
}

const UmiStudioReleaseRequirement *UmiStudioReleaseRequirementAt(size_t index)
{
    return index < UmiStudioReleaseRequirementCount() ? &requirements[index] : NULL;
}

const UmiStudioReleaseRequirement *UmiStudioReleaseRequirementFind(const char *id)
{
    size_t i;
    if (id == NULL || id[0] == '\0') return NULL;
    for (i = 0U; i < UmiStudioReleaseRequirementCount(); ++i)
        if (strcmp(requirements[i].id, id) == 0) return &requirements[i];
    return NULL;
}

void UmiStudioReleaseSummaryInit(UmiStudioReleaseSummary *summary)
{
    if (summary != NULL) memset(summary, 0, sizeof(*summary));
}

UmiStatus UmiStudioReleaseAssess(
    const UmiStudioReleaseEvidence *evidence,
    size_t evidence_count,
    UmiStudioReleaseSummary *out_summary)
{
    UmiStudioReleaseSummary candidate;
    unsigned char seen[sizeof(requirements) / sizeof(requirements[0])] = {0};
    size_t i;

    if (out_summary == NULL || (evidence_count != 0U && evidence == NULL))
        return UMI_STATUS_INVALID_ARGUMENT;

    UmiStudioReleaseSummaryInit(&candidate);
    candidate.requirement_count = UmiStudioReleaseRequirementCount();
    candidate.not_run = candidate.requirement_count;

    for (i = 0U; i < evidence_count; ++i) {
        const UmiStudioReleaseRequirement *requirement;
        size_t index;

        if (evidence[i].id[0] == '\0' ||
            memchr(evidence[i].id, '\0', sizeof(evidence[i].id)) == NULL ||
            memchr(evidence[i].detail, '\0', sizeof(evidence[i].detail)) == NULL ||
            !valid_state(evidence[i].state))
            return UMI_STATUS_INVALID_ARGUMENT;

        requirement = UmiStudioReleaseRequirementFind(evidence[i].id);
        if (requirement == NULL) return UMI_STATUS_NOT_FOUND;

        index = (size_t)(requirement - requirements);
        if (seen[index] != 0U) return UMI_STATUS_ALREADY_EXISTS;
        seen[index] = 1U;

        --candidate.not_run;
        switch (evidence[i].state) {
            case UMI_STUDIO_RELEASE_PASSED: ++candidate.passed; break;
            case UMI_STUDIO_RELEASE_FAILED: ++candidate.failed; break;
            case UMI_STUDIO_RELEASE_BLOCKED: ++candidate.blocked; break;
            case UMI_STUDIO_RELEASE_SKIPPED: ++candidate.skipped; break;
            case UMI_STUDIO_RELEASE_NOT_RUN: ++candidate.not_run; break;
            default: return UMI_STATUS_INVALID_ARGUMENT;
        }
    }

    candidate.release_ready =
        candidate.passed == candidate.requirement_count &&
        candidate.failed == 0U &&
        candidate.blocked == 0U &&
        candidate.skipped == 0U &&
        candidate.not_run == 0U;

    *out_summary = candidate;
    return UMI_STATUS_OK;
}
