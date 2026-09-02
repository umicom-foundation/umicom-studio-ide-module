/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_interactions.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder interactions contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Named commands keep generated interactions inspectable and portable. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_INTERACTIONS_H
#define UMICOM_STUDIO_VISUAL_BUILDER_INTERACTIONS_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Provide the studio visual builder interaction upsert operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_visual_builder_interaction_upsert(UmiStudioVisualBuilderCentre *centre,const UmiDesignerInteraction *interaction);
/**
 * Remove studio visual builder interaction while keeping the remaining records in a valid
 * and discoverable state.
 */
UmiStatus umi_studio_visual_builder_interaction_remove(UmiStudioVisualBuilderCentre *centre,const char *interaction_id);
/**
 * Find studio visual builder interaction while leaving the underlying catalogue or model
 * owned by this module.
 */
UmiStatus umi_studio_visual_builder_interaction_at(UmiStudioVisualBuilderCentre *centre,size_t index,UmiDesignerInteraction *out_interaction);
/**
 * Return the number of records represented by studio visual builder interaction without
 * changing their state.
 */
size_t umi_studio_visual_builder_interaction_count(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
