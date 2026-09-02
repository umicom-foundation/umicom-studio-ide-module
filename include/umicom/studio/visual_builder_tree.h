/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_tree.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder tree contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* The displayed tree is rebuilt from the Framework semantic document. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_TREE_H
#define UMICOM_STUDIO_VISUAL_BUILDER_TREE_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio visual builder tree refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_refresh(UmiStudioVisualBuilderCentre *centre,UmiDesignerComponentTree *out_tree);
/**
 * Provide the studio visual builder tree select operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_select(UmiStudioVisualBuilderCentre *centre,const char *node_id,int extend_selection);
/**
 * Provide the studio visual builder tree reparent operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_tree_reparent(UmiStudioVisualBuilderCentre *centre,const char *node_id,const char *parent_id,uint32_t sibling_index);
#ifdef __cplusplus
}
#endif
#endif
