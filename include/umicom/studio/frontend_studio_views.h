/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_views.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_VIEWS_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_VIEWS_H
#include "umicom/studio/frontend_studio_centre.h"
/**
 * List the named studio frontend view kind values accepted by this public contract.
 */
typedef enum UmiStudioFrontendViewKind { UMI_STUDIO_FRONTEND_VIEW_OVERVIEW=1, UMI_STUDIO_FRONTEND_VIEW_FILES=2, UMI_STUDIO_FRONTEND_VIEW_PREVIEW=3, UMI_STUDIO_FRONTEND_VIEW_DOM=4, UMI_STUDIO_FRONTEND_VIEW_NETWORK=5, UMI_STUDIO_FRONTEND_VIEW_CONSOLE=6, UMI_STUDIO_FRONTEND_VIEW_BUILD=7, UMI_STUDIO_FRONTEND_VIEW_QUALITY=8 } UmiStudioFrontendViewKind;
/**
 * Represent the studio frontend view data shared with callers of this public contract.
 */
typedef struct UmiStudioFrontendView { UmiStudioFrontendViewKind kind; char id[UMI_FRONTEND_DEV_ID_CAPACITY]; char title[UMI_FRONTEND_DEV_NAME_CAPACITY]; char summary[UMI_FRONTEND_DEV_TEXT_CAPACITY]; size_t row_count; uint64_t revision; } UmiStudioFrontendView;
/**
 * Provide the studio frontend view build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_view_build(const UmiStudioFrontendCentre *centre,UmiStudioFrontendViewKind kind,UmiStudioFrontendView *out_view);
/**
 * Provide the studio frontend view kind id operation used by this module and its client
 * applications.
 */
const char *umi_studio_frontend_view_kind_id(UmiStudioFrontendViewKind kind);
#endif
