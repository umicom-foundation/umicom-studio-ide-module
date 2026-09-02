/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/source_control_centre.h
 *
 * PURPOSE:
 *   Compose the Source Control Centre over reusable Umicom Framework services.
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
#ifndef UMICOM_STUDIO_SOURCE_CONTROL_CENTRE_H
#define UMICOM_STUDIO_SOURCE_CONTROL_CENTRE_H
#include <stdint.h>
#include "umicom/source_control/service.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio source control centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioSourceControlCentre UmiStudioSourceControlCentre;
/**
 * Represent the studio source control centre snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioSourceControlCentreSnapshot {
    uint32_t struct_size; uint32_t api_version;
    char area_id[128]; char title[256]; char summary[512];
    UmiSourceControlServiceSnapshot service; uint64_t revision; int available;
} UmiStudioSourceControlCentreSnapshot;
/**
 * Initialise studio source control centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_source_control_centre_create(UmiStudioSourceControlCentre **out_centre);
/**
 * Release or reset state held by studio source control centre so the same storage can be
 * reused safely.
 */
void umi_studio_source_control_centre_destroy(UmiStudioSourceControlCentre *centre);
/**
 * Provide the studio source control centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_centre_snapshot(UmiStudioSourceControlCentre *centre,UmiStudioSourceControlCentreSnapshot *out_snapshot);
/**
 * Provide the studio source control centre service operation used by this module and its
 * client applications.
 */
UmiSourceControlService *umi_studio_source_control_centre_service(UmiStudioSourceControlCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
