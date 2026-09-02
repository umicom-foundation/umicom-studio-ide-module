/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/problems_centre.h
 *
 * PURPOSE:
 *   Compose the Problems Centre over Framework workbench services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PROBLEMS_CENTRE_H
#define UMICOM_STUDIO_PROBLEMS_CENTRE_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/ui/workbench_services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio problems centre data shared with callers of this public contract.
 */
typedef struct UmiStudioProblemsCentre UmiStudioProblemsCentre;
/**
 * Represent the studio problems centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioProblemsCentreSnapshot {uint32_t struct_size;uint32_t api_version;char area_id[128];char title[256];char summary[512];size_t item_count;uint64_t revision;int available;} UmiStudioProblemsCentreSnapshot;
/**
 * Initialise studio problems centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_problems_centre_create(UmiUiWorkbenchServices *services,UmiStudioProblemsCentre **out_centre);
/**
 * Release or reset state held by studio problems centre so the same storage can be reused
 * safely.
 */
void umi_studio_problems_centre_destroy(UmiStudioProblemsCentre *centre);
/**
 * Provide the studio problems centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_problems_centre_snapshot(UmiStudioProblemsCentre *centre,UmiStudioProblemsCentreSnapshot *out_snapshot);
/**
 * Provide the studio problems centre registry operation used by this module and its client
 * applications.
 */
UmiUiProblemRegistry *umi_studio_problems_centre_registry(UmiStudioProblemsCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
