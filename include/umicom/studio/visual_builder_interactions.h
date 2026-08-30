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
/* BEGINNER NOTE: Named commands keep generated interactions inspectable and portable. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_INTERACTIONS_H
#define UMICOM_STUDIO_VISUAL_BUILDER_INTERACTIONS_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
UmiStatus umi_studio_visual_builder_interaction_upsert(UmiStudioVisualBuilderCentre *centre,const UmiDesignerInteraction *interaction);
UmiStatus umi_studio_visual_builder_interaction_remove(UmiStudioVisualBuilderCentre *centre,const char *interaction_id);
UmiStatus umi_studio_visual_builder_interaction_at(UmiStudioVisualBuilderCentre *centre,size_t index,UmiDesignerInteraction *out_interaction);
size_t umi_studio_visual_builder_interaction_count(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
