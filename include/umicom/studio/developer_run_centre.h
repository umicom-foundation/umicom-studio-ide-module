/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_run_centre.h
 *
 * PURPOSE:
 *   Present project launch profiles and submit run/debug operations through the
 *   shared Framework developer runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEVELOPER_RUN_CENTRE_H
#define UMICOM_STUDIO_DEVELOPER_RUN_CENTRE_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/developer/developer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio developer run centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperRunCentre UmiStudioDeveloperRunCentre;

/**
 * Represent the studio developer run centre snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioDeveloperRunCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char title[256];
    char summary[512];
    size_t launch_profile_count;
    uint64_t launch_profile_revision;
    uint64_t revision;
    int available;
} UmiStudioDeveloperRunCentreSnapshot;

/**
 * Initialise studio developer run centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_run_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperRunCentre **out_centre);
/**
 * Release or reset state held by studio developer run centre so the same storage can be
 * reused safely.
 */
void umi_studio_developer_run_centre_destroy(UmiStudioDeveloperRunCentre *centre);
/**
 * Provide the studio developer run centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_run_centre_snapshot(
    UmiStudioDeveloperRunCentre *centre,
    UmiStudioDeveloperRunCentreSnapshot *out_snapshot);
/**
 * Find studio developer run centre while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_developer_run_centre_at(
    UmiStudioDeveloperRunCentre *centre,
    size_t index,
    UmiProjectLaunchProfileSnapshot *out_profile);
/**
 * Provide the studio developer run centre submit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_run_centre_submit(
    UmiStudioDeveloperRunCentre *centre,
    const UmiDeveloperLaunchPlanRequest *request,
    UmiDeveloperLaunchPlanSnapshot *out_plan);
/**
 * Provide the studio developer run centre runtime operation used by this module and its
 * client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_run_centre_runtime(
    UmiStudioDeveloperRunCentre *centre);


/* Observe whether a profile returned by at/find still has the same saved
 * fields and revision. This delegates to Framework without submitting an
 * operation. See umi_developer_launch_plan_compare_profile for workspace,
 * aliasing, failure and serialization rules. A measured-only result is not
 * equality, and equality does not grant launch authority. */
UmiStatus umi_studio_developer_run_centre_compare_profile(
    UmiStudioDeveloperRunCentre *centre,
    const UmiProjectLaunchProfileSnapshot *expected,
    void *workspace, size_t capacity,
    UmiValueArchiveComparison *out_comparison);


/** Submit the profile shown in a review only while its saved value is current.
 * Delegates all profile admission and queue ownership to Framework. See
 * umi_developer_launch_plan_submit_reviewed for scratch, overrides, aliasing
 * and owner-thread rules. Refusal leaves the centre revision and out_plan
 * unchanged. This adapter does not add a visible review dialog or launch a
 * process; presentation and execution remain separate host responsibilities. */
UmiStatus umi_studio_developer_run_centre_submit_reviewed(
    UmiStudioDeveloperRunCentre *centre,
    const UmiDeveloperLaunchPlanRequest *request,
    const UmiProjectLaunchProfileSnapshot *expected,
    void *workspace, size_t capacity,
    UmiDeveloperLaunchPlanSnapshot *out_plan);

#ifdef __cplusplus
}
#endif
#endif
