/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_commands.c
 *
 * PURPOSE:
 *   Route Studio Quality Centre commands to reusable Framework evidence,
 *   evaluation, compatibility and remediation services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/quality_commands.h"

#include "umicom/studio/quality_compatibility.h"
#include "umicom/studio/quality_duplicates.h"
#include "umicom/studio/quality_evidence.h"
#include "umicom/studio/quality_remediation.h"

/* Create deterministic learning evidence without claiming it is a real build result. */
UmiStatus umi_studio_quality_seed_evidence(UmiStudioQualityCentre *centre)
{
    UmiCodeGuardDuplicateFile first = {"src/common_a.c", 55U, 77U, 240U};
    UmiCodeGuardDuplicateFile second = {"src/common_b.c", 55U, 77U, 240U};
    UmiCodeGuardApiBaseline expected;
    UmiCodeGuardApiBaseline current;
    UmiStatus status;
    /* A missing centre cannot own seeded evidence. */
    if (centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_studio_quality_record_check(
        centre,
        "build.windows",
        UMI_CODEGUARD_EVIDENCE_BUILD,
        true,
        0U,
        0U,
        "CMakeLists.txt",
        1U,
        "Windows build completed",
        "Review compiler output when this check fails");
    /* Record a separate test result only after build evidence succeeded. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_quality_record_check(
            centre,
            "tests.native",
            UMI_CODEGUARD_EVIDENCE_TEST,
            true,
            0U,
            0U,
            "tests",
            1U,
            "Native tests completed",
            "Open the failing test and its captured diagnostics");
    }
    /* Demonstrate one valid application-to-Framework dependency. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_quality_record_architecture_dependency(
            centre,
            "applications/studio/src/app/quality.c",
            "framework/include/umicom/codeguard/codeguard.h");
    }
    /* Add one duplicate candidate so the review view has deterministic content. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_quality_duplicate_add(centre, &first, &second);
    }
    /* Stop before compatibility fixtures if any earlier evidence was rejected. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_codeguard_api_baseline_init(
        &expected, "framework", 1U, 0U);
    /* The current fixture is a compatible later minor release. */
    if (status == UMI_STATUS_OK) {
        status = umi_codeguard_api_baseline_init(
            &current, "framework", 1U, 1U);
    }
    /* Both baselines retain the same existing public start symbol. */
    if (status == UMI_STATUS_OK) {
        status = umi_codeguard_api_baseline_add(
            &expected, "umi_framework_start", 101U, false);
    }
    /* Matching the current signature proves the existing symbol is compatible. */
    if (status == UMI_STATUS_OK) {
        status = umi_codeguard_api_baseline_add(
            &current, "umi_framework_start", 101U, false);
    }
    /* The current fixture adds one compatible quality-session symbol. */
    if (status == UMI_STATUS_OK) {
        status = umi_codeguard_api_baseline_add(
            &current, "umi_quality_session", 102U, false);
    }
    /* Store comparison evidence only after both fixtures are complete. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_quality_compare_api(
            centre, &expected, &current);
    }
    return status;
}

/* Dispatch stable command IDs without duplicating Framework quality behaviour. */
UmiStatus umi_studio_quality_execute(
    UmiStudioQualityCentre *centre,
    UmiStudioQualityCommand command,
    UmiCodeGuardQualityEvaluation *out_evaluation)
{
    /* Each command delegates to one existing Studio facade over Framework logic. */
    switch (command) {
        case UMI_STUDIO_QUALITY_COMMAND_SEED_EVIDENCE:
            return umi_studio_quality_seed_evidence(centre);
        case UMI_STUDIO_QUALITY_COMMAND_EVALUATE:
            /* Evaluation returns a value result and therefore requires output storage. */
            if (out_evaluation == NULL) {
                return UMI_STATUS_INVALID_ARGUMENT;
            }
            return umi_studio_quality_evaluate(centre, out_evaluation);
        case UMI_STUDIO_QUALITY_COMMAND_REFRESH_REMEDIATION:
            return umi_studio_quality_remediation_refresh(centre);
        default:
            return UMI_STATUS_NOT_FOUND;
    }
}
