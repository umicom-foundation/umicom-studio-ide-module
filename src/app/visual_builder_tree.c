/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_tree.c
 *
 * PURPOSE:
 *   Implement the visual builder tree behavior for
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
 * Umicom Studio IDE - Visual Builder v2 component hierarchy adapter
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_tree.h"
/*
 * Provide the studio visual builder tree refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_refresh(UmiStudioVisualBuilderCentre *centre,UmiDesignerComponentTree *out_tree){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_builder_session_tree(session,out_tree):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio visual builder tree select operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_select(UmiStudioVisualBuilderCentre *centre,const char *node_id,int extend_selection){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_builder_session_select(session,node_id,extend_selection):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio visual builder tree reparent operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_reparent(UmiStudioVisualBuilderCentre *centre,const char *node_id,const char *parent_id,uint32_t sibling_index){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_component_tree_reparent(umi_designer_builder_session_history(session),umi_designer_builder_session_document(session),node_id,parent_id,sibling_index);}
