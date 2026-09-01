/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_preview.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder preview contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Preview health exposes current, stale and invalid states honestly. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_PREVIEW_H
#define UMICOM_STUDIO_VISUAL_BUILDER_PREVIEW_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
UmiStatus umi_studio_visual_builder_preview_refresh(UmiStudioVisualBuilderCentre *centre);
UmiStatus umi_studio_visual_builder_preview_profile(UmiStudioVisualBuilderCentre *centre,UmiDesignerPreviewProfile profile);
const UmiDesignerLivePreview *umi_studio_visual_builder_preview_state(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
