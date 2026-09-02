/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/ai_workspace.c
 *
 * PURPOSE:
 *   Implement Studio AI workspace composing AuthorEngine, retrieval, tools and Helix governance.
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
#include "umicom/studio/ai_workspace.h"
#include <string.h>
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char *dst,size_t cap,const char *src){size_t len;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(dst==NULL||cap==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(src==NULL)src="";len=strlen(src);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(len>=cap)len=cap-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(len>0U)memcpy(dst,src,len);dst[len]='\0';}
/*
 * Provide the studio ai workspace snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_workspace_snapshot(UmiStudioServices *services,UmiStudioAiWorkspaceSnapshot *out)
{
    UmiStudioAiPlatform *platform;
    UmiAiAuthorEngineServiceSnapshot integration;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(services==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out)); out->struct_size=(uint32_t)sizeof(*out); out->api_version=1U;
    copy_text(out->area_id,sizeof(out->area_id),"studio.ai-workspace");
    copy_text(out->view_type,sizeof(out->view_type),"studio.ai-workspace");
    copy_text(out->title,sizeof(out->title),"AI Workspace");
    copy_text(out->summary,sizeof(out->summary),"AuthorEngine-orchestrated providers, governed project context, conversation sessions and privacy controls.");
    platform=umi_studio_services_ai_platform(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(platform==NULL)return UMI_STATUS_INVALID_STATE;
    status=umi_studio_ai_platform_snapshot(platform,&integration);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;
    out->revision=integration.revision;
    out->provider_count=integration.providers;
    out->runtime_count=integration.runtimes;
    out->healthy_runtime_count=integration.healthy_runtimes;
    out->context_source_count=integration.context_sources;
    out->session_count=integration.sessions;
    out->item_count=integration.runtimes+integration.context_sources+
                    integration.sessions;
    out->context_limit=integration.context_limit;
    out->reserved_output_tokens=integration.reserved_output_tokens;
    out->remote_allowed=integration.remote_allowed;
    out->persistence_allowed=integration.persistence_allowed;
    copy_text(out->active_session_id,sizeof(out->active_session_id),
              integration.active_session_id);
    out->available=1;
    return status;
}
