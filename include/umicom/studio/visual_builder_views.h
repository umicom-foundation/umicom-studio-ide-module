/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_views.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* A frontend renders view-model properties without owning builder state. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_VIEWS_H
#define UMICOM_STUDIO_VISUAL_BUILDER_VIEWS_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * List the named studio visual builder pane values accepted by this public contract.
 */
typedef enum UmiStudioVisualBuilderPane {
    UMI_STUDIO_VISUAL_BUILDER_PANE_CANVAS=1,
    UMI_STUDIO_VISUAL_BUILDER_PANE_HIERARCHY=2,
    UMI_STUDIO_VISUAL_BUILDER_PANE_PROPERTIES=3,
    UMI_STUDIO_VISUAL_BUILDER_PANE_BINDINGS=4,
    UMI_STUDIO_VISUAL_BUILDER_PANE_INTERACTIONS=5,
    UMI_STUDIO_VISUAL_BUILDER_PANE_TEMPLATES=6,
    UMI_STUDIO_VISUAL_BUILDER_PANE_PREVIEW=7,
    UMI_STUDIO_VISUAL_BUILDER_PANE_SOURCE=8
} UmiStudioVisualBuilderPane;
/**
 * Initialise studio visual builder view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_visual_builder_view_create(UmiStudioVisualBuilderCentre *centre,UmiStudioVisualBuilderPane pane,UmiUiViewModel **out_view);
/**
 * Provide the studio visual builder pane id operation used by this module and its client
 * applications.
 */
const char *umi_studio_visual_builder_pane_id(UmiStudioVisualBuilderPane pane);
#ifdef __cplusplus
}
#endif
#endif
