/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_monitoring.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer monitoring contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_MONITORING_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_MONITORING_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * Represent the studio integration designer health data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioIntegrationDesignerHealth {
    UmiIntegrationRunMetrics runs;
    size_t enabled_connections;
    size_t disabled_connections;
    bool healthy;
} UmiStudioIntegrationDesignerHealth;
/**
 * Provide the studio integration designer health operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_health(const UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerHealth *out_health);
/**
 * Perform studio integration designer find through the module contract so client
 * applications do not duplicate its policy.
 */
const UmiIntegrationRunRecord *umi_studio_integration_designer_find_run(const UmiStudioIntegrationDesignerCentre *centre,const char *run_id);
#endif
