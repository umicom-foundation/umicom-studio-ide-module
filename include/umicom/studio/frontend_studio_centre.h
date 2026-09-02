/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_CENTRE_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_CENTRE_H
#include "umicom/studio/frontend_studio_settings.h"
/**
 * Represent the studio frontend centre data shared with callers of this public contract.
 */
typedef struct UmiStudioFrontendCentre { UmiStudioWebPlatform *platform; char active_pane[UMI_FRONTEND_DEV_ID_CAPACITY]; UmiFrontendQualityResult latest_audit; uint64_t revision; } UmiStudioFrontendCentre;
/**
 * Represent the studio frontend centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioFrontendCentreSnapshot { UmiFrontendStudioSnapshot frontend; char active_pane[UMI_FRONTEND_DEV_ID_CAPACITY]; UmiFrontendQualityResult latest_audit; uint64_t revision; } UmiStudioFrontendCentreSnapshot;
/**
 * Initialise studio frontend centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_frontend_centre_init(UmiStudioFrontendCentre *centre,UmiStudioWebPlatform *platform);
/**
 * Provide the studio frontend centre activate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_centre_activate(UmiStudioFrontendCentre *centre,const char *pane_id);
/**
 * Provide the studio frontend centre open document operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_centre_open_document(UmiStudioFrontendCentre *centre,const UmiFrontendDocument *document);
/**
 * Provide the studio frontend centre publish operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_centre_publish(UmiStudioFrontendCentre *centre,uint64_t *out_generation);
/**
 * Provide the studio frontend centre audit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_centre_audit(UmiStudioFrontendCentre *centre,const UmiFrontendQualityMetrics *metrics);
/**
 * Provide the studio frontend centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_centre_snapshot(const UmiStudioFrontendCentre *centre,UmiStudioFrontendCentreSnapshot *out_snapshot);
#endif
