/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/progress_centre.h
 *
 * PURPOSE:
 *   Compose the Progress Centre over Framework workbench services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PROGRESS_CENTRE_H
#define UMICOM_STUDIO_PROGRESS_CENTRE_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/ui/workbench_services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio progress centre data shared with callers of this public contract.
 */
typedef struct UmiStudioProgressCentre UmiStudioProgressCentre;
/**
 * Represent the studio progress centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioProgressCentreSnapshot {uint32_t struct_size;uint32_t api_version;char area_id[128];char title[256];char summary[512];size_t item_count;uint64_t revision;int available;} UmiStudioProgressCentreSnapshot;
/**
 * Initialise studio progress centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_progress_centre_create(UmiUiWorkbenchServices *services,UmiStudioProgressCentre **out_centre);
/**
 * Release or reset state held by studio progress centre so the same storage can be reused
 * safely.
 */
void umi_studio_progress_centre_destroy(UmiStudioProgressCentre *centre);
/**
 * Provide the studio progress centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_progress_centre_snapshot(UmiStudioProgressCentre *centre,UmiStudioProgressCentreSnapshot *out_snapshot);
/**
 * Provide the studio progress centre registry operation used by this module and its client
 * applications.
 */
UmiUiProgressRegistry *umi_studio_progress_centre_registry(UmiStudioProgressCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
