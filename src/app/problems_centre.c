/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/problems_centre.c
 *
 * PURPOSE:
 *   Implement the Problems Centre.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/problems_centre.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioProblemsCentre{UmiUiWorkbenchServices*services;uint64_t revision;};
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char*d,size_t c,const char*s){size_t n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||c==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)s="";n=strlen(s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>=c)n=c-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>0U)memcpy(d,s,n);d[n]='\0';}
/*
 * Initialise studio problems centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_problems_centre_create(UmiUiWorkbenchServices*s,UmiStudioProblemsCentre**out){UmiStudioProblemsCentre*p;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;p->services=s;p->revision=1U;*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio problems centre so the same storage can be reused
 * safely.
 */
void umi_studio_problems_centre_destroy(UmiStudioProblemsCentre*p){free(p);}
/*
 * Provide the studio problems centre registry operation used by this module and its client
 * applications.
 */
UmiUiProblemRegistry *umi_studio_problems_centre_registry(UmiStudioProblemsCentre*p){return p!=NULL?umi_ui_workbench_services_problem(p->services):NULL;}
/*
 * Provide the studio problems centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_problems_centre_snapshot(UmiStudioProblemsCentre*p,UmiStudioProblemsCentreSnapshot*o){UmiUiProblemRegistry*r;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;copy_text(o->area_id,sizeof(o->area_id),"studio.problems-centre");copy_text(o->title,sizeof(o->title),"Problems Centre");copy_text(o->summary,sizeof(o->summary),"Unified build, language, runtime and validation problems.");r=umi_studio_problems_centre_registry(p);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(r==NULL)return UMI_STATUS_INVALID_STATE;o->item_count=umi_ui_problem_registry_count(r);o->revision=p->revision;o->available=1;return UMI_STATUS_OK;}
