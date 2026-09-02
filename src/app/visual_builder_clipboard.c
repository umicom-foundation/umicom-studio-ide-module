/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_clipboard.c
 *
 * PURPOSE:
 *   Implement the visual builder clipboard behavior for
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
 * Umicom Studio IDE - Visual Builder v2 clipboard coordinator
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_clipboard.h"
/*
 * Copy studio visual builder into module-owned storage so callers keep ownership of their
 * input values.
 */
UmiStatus umi_studio_visual_builder_copy(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_clipboard_copy(umi_designer_builder_session_clipboard(session),umi_designer_builder_session_document(session),umi_designer_builder_session_selection(session));}
/*
 * Provide the studio visual builder paste operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_visual_builder_paste(UmiStudioVisualBuilderCentre *centre,const char *parent_id,const char *id_prefix){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(session==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_designer_clipboard_paste(umi_designer_builder_session_clipboard(session),umi_designer_builder_session_document(session),umi_designer_builder_session_transactions(session),parent_id,id_prefix);}
/*
 * Return the number of records represented by studio visual builder clipboard without
 * changing their state.
 */
size_t umi_studio_visual_builder_clipboard_count(UmiStudioVisualBuilderCentre *centre){UmiDesignerBuilderSession *session=umi_studio_visual_builder_centre_session(centre);UmiDesignerSemanticClipboard *clipboard=session!=NULL?umi_designer_builder_session_clipboard(session):NULL;return clipboard!=NULL?clipboard->node_count:0U;}
