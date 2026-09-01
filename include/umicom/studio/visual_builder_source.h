/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/visual_builder_source.h
 *
 * PURPOSE:
 *   Publish Studio's public visual builder source contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Every generated file remains visible for review, saving and diffing. */
#ifndef UMICOM_STUDIO_VISUAL_BUILDER_SOURCE_H
#define UMICOM_STUDIO_VISUAL_BUILDER_SOURCE_H
#include "umicom/studio/visual_builder_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
UmiStatus umi_studio_visual_builder_source_refresh(UmiStudioVisualBuilderCentre *centre);
const UmiDesignerGeneratedFile *umi_studio_visual_builder_source_find(UmiStudioVisualBuilderCentre *centre,const char *path);
size_t umi_studio_visual_builder_source_count(UmiStudioVisualBuilderCentre *centre);
#ifdef __cplusplus
}
#endif
#endif
