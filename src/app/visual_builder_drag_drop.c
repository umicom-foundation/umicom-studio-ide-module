/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_drag_drop.c
 *
 * PURPOSE:
 *   Implement the visual builder drag drop behavior for
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
 * Umicom Studio IDE - Visual Builder v2 drag/drop coordinator
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_drag_drop.h"
/*
 * Provide the studio visual builder drag palette operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_drag_palette(UmiDesignerDragDrop *drag,const char *component_type,const char *node_id){return umi_designer_drag_drop_begin_palette(drag,component_type,node_id);}
/*
 * Provide the studio visual builder drag node operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_visual_builder_drag_node(UmiDesignerDragDrop *drag,const char *node_id){return umi_designer_drag_drop_begin_node(drag,node_id);}
/*
 * Provide the studio visual builder drag target operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_drag_target(UmiDesignerDragDrop *drag,const char *target_id,UmiDesignerDropPosition position){return umi_designer_drag_drop_target(drag,target_id,position);}
/*
 * Provide the studio visual builder drop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_visual_builder_drop(UmiStudioVisualBuilderCentre *centre,UmiDesignerDragDrop *drag){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_drag_drop_commit(drag,umi_designer_builder_session_history(session),umi_designer_builder_session_document(session));}
