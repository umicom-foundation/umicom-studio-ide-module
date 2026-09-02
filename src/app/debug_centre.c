/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/debug_centre.c
 *
 * PURPOSE:
 *   Implement the Debug Centre composition.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/debug_centre.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioDebugCentre { UmiDebugService *service; uint64_t revision; };
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char*d,size_t c,const char*s){size_t n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||c==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)s="";n=strlen(s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>=c)n=c-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>0U)memcpy(d,s,n);d[n]='\0';}
/*
 * Initialise studio debug centre from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_debug_centre_create(UmiStudioDebugCentre **out){UmiStudioDebugCentre*p;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;s=umi_debug_service_create(&p->service);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){free(p);return s;}p->revision=1U;*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio debug centre so the same storage can be reused
 * safely.
 */
void umi_studio_debug_centre_destroy(UmiStudioDebugCentre*p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;umi_debug_service_destroy(p->service);free(p);}
/*
 * Provide the studio debug centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_debug_centre_snapshot(UmiStudioDebugCentre*p,UmiStudioDebugCentreSnapshot*o){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;copy_text(o->area_id,sizeof(o->area_id),"studio.debug-centre");copy_text(o->title,sizeof(o->title),"Debug Centre");copy_text(o->summary,sizeof(o->summary),"Launch configurations, sessions, breakpoints, stacks, variables, watches and debugger events.");s=umi_debug_service_snapshot(p->service,&o->service);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;o->revision=p->revision;o->available=1;return UMI_STATUS_OK;}
/*
 * Provide the studio debug centre service operation used by this module and its client
 * applications.
 */
UmiDebugService *umi_studio_debug_centre_service(UmiStudioDebugCentre*p){return p!=NULL?p->service:NULL;}
