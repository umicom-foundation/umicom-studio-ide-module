/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/frontend_studio_views.c
 *
 * PURPOSE:
 *   Implement the frontend studio views behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/frontend_studio_views.h"
#include <stdio.h>
#include <string.h>
/*
 * Provide the studio frontend view kind id operation used by this module and its client
 * applications.
 */
const char *umi_studio_frontend_view_kind_id(UmiStudioFrontendViewKind k){/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_FRONTEND_VIEW_OVERVIEW:return "overview";case UMI_STUDIO_FRONTEND_VIEW_FILES:return "files";case UMI_STUDIO_FRONTEND_VIEW_PREVIEW:return "preview";case UMI_STUDIO_FRONTEND_VIEW_DOM:return "dom";case UMI_STUDIO_FRONTEND_VIEW_NETWORK:return "network";case UMI_STUDIO_FRONTEND_VIEW_CONSOLE:return "console";case UMI_STUDIO_FRONTEND_VIEW_BUILD:return "build";case UMI_STUDIO_FRONTEND_VIEW_QUALITY:return "quality";default:return "unknown";}}
/* Provide the title operation used by this module and its client applications. */
static const char *title(UmiStudioFrontendViewKind k){/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_FRONTEND_VIEW_OVERVIEW:return "Frontend Overview";case UMI_STUDIO_FRONTEND_VIEW_FILES:return "Frontend Files";case UMI_STUDIO_FRONTEND_VIEW_PREVIEW:return "Live Preview";case UMI_STUDIO_FRONTEND_VIEW_DOM:return "DOM Inspector";case UMI_STUDIO_FRONTEND_VIEW_NETWORK:return "Network";case UMI_STUDIO_FRONTEND_VIEW_CONSOLE:return "Browser Console";case UMI_STUDIO_FRONTEND_VIEW_BUILD:return "Frontend Build";case UMI_STUDIO_FRONTEND_VIEW_QUALITY:return "Quality Audit";default:return "Frontend";}}
/*
 * Provide the studio frontend view build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_view_build(const UmiStudioFrontendCentre *c,UmiStudioFrontendViewKind k,UmiStudioFrontendView *out){UmiStudioFrontendCentreSnapshot s;UmiStatus st;int n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL||out==NULL||k<UMI_STUDIO_FRONTEND_VIEW_OVERVIEW||k>UMI_STUDIO_FRONTEND_VIEW_QUALITY)return UMI_STATUS_INVALID_ARGUMENT;st=umi_studio_frontend_centre_snapshot(c,&s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;(void)memset(out,0,sizeof(*out));out->kind=k;st=umi_frontend_dev_copy_text(out->id,sizeof(out->id),umi_studio_frontend_view_kind_id(k));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)st=umi_frontend_dev_copy_text(out->title,sizeof(out->title),title(k));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_FRONTEND_VIEW_FILES:out->row_count=s.frontend.document_count;break;case UMI_STUDIO_FRONTEND_VIEW_DOM:out->row_count=s.frontend.dom_node_count;break;case UMI_STUDIO_FRONTEND_VIEW_NETWORK:out->row_count=s.frontend.network_entry_count;break;case UMI_STUDIO_FRONTEND_VIEW_CONSOLE:out->row_count=s.frontend.console_entry_count;break;case UMI_STUDIO_FRONTEND_VIEW_BUILD:out->row_count=s.frontend.build_profile_count;break;default:out->row_count=1U;break;}n=snprintf(out->summary,sizeof(out->summary),"%s: %zu row(s); preview %s; device %s",out->title,out->row_count,s.frontend.preview_url,s.frontend.active_device);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n<0||(size_t)n>=sizeof(out->summary))return UMI_STATUS_CAPACITY_EXCEEDED;out->revision=s.revision;return UMI_STATUS_OK;}
