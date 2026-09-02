/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_source.c
 *
 * PURPOSE:
 *   Implement the visual builder source behavior for
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
 * Umicom Studio IDE - Visual Builder v2 transparent source adapter
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_source.h"
/*
 * Provide the studio visual builder source refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_source_refresh(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_builder_session_generate(session):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Find studio visual builder source while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiDesignerGeneratedFile *umi_studio_visual_builder_source_find(UmiStudioVisualBuilderCentre *centre,const char *path){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);return session!=NULL?umi_designer_source_generation_find(umi_designer_builder_session_generated_source(session),path):NULL;}
/*
 * Return the number of records represented by studio visual builder source without
 * changing their state.
 */
size_t umi_studio_visual_builder_source_count(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);const UmiDesignerGeneratedSource *source=session!=NULL?umi_designer_builder_session_generated_source(session):NULL;return source!=NULL?source->file_count:0U;}
