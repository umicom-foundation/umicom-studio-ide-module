/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_preview.c
 *
 * PURPOSE:
 *   Implement the visual builder preview behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Umicom Studio IDE - Visual Builder v2 live-preview adapter
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_preview.h"
/*
 * Provide the studio visual builder preview refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_preview_refresh(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_builder_session_preview(session):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio visual builder preview profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_preview_profile(UmiStudioVisualBuilderCentre *centre,UmiDesignerPreviewProfile profile){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);UmiDesignerLivePreview *preview=session!=NULL?umi_designer_builder_session_live_preview(session):NULL;return preview!=NULL?umi_designer_preview_apply_profile(&preview->viewport,profile):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio visual builder preview state operation used by this module and its
 * client applications.
 */
const UmiDesignerLivePreview *umi_studio_visual_builder_preview_state(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_builder_session_live_preview(session):NULL;}
