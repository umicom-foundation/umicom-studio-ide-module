/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/frontend_studio_inspector.c
 *
 * PURPOSE:
 *   Implement the frontend studio inspector behavior for
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
#include "umicom/studio/frontend_studio_inspector.h"
/* Provide the service operation used by this module and its client applications. */
static UmiFrontendStudio *service(UmiStudioWebPlatform *p){return umi_studio_web_platform_frontend_studio(p);}
/*
 * Provide the studio frontend inspector capture node operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_inspector_capture_node(UmiStudioWebPlatform *p,const UmiFrontendDomNode *n){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_dom_inspector_upsert(&s->dom,n);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
/*
 * Provide the studio frontend inspector select node operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_inspector_select_node(UmiStudioWebPlatform *p,const char *id,UmiFrontendDomNode *out){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_dom_inspector_select(&s->dom,id);return st==UMI_STATUS_OK?umi_frontend_dom_inspector_selected(&s->dom,out):st;}
/*
 * Provide the studio frontend inspector record network operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_frontend_inspector_record_network(UmiStudioWebPlatform *p,const UmiFrontendNetworkEntry *e){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_network_log_append(&s->network,e);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
/*
 * Provide the studio frontend inspector record console operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_frontend_inspector_record_console(UmiStudioWebPlatform *p,const UmiFrontendConsoleEntry *e){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_console_log_append(&s->console,e);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
/*
 * Release or reset state held by studio frontend inspector so the same storage can be
 * reused safely.
 */
void umi_studio_frontend_inspector_clear(UmiStudioWebPlatform *p){UmiFrontendStudio *s=service(p);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return;umi_frontend_network_log_clear(&s->network);umi_frontend_console_log_clear(&s->console);s->revision+=1U;}
