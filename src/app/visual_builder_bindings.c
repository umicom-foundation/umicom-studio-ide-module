/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_bindings.c
 *
 * PURPOSE:
 *   Implement the visual builder bindings behavior for
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
 * Umicom Studio IDE - Visual Builder v2 property-binding editor
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_bindings.h"
/*
 * Provide the studio visual builder binding upsert operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_binding_upsert(UmiStudioVisualBuilderCentre *centre,const UmiDesignerBinding *binding){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_binding_editor_upsert(umi_designer_builder_session_bindings(session),umi_designer_builder_session_document(session),umi_designer_builder_session_components(session),binding);}
/*
 * Remove studio visual builder binding while keeping the remaining records in a valid and
 * discoverable state.
 */
UmiStatus umi_studio_visual_builder_binding_remove(UmiStudioVisualBuilderCentre *centre,const char *binding_id){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_binding_editor_remove(umi_designer_builder_session_bindings(session),binding_id):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Find studio visual builder binding while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_visual_builder_binding_at(UmiStudioVisualBuilderCentre *centre,size_t index,UmiDesignerBinding *out_binding){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_binding_editor_at(umi_designer_builder_session_bindings(session),index,out_binding):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Return the number of records represented by studio visual builder binding without
 * changing their state.
 */
size_t umi_studio_visual_builder_binding_count(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_binding_editor_count(umi_designer_builder_session_bindings(session)):0U;}
