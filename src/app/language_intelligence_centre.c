/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/language_intelligence_centre.c
 *
 * PURPOSE:
 *   Implement the Language Intelligence Centre composition.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/language_intelligence_centre.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioLanguageIntelligenceCentre { UmiLanguageService *service; uint64_t revision; };
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char*d,size_t c,const char*s){size_t n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||c==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)s="";n=strlen(s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>=c)n=c-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>0U)memcpy(d,s,n);d[n]='\0';}
/*
 * Initialise studio language intelligence centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_language_intelligence_centre_create(UmiStudioLanguageIntelligenceCentre **out){UmiStudioLanguageIntelligenceCentre*p;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;s=umi_language_service_create(&p->service);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){free(p);return s;}p->revision=1U;*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio language intelligence centre so the same storage
 * can be reused safely.
 */
void umi_studio_language_intelligence_centre_destroy(UmiStudioLanguageIntelligenceCentre*p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;umi_language_service_destroy(p->service);free(p);}
/*
 * Provide the studio language intelligence centre snapshot operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_language_intelligence_centre_snapshot(UmiStudioLanguageIntelligenceCentre*p,UmiStudioLanguageIntelligenceCentreSnapshot*o){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;copy_text(o->area_id,sizeof(o->area_id),"studio.language-intelligence-centre");copy_text(o->title,sizeof(o->title),"Language Intelligence Centre");copy_text(o->summary,sizeof(o->summary),"Provider-neutral completion, symbols, navigation, diagnostics, refactoring and semantic information.");s=umi_language_service_snapshot(p->service,&o->service);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;o->revision=p->revision;o->available=1;return UMI_STATUS_OK;}
/*
 * Provide the studio language intelligence centre service operation used by this module
 * and its client applications.
 */
UmiLanguageService *umi_studio_language_intelligence_centre_service(UmiStudioLanguageIntelligenceCentre*p){return p!=NULL?p->service:NULL;}
