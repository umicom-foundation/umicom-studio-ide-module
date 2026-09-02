/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/web_platform.c
 *
 * PURPOSE:
 *   Compose the reusable Framework web, browser, charting and frontend services for Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns how these services are combined, but it does not reimplement their generic algorithms.
 */

#include "umicom/studio/web_platform.h"
#include "umicom/studio/web.h"
#include "umicom/studio/charting.h"
#include "umicom/studio/frontend.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioWebPlatform{UmiWebService *web;UmiBrowser browser;UmiChartModel chart;UmiFrontendPlan frontend;UmiFrontendStudio frontend_studio;};
/* Provide the browser sink operation used by this module and its client applications. */
static void browser_sink(const UmiBrowserEvent *event,void *user_data){(void)event;(void)user_data;}
/*
 * Initialise studio web platform from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_web_platform_create(UmiStudioWebPlatform **out){UmiStudioWebPlatform *p;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=(UmiStudioWebPlatform*)calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;s=umi_web_service_create(&p->web);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_browser_headless_create(browser_sink,p,&p->browser);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_chart_model_init(&p->chart,"Umicom Studio");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_plan_init(&p->frontend,"org.umicom.studio",UMI_FRONTEND_KIND_WEB);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_studio_init(&p->frontend_studio,NULL);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_web_register_defaults(p);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){umi_studio_web_platform_destroy(p);return s;}*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio web platform so the same storage can be reused
 * safely.
 */
void umi_studio_web_platform_destroy(UmiStudioWebPlatform *p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p->browser.instance!=NULL&&p->browser.destroy!=NULL)p->browser.destroy(p->browser.instance);umi_web_service_destroy(p->web);free(p);}
/*
 * Provide the studio web platform snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_web_platform_snapshot(const UmiStudioWebPlatform *p,UmiStudioWebPlatformSnapshot *out){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;(void)memset(out,0,sizeof(*out));out->routes=umi_web_router_count(umi_web_service_router(p->web));out->endpoints=umi_web_endpoint_count(umi_web_service_endpoints(p->web));out->sessions=umi_web_session_count(umi_web_service_sessions(p->web));out->requests=umi_web_service_metrics(p->web)->requests;out->chart_series=p->chart.series_count;out->frontend_pages=p->frontend.page_count;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p->browser.current_url!=NULL){const char *u=p->browser.current_url(p->browser.instance);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(u!=NULL)(void)umi_web_copy_text(out->browser_url,sizeof(out->browser_url),u);}return UMI_STATUS_OK;}
/*
 * Provide the studio web platform web operation used by this module and its client
 * applications.
 */
UmiWebService *umi_studio_web_platform_web(UmiStudioWebPlatform *p){return p!=NULL?p->web:NULL;}
/*
 * Provide the studio web platform browser operation used by this module and its client
 * applications.
 */
UmiBrowser *umi_studio_web_platform_browser(UmiStudioWebPlatform *p){return p!=NULL?&p->browser:NULL;}
/*
 * Provide the studio web platform chart operation used by this module and its client
 * applications.
 */
UmiChartModel *umi_studio_web_platform_chart(UmiStudioWebPlatform *p){return p!=NULL?&p->chart:NULL;}
/*
 * Provide the studio web platform frontend operation used by this module and its client
 * applications.
 */
UmiFrontendPlan *umi_studio_web_platform_frontend(UmiStudioWebPlatform *p){return p!=NULL?&p->frontend:NULL;}
/*
 * Provide the studio web platform frontend studio operation used by this module and its
 * client applications.
 */
UmiFrontendStudio *umi_studio_web_platform_frontend_studio(UmiStudioWebPlatform *p){return p!=NULL?&p->frontend_studio:NULL;}
