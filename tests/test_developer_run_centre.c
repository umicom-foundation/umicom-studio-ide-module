/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_developer_run_centre.c
 *
 * PURPOSE:
 *   Verify Studio run/debug profiles use the shared Framework runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_workbench.h"

/* Existing fixture setup uses assertions; retain those checks in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>


/* Review must observe Framework-owned values without submitting a command.
 * Check both saved-field changes and registry normalization through the real
 * runtime used by this existing Studio test. */
static int RunProfileComparisonCases(UmiStudioDeveloperRunCentre *centre)
{
    UmiDeveloperRuntime *runtime = umi_studio_developer_run_centre_runtime(centre);
    UmiProjectLaunchProfileRegistry *registry = umi_project_workspace_launch_profile(
        umi_developer_runtime_projects(runtime));
    UmiProjectLaunchProfileSnapshot expected, changed;
    UmiDeveloperRuntimeSnapshot runtime_before, runtime_after;
    UmiValueArchiveComparison observation = {0}, before;
    unsigned char scratch[2U * sizeof(expected) + 256U];
    if (umi_studio_developer_run_centre_at(centre, 0U, &expected) != UMI_STATUS_OK ||
        umi_developer_runtime_snapshot(runtime, &runtime_before) != UMI_STATUS_OK)
        return 1;
    uint64_t revision = umi_project_launch_profile_registry_revision(registry);
    if (umi_studio_developer_run_centre_compare_profile(centre, &expected,
            NULL, 0U, &observation) != UMI_STATUS_OK ||
        observation.relation != UMI_VALUE_ARCHIVE_NOT_COMPARED ||
        observation.required_workspace > sizeof(scratch)) return 1;
    size_t needed = observation.required_workspace;
    memset(scratch, 0xa5, sizeof(scratch));
    if (umi_studio_developer_run_centre_compare_profile(centre, &expected,
            scratch, needed - 1U, &observation) != UMI_STATUS_CAPACITY_EXCEEDED ||
        observation.relation != UMI_VALUE_ARCHIVE_NOT_COMPARED) return 1;
    for (size_t i = 0U; i < sizeof(scratch); ++i) if (scratch[i] != 0xa5U) return 1;
    if (umi_studio_developer_run_centre_compare_profile(centre, &expected,
            scratch, sizeof(scratch), &observation) != UMI_STATUS_OK ||
        observation.relation != UMI_VALUE_ARCHIVE_EQUAL) return 1;
    if (umi_developer_runtime_snapshot(runtime, &runtime_after) != UMI_STATUS_OK ||
        runtime_after.revision != runtime_before.revision ||
        umi_project_launch_profile_registry_revision(registry) != revision) return 1;
    memcpy(&before, &observation, sizeof(before));
    if (umi_studio_developer_run_centre_compare_profile(NULL, &expected,
            scratch, sizeof(scratch), &observation) != UMI_STATUS_INVALID_ARGUMENT ||
        memcmp(&before, &observation, sizeof(before)) != 0) return 1;
    changed = expected;
    memset(changed.id, 'x', sizeof(changed.id));
    if (umi_studio_developer_run_centre_compare_profile(centre, &changed,
            scratch, sizeof(scratch), &observation) == UMI_STATUS_OK ||
        memcmp(&before, &observation, sizeof(before)) != 0) return 1;
    changed = expected;
    strcpy(changed.id, "missing.profile");
    if (umi_studio_developer_run_centre_compare_profile(centre, &changed,
            scratch, sizeof(scratch), &observation) != UMI_STATUS_NOT_FOUND ||
        memcmp(&before, &observation, sizeof(before)) != 0) return 1;
    changed = expected;
    strcpy(changed.arguments, "--reviewed");
    if (umi_project_launch_profile_registry_upsert(registry, &changed) != UMI_STATUS_OK ||
        umi_studio_developer_run_centre_compare_profile(centre, &expected,
            scratch, sizeof(scratch), &observation) != UMI_STATUS_OK ||
        observation.relation != UMI_VALUE_ARCHIVE_DIFFERENT) return 1;
    /* The existing submission case below retains its original profile data. */
    if (umi_project_launch_profile_registry_upsert(registry, &expected) != UMI_STATUS_OK)
        return 1;
    return 0;
}

/* Studio must retain its local revision when Framework refuses a stale review,
 * then expose the queued value from the successful current-profile submission. */
static int RunReviewedSubmissionCases(UmiStudioDeveloperRunCentre *centre)
{
    UmiProjectLaunchProfileSnapshot expected, changed;
    UmiDeveloperRuntime *runtime = umi_studio_developer_run_centre_runtime(centre);
    UmiProjectLaunchProfileRegistry *registry = umi_project_workspace_launch_profile(
        umi_developer_runtime_projects(runtime));
    if (umi_studio_developer_run_centre_at(centre, 0U, &expected) != UMI_STATUS_OK) return 1;
    changed = expected;
    strcpy(changed.arguments, "--changed-after-review");
    if (umi_project_launch_profile_registry_upsert(registry, &changed) != UMI_STATUS_OK) return 1;
    UmiDeveloperLaunchPlanRequest request = {0};
    request.struct_size = sizeof(request);
    request.api_version = UMI_DEVELOPER_LAUNCH_PLAN_API_VERSION;
    request.launch_profile_id = expected.id;
    request.operation_id = "studio.reviewed";
    request.debug_mode = -1;
    unsigned char scratch[2U * sizeof(expected) + 256U];
    UmiDeveloperLaunchPlanSnapshot plan, sentinel;
    memset(&plan, 0x5a, sizeof(plan)); memcpy(&sentinel, &plan, sizeof(plan));
    UmiStudioDeveloperRunCentreSnapshot before, after;
    if (umi_studio_developer_run_centre_snapshot(centre, &before) != UMI_STATUS_OK ||
        umi_studio_developer_run_centre_submit_reviewed(centre, &request, &expected,
            scratch, sizeof(scratch), &plan) != UMI_STATUS_INVALID_STATE ||
        umi_studio_developer_run_centre_snapshot(centre, &after) != UMI_STATUS_OK ||
        before.revision != after.revision || memcmp(&plan, &sentinel, sizeof(plan)) != 0)
        return 1;
    if (umi_studio_developer_run_centre_at(centre, 0U, &expected) != UMI_STATUS_OK ||
        umi_studio_developer_run_centre_submit_reviewed(centre, &request, &expected,
            scratch, sizeof(scratch), &plan) != UMI_STATUS_OK ||
        umi_studio_developer_run_centre_snapshot(centre, &after) != UMI_STATUS_OK ||
        after.revision != before.revision + 1U) return 1;
    UmiDeveloperOperationSnapshot operation;
    if (umi_developer_pipeline_find(umi_developer_runtime_pipeline(runtime),
            plan.operation_id, &operation) != UMI_STATUS_OK ||
        operation.argument_count != 1U ||
        strcmp(operation.arguments[0], "--changed-after-review") != 0) return 1;
    memcpy(&sentinel, &plan, sizeof(plan));
    before = after;
    if (umi_studio_developer_run_centre_submit_reviewed(centre, &request, &expected,
            scratch, sizeof(scratch), &plan) != UMI_STATUS_ALREADY_EXISTS ||
        umi_studio_developer_run_centre_snapshot(centre, &after) != UMI_STATUS_OK ||
        before.revision != after.revision || memcmp(&plan, &sentinel, sizeof(plan)) != 0)
        return 1;
    return 0;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioDeveloperWorkbench *workbench = NULL;
    UmiStudioDeveloperRunCentre *centre;
    UmiStudioDeveloperRunCentreSnapshot snapshot;
    UmiProjectLaunchProfileSnapshot profile = {0};
    UmiDeveloperLaunchPlanRequest request = {0};
    UmiDeveloperLaunchPlanSnapshot plan;

    assert(umi_studio_developer_workbench_create(&workbench) == UMI_STATUS_OK);
    centre = umi_studio_developer_workbench_run_centre(workbench);
    profile.struct_size = (uint32_t)sizeof(profile);
    profile.api_version = UMI_PROJECT_LAUNCH_PROFILE_API_VERSION;
    strcpy(profile.id, "studio.console"); strcpy(profile.project_id, "studio");
    strcpy(profile.name, "Studio Console"); strcpy(profile.program, "umicom-studio-console");
    profile.debug = 1;
    assert(umi_project_launch_profile_registry_upsert(
        umi_project_workspace_launch_profile(umi_developer_runtime_projects(
            umi_studio_developer_run_centre_runtime(centre))), &profile)
        == UMI_STATUS_OK);
    assert(umi_studio_developer_run_centre_snapshot(centre, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.launch_profile_count == 1U);

    if (RunProfileComparisonCases(centre) != 0) {
        umi_studio_developer_workbench_destroy(workbench);
        return 1;
    }
    if (RunReviewedSubmissionCases(centre) != 0) {
        umi_studio_developer_workbench_destroy(workbench);
        return 1;
    }

    request.struct_size = (uint32_t)sizeof(request);
    request.api_version = UMI_DEVELOPER_LAUNCH_PLAN_API_VERSION;
    request.launch_profile_id = "studio.console";
    request.debug_mode = -1;
    assert(umi_studio_developer_run_centre_submit(centre, &request, &plan) == UMI_STATUS_OK);
    assert(plan.operation_kind == UMI_DEVELOPER_OPERATION_DEBUG);

    umi_studio_developer_workbench_destroy(workbench);
    return 0;
}
