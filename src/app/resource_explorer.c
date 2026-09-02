/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/resource_explorer.c
 *
 * PURPOSE:
 *   Implement Studio resource-explorer composition over Framework resource navigation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#include "umicom/studio/resource_explorer.h"
#include <string.h>
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char *dst,size_t cap,const char *src){size_t len;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(dst==NULL||cap==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(src==NULL)src="";len=strlen(src);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(len>=cap)len=cap-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(len>0U)memcpy(dst,src,len);dst[len]='\0';}
/*
 * Provide the studio resource explorer snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_resource_explorer_snapshot(UmiStudioServices *services,UmiStudioResourceExplorerSnapshot *out)
{
    (void)services;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out)); out->struct_size=(uint32_t)sizeof(*out); out->api_version=1U;
    copy_text(out->area_id,sizeof(out->area_id),"studio.resource-explorer");
    copy_text(out->view_type,sizeof(out->view_type),"studio.resource-explorer");
    copy_text(out->title,sizeof(out->title),"ResourceExplorer");
    copy_text(out->summary,sizeof(out->summary),"Files, folders, workspaces, bookmarks and recent resources.");
    out->revision=1U; out->available=1;
    return UMI_STATUS_OK;
}
