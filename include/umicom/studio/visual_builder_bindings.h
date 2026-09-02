/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_bindings.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder bindings contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Studio edits stable Framework binding records, not hidden widget state. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_BINDINGS_H
#define UMICOM_STUDIO_VISUAL_BUILDER_BINDINGS_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio visual builder binding upsert operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_binding_upsert(UmiStudioVisualBuilderCentre *centre,const UmiDesignerBinding *binding);
/**
 * Remove studio visual builder binding while keeping the remaining records in a valid and
 * discoverable state.
 */
UmiStatus umi_studio_visual_builder_binding_remove(UmiStudioVisualBuilderCentre *centre,const char *binding_id);
/**
 * Find studio visual builder binding while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_visual_builder_binding_at(UmiStudioVisualBuilderCentre *centre,size_t index,UmiDesignerBinding *out_binding);
/**
 * Return the number of records represented by studio visual builder binding without
 * changing their state.
 */
size_t umi_studio_visual_builder_binding_count(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
