/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_task_centre.h
 *
 * PURPOSE:
 *   Present project tasks through the Studio developer workbench and submit
 *   selected tasks into the shared Framework developer pipeline.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The centre borrows the existing UmiDeveloperRuntime.  It does not create a
 * second project workspace, so tasks shown here are the same tasks visible to
 * the Project Centre and to future automation commands.
 */
#ifndef UMICOM_STUDIO_DEVELOPER_TASK_CENTRE_H
#define UMICOM_STUDIO_DEVELOPER_TASK_CENTRE_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/developer/developer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio developer task centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperTaskCentre UmiStudioDeveloperTaskCentre;

/**
 * Represent the studio developer task centre snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioDeveloperTaskCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char title[256];
    char summary[512];
    size_t task_count;
    uint64_t task_revision;
    uint64_t revision;
    int available;
} UmiStudioDeveloperTaskCentreSnapshot;

/**
 * Initialise studio developer task centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_task_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperTaskCentre **out_centre);
/**
 * Release or reset state held by studio developer task centre so the same storage can be
 * reused safely.
 */
void umi_studio_developer_task_centre_destroy(UmiStudioDeveloperTaskCentre *centre);
/**
 * Provide the studio developer task centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_task_centre_snapshot(
    UmiStudioDeveloperTaskCentre *centre,
    UmiStudioDeveloperTaskCentreSnapshot *out_snapshot);
/**
 * Find studio developer task centre while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_developer_task_centre_at(
    UmiStudioDeveloperTaskCentre *centre,
    size_t index,
    UmiProjectTaskSnapshot *out_task);
/**
 * Provide the studio developer task centre submit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_task_centre_submit(
    UmiStudioDeveloperTaskCentre *centre,
    const UmiDeveloperTaskPlanRequest *request,
    UmiDeveloperTaskPlanSnapshot *out_plan);
/**
 * Provide the studio developer task centre runtime operation used by this module and its
 * client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_task_centre_runtime(
    UmiStudioDeveloperTaskCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
