/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_interactions.c
 *
 * PURPOSE:
 *   Implement the visual builder interactions behavior for
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
 * Umicom Studio IDE - Visual Builder v2 signal/action/event/command editor
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_interactions.h"
/*
 * Provide the studio visual builder interaction upsert operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_visual_builder_interaction_upsert(UmiStudioVisualBuilderCentre *centre,const UmiDesignerInteraction *interaction){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_interaction_editor_upsert(umi_designer_builder_session_interactions(session),umi_designer_builder_session_document(session),interaction);}
/*
 * Remove studio visual builder interaction while keeping the remaining records in a valid
 * and discoverable state.
 */
UmiStatus umi_studio_visual_builder_interaction_remove(UmiStudioVisualBuilderCentre *centre,const char *interaction_id){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_interaction_editor_remove(umi_designer_builder_session_interactions(session),interaction_id):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Find studio visual builder interaction while leaving the underlying catalogue or model
 * owned by this module.
 */
UmiStatus umi_studio_visual_builder_interaction_at(UmiStudioVisualBuilderCentre *centre,size_t index,UmiDesignerInteraction *out_interaction){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_interaction_editor_at(umi_designer_builder_session_interactions(session),index,out_interaction):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Return the number of records represented by studio visual builder interaction without
 * changing their state.
 */
size_t umi_studio_visual_builder_interaction_count(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_interaction_editor_count(umi_designer_builder_session_interactions(session)):0U;}
