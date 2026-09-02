/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_CENTRE_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_CENTRE_H
#include "umicom/studio/services.h"
#include "umicom/integration/designer.h"
/**
 * Represent the studio integration designer centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioIntegrationDesignerCentre {
    UmiStudioServices *services;
    UmiIntegrationDesignerFabric fabric;
    UmiIntegrationExecutionPolicy policy;
    char active_view[UMI_INTEGRATION_DESIGNER_ID_CAPACITY];
    uint64_t next_run_sequence;
    uint64_t revision;
} UmiStudioIntegrationDesignerCentre;
/**
 * Represent the studio integration designer snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioIntegrationDesignerSnapshot {
    UmiIntegrationDesignerSnapshot fabric;
    char active_view[UMI_INTEGRATION_DESIGNER_ID_CAPACITY];
    uint64_t revision;
} UmiStudioIntegrationDesignerSnapshot;
/**
 * Initialise studio integration designer from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_integration_designer_create(UmiStudioServices *services,UmiStudioIntegrationDesignerCentre **out_centre);
/**
 * Release or reset state held by studio integration designer so the same storage can be
 * reused safely.
 */
void umi_studio_integration_designer_destroy(UmiStudioIntegrationDesignerCentre *centre);
/**
 * Provide the studio integration designer activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_activate(UmiStudioIntegrationDesignerCentre *centre,const char *view_id);
/**
 * Provide the studio integration designer snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_snapshot(const UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerSnapshot *out_snapshot);
/**
 * Provide the studio integration designer fabric operation used by this module and its
 * client applications.
 */
UmiIntegrationDesignerFabric *umi_studio_integration_designer_fabric(UmiStudioIntegrationDesignerCentre *centre);
#endif
