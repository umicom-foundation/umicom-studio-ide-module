/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/browser.c
 *
 * PURPOSE:
 *   Implement Studio embedded-browser navigation through the Framework browser function table.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio asks the browser contract to navigate. A future WebKit or Edge adapter can replace the headless provider without changing this use case.
 */

#include "umicom/studio/browser.h"
/*
 * Provide the studio browser open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_browser_open(UmiStudioWebPlatform *p,const char *url){UmiBrowser *b;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||url==NULL)return UMI_STATUS_INVALID_ARGUMENT;b=umi_studio_web_platform_browser(p);return b!=NULL&&b->navigate!=NULL?b->navigate(b->instance,url):UMI_STATUS_INVALID_STATE;}
/*
 * Provide the studio browser current operation used by this module and its client
 * applications.
 */
const char *umi_studio_browser_current(const UmiStudioWebPlatform *p){UmiBrowser *b=umi_studio_web_platform_browser((UmiStudioWebPlatform*)p);return b!=NULL&&b->current_url!=NULL?b->current_url(b->instance):NULL;}
