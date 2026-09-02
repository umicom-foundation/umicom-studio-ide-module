/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_centre.c
 *
 * PURPOSE:
 *   Implement the visual builder centre behavior for
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
 * Umicom Studio IDE - Visual Application Builder v2 composition centre
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_centre.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioVisualBuilderCentre { UmiDesignerBuilderSession *session; char active_pane[UMI_DECL_ID_CAPACITY]; uint64_t revision; };
/*
 * Provide the studio visual builder capability id operation used by this module and its
 * client applications.
 */
const char *umi_studio_visual_builder_capability_id(void){return "umicom.studio.visual-application-builder";}
/*
 * Initialise studio visual builder centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_visual_builder_centre_create(const char *application_id,UmiStudioVisualBuilderCentre **out_centre)
{
    UmiStudioVisualBuilderCentre *centre; UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(application_id==NULL||out_centre==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre=NULL;
    centre=calloc(1U,sizeof(*centre)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if(centre==NULL)return UMI_STATUS_OUT_OF_MEMORY;
    status=umi_designer_builder_session_create(application_id,&centre->session);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_decl_copy_text(centre->active_pane,sizeof(centre->active_pane),"canvas");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK){umi_designer_builder_session_destroy(centre->session);free(centre);return status;}
    centre->revision=1U; *out_centre=centre; return UMI_STATUS_OK;
}
/*
 * Release or reset state held by studio visual builder centre so the same storage can be
 * reused safely.
 */
void umi_studio_visual_builder_centre_destroy(UmiStudioVisualBuilderCentre *centre){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(centre==NULL)return;umi_designer_builder_session_destroy(centre->session);free(centre);}
/*
 * Provide the studio visual builder centre session operation used by this module and its
 * client applications.
 */
UmiDesignerBuilderSession *umi_studio_visual_builder_centre_session(UmiStudioVisualBuilderCentre *centre){return centre!=NULL?centre->session:NULL;}
/*
 * Provide the studio visual builder centre activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_centre_activate(UmiStudioVisualBuilderCentre *centre,const char *pane_id){UmiStatus status;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(centre==NULL||pane_id==NULL)return UMI_STATUS_INVALID_ARGUMENT;status=umi_decl_copy_text(centre->active_pane,sizeof(centre->active_pane),pane_id);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(status==UMI_STATUS_OK)centre->revision+=1U;return status;}
/*
 * Provide the studio visual builder centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_visual_builder_centre_snapshot(const UmiStudioVisualBuilderCentre *centre,UmiStudioVisualBuilderSnapshot *out_snapshot){UmiStatus status;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(centre==NULL||out_snapshot==NULL)return UMI_STATUS_INVALID_ARGUMENT;(void)memset(out_snapshot,0,sizeof(*out_snapshot));status=umi_designer_builder_session_snapshot(centre->session,&out_snapshot->builder);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(status==UMI_STATUS_OK)status=umi_decl_copy_text(out_snapshot->active_pane,sizeof(out_snapshot->active_pane),centre->active_pane);out_snapshot->revision=centre->revision;return status;}
