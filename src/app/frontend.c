/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/frontend.c
 *
 * PURPOSE:
 *   Build Studio's semantic frontend plan and generate browser artifacts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The frontend plan describes pages and routes. It does not make React, gtk-js or a browser the owner of Studio state.
 */

#include "umicom/studio/frontend.h"
/* Release or reset state held by studio frontend so the same storage can be reused safely. */
UmiStatus umi_studio_frontend_reset(UmiStudioWebPlatform *p,UmiFrontendKind kind){UmiFrontendPlan *plan;UmiFrontendPage page;UmiFrontendRoute route;UmiFrontendAsset asset;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;plan=umi_studio_web_platform_frontend(p);s=umi_frontend_plan_init(plan,"org.umicom.studio",kind);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_page_init(&page,"workbench","Umicom Studio IDE","studio.workbench");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_plan_add_page(plan,&page);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_route_init(&route,"/","workbench");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_plan_add_route(plan,&route);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_asset_init(&asset,"theme","assets/umicom.css","text/css");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_plan_add_asset(plan,&asset);return s;}
/*
 * Provide the studio frontend generate html operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_frontend_generate_html(UmiStudioWebPlatform *p,char *out,size_t cap){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_frontend_generate_html(umi_studio_web_platform_frontend(p),out,cap);}
/*
 * Provide the studio frontend generate manifest operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_generate_manifest(UmiStudioWebPlatform *p,char *out,size_t cap){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_frontend_generate_manifest(umi_studio_web_platform_frontend(p),out,cap);}
