/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_properties.c
 *
 * PURPOSE:
 *   Implement the visual builder properties behavior for
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
 * Umicom Studio IDE - Visual Builder v2 typed property inspector
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_properties.h"
/*
 * Provide the studio visual builder property begin operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_property_begin(UmiStudioVisualBuilderCentre *centre,const char *node_id,const char *property_name,UmiDesignerPropertyDraft *out_draft){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_property_editor_begin(umi_designer_builder_session_document(session),umi_designer_builder_session_components(session),node_id,property_name,out_draft);}
/*
 * Copy studio visual builder property into module-owned storage so callers keep ownership
 * of their input values.
 */
UmiStatus umi_studio_visual_builder_property_set(UmiDesignerPropertyDraft *draft,const char *value_text){return umi_designer_property_editor_set(draft,value_text);}
/*
 * Provide the studio visual builder property commit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_property_commit(UmiStudioVisualBuilderCentre *centre,const UmiDesignerPropertyDraft *draft){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_property_editor_commit(umi_designer_builder_session_history(session),umi_designer_builder_session_document(session),draft);}
