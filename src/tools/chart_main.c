/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/chart_main.c
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
#include "umicom/studio/charting.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioWebPlatform *p=NULL;UmiStudioWebPlatformSnapshot snap;UmiStatus s=umi_studio_web_platform_create(&p);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_charting_add_close(p,1,100.0);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_charting_add_close(p,2,101.0);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)s=umi_studio_web_platform_snapshot(p,&snap);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s==UMI_STATUS_OK)(void)printf("Chart series: %zu\n",snap.chart_series);umi_studio_web_platform_destroy(p);return s==UMI_STATUS_OK?0:1;}
