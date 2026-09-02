/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/frontend_main.c
 *
 * PURPOSE:
 *   Provide a small native command for the Studio Web Server and multi-frontend platform.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command is intentionally small: it calls the same Studio service used by GUI code, which makes the feature easy to test from a terminal.
 */

#include "umicom/studio/web_platform.h"
#include "umicom/studio/frontend.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioWebPlatform *p=NULL;char html[2048],manifest[1024];UmiStatus s=umi_studio_web_platform_create(&p);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_frontend_reset(p,UMI_FRONTEND_KIND_WEB);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_frontend_generate_html(p,html,sizeof(html));/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_frontend_generate_manifest(p,manifest,sizeof(manifest));/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK){(void)puts(html);(void)puts(manifest);}umi_studio_web_platform_destroy(p);return s==UMI_STATUS_OK?0:1;}
