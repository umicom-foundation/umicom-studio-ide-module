/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_views.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_VIEWS_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_VIEWS_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * List the named studio integration designer view kind values accepted by this public
 * contract.
 */
typedef enum UmiStudioIntegrationDesignerViewKind {
    UMI_STUDIO_INTEGRATION_VIEW_OVERVIEW = 1,
    UMI_STUDIO_INTEGRATION_VIEW_CONNECTIONS,
    UMI_STUDIO_INTEGRATION_VIEW_API_EXPLORER,
    UMI_STUDIO_INTEGRATION_VIEW_MAPPING,
    UMI_STUDIO_INTEGRATION_VIEW_WORKFLOW,
    UMI_STUDIO_INTEGRATION_VIEW_EXECUTION,
    UMI_STUDIO_INTEGRATION_VIEW_MONITORING
} UmiStudioIntegrationDesignerViewKind;
/**
 * Represent the studio integration designer view data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioIntegrationDesignerView {
    UmiStudioIntegrationDesignerViewKind kind;
    char id[UMI_INTEGRATION_DESIGNER_ID_CAPACITY];
    char title[UMI_INTEGRATION_DESIGNER_NAME_CAPACITY];
    char summary[UMI_INTEGRATION_DESIGNER_TEXT_CAPACITY];
    size_t item_count;
    uint64_t revision;
} UmiStudioIntegrationDesignerView;
/**
 * Provide the studio integration designer view id operation used by this module and its
 * client applications.
 */
const char *umi_studio_integration_designer_view_id(UmiStudioIntegrationDesignerViewKind kind);
/**
 * Provide the studio integration designer view build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_view_build(const UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerViewKind kind,UmiStudioIntegrationDesignerView *out_view);
#endif
