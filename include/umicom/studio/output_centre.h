/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/output_centre.h
 *
 * PURPOSE:
 *   Compose the Output Centre over Framework workbench services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_OUTPUT_CENTRE_H
#define UMICOM_STUDIO_OUTPUT_CENTRE_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/ui/workbench_services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio output centre data shared with callers of this public contract.
 */
typedef struct UmiStudioOutputCentre UmiStudioOutputCentre;
/**
 * Represent the studio output centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioOutputCentreSnapshot {uint32_t struct_size;uint32_t api_version;char area_id[128];char title[256];char summary[512];size_t item_count;uint64_t revision;int available;} UmiStudioOutputCentreSnapshot;
/**
 * Initialise studio output centre from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_output_centre_create(UmiUiWorkbenchServices *services,UmiStudioOutputCentre **out_centre);
/**
 * Release or reset state held by studio output centre so the same storage can be reused
 * safely.
 */
void umi_studio_output_centre_destroy(UmiStudioOutputCentre *centre);
/**
 * Provide the studio output centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_output_centre_snapshot(UmiStudioOutputCentre *centre,UmiStudioOutputCentreSnapshot *out_snapshot);
/**
 * Provide the studio output centre registry operation used by this module and its client
 * applications.
 */
UmiUiOutputChannelRegistry *umi_studio_output_centre_registry(UmiStudioOutputCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
