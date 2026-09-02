/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_drag_drop.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder drag drop contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Frontends report a completed semantic drop; Framework owns the edit. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_DRAG_DROP_H
#define UMICOM_STUDIO_VISUAL_BUILDER_DRAG_DROP_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio visual builder drag palette operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_drag_palette(UmiDesignerDragDrop *drag,const char *component_type,const char *node_id);
/**
 * Provide the studio visual builder drag node operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_visual_builder_drag_node(UmiDesignerDragDrop *drag,const char *node_id);
/**
 * Provide the studio visual builder drag target operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_drag_target(UmiDesignerDragDrop *drag,const char *target_id,UmiDesignerDropPosition position);
/**
 * Provide the studio visual builder drop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_visual_builder_drop(UmiStudioVisualBuilderCentre *centre,UmiDesignerDragDrop *drag);
#ifdef __cplusplus
}
#endif
#endif
