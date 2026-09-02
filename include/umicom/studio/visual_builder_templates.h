/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_templates.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder templates contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Templates expand to normal document nodes in one transaction. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_TEMPLATES_H
#define UMICOM_STUDIO_VISUAL_BUILDER_TEMPLATES_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Find studio visual builder template while leaving the underlying catalogue or model
 * owned by this module.
 */
UmiStatus umi_studio_visual_builder_template_find(UmiStudioVisualBuilderCentre *centre,const char *template_id,UmiDesignerTemplate *out_template);
/**
 * Perform studio visual builder template through the module contract so client
 * applications do not duplicate its policy.
 */
UmiStatus umi_studio_visual_builder_template_apply(UmiStudioVisualBuilderCentre *centre,const char *template_id,const char *parent_id,const char *id_prefix);
/**
 * Return the number of records represented by studio visual builder template without
 * changing their state.
 */
size_t umi_studio_visual_builder_template_count(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
