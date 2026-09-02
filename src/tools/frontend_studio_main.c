/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/frontend_studio_main.c
 *
 * PURPOSE:
 *   Provide the frontend studio main development utility for
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
#include <stdio.h>
#include "umicom/studio/frontend_studio_commands.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioWebPlatform *p=NULL;UmiStudioFrontendCentre c;UmiStudioFrontendCentreSnapshot s;char message[512];UmiStatus st=umi_studio_web_platform_create(&p);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(st==UMI_STATUS_OK)st=umi_studio_frontend_centre_init(&c,p);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(st==UMI_STATUS_OK)st=umi_studio_frontend_command_execute(&c,"frontend.preview.start",NULL,message,sizeof(message));/* Preserve the original failure result so the caller can respond to the correct cause. */ if(st==UMI_STATUS_OK)st=umi_studio_frontend_centre_snapshot(&c,&s);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(st==UMI_STATUS_OK)(void)printf("%s\nPreview: %s (%zu devices)\n",message,s.frontend.preview_url,s.frontend.device_count);umi_studio_web_platform_destroy(p);return st==UMI_STATUS_OK?0:1;}
