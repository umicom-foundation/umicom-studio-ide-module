/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/frontend_studio_preview.c
 *
 * PURPOSE:
 *   Implement the frontend studio preview behavior for
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
#include "umicom/studio/frontend_studio_preview.h"
/* Provide the service operation used by this module and its client applications. */
static UmiFrontendStudio *service(UmiStudioWebPlatform *p){return umi_studio_web_platform_frontend_studio(p);}
/*
 * Provide the studio frontend preview start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_start(UmiStudioWebPlatform *p){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_dev_server_start(&s->server);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK){st=umi_frontend_preview_session_navigate(&s->preview,s->server.base_url);s->revision+=1U;}return st;}
/*
 * Provide the studio frontend preview stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_stop(UmiStudioWebPlatform *p){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_dev_server_stop(&s->server);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK){s->preview.connected=0;s->preview.revision+=1U;s->revision+=1U;}return st;}
/*
 * Provide the studio frontend preview navigate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_preview_navigate(UmiStudioWebPlatform *p,const char *url){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_preview_session_navigate(&s->preview,url);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
/*
 * Provide the studio frontend preview select device operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_preview_select_device(UmiStudioWebPlatform *p,const char *id){UmiFrontendStudio *s=service(p);UmiFrontendDevicePreset d;UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_device_registry_find(&s->devices,id,&d);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)st=umi_frontend_preview_session_set_device(&s->preview,&d);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
/*
 * Provide the studio frontend preview reload operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_preview_reload(UmiStudioWebPlatform *p){UmiFrontendStudio *s=service(p);UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;st=umi_frontend_preview_session_reload(&s->preview);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)s->revision+=1U;return st;}
