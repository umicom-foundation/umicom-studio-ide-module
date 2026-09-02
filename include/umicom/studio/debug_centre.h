/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/debug_centre.h
 *
 * PURPOSE:
 *   Compose the Debug Centre over reusable Umicom Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns product composition only. The reusable data model and service
 * remain in Umicom Framework so future applications can consume the same API.
 */
#ifndef UMICOM_STUDIO_DEBUG_CENTRE_H
#define UMICOM_STUDIO_DEBUG_CENTRE_H
#include <stdint.h>
#include "umicom/debug/service.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio debug centre data shared with callers of this public contract.
 */
typedef struct UmiStudioDebugCentre UmiStudioDebugCentre;
/**
 * Represent the studio debug centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDebugCentreSnapshot {
    uint32_t struct_size; uint32_t api_version;
    char area_id[128]; char title[256]; char summary[512];
    UmiDebugServiceSnapshot service; uint64_t revision; int available;
} UmiStudioDebugCentreSnapshot;
/**
 * Initialise studio debug centre from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_debug_centre_create(UmiStudioDebugCentre **out_centre);
/**
 * Release or reset state held by studio debug centre so the same storage can be reused
 * safely.
 */
void umi_studio_debug_centre_destroy(UmiStudioDebugCentre *centre);
/**
 * Provide the studio debug centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debug_centre_snapshot(UmiStudioDebugCentre *centre,UmiStudioDebugCentreSnapshot *out_snapshot);
/**
 * Provide the studio debug centre service operation used by this module and its client
 * applications.
 */
UmiDebugService *umi_studio_debug_centre_service(UmiStudioDebugCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
