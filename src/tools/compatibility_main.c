/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/compatibility_main.c
 *
 * PURPOSE:
 *   Run the Studio reference-product compatibility report.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command exposes one compatibility check in a form that is easy to run from PowerShell, CI, or the future Studio workbench UI.
 */
#include <stdio.h>
#include "umicom/studio/compatibility_platform.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioCompatibilityPlatform *p=NULL;char b[256];/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_compatibility_platform_create(&p)!=UMI_STATUS_OK)return 1;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_compatibility_summary(p,b,sizeof(b))==UMI_STATUS_OK)fputs(b,stdout);umi_studio_compatibility_platform_destroy(p);return 0;}
