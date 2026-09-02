/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_web_service_accessor.c
 *
 * PURPOSE:
 *   Verify Studio integration with Framework web, browser, charting and frontend services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The test uses public Studio and Framework contracts so it documents the expected behaviour for new contributors.
 */

#include "umicom/studio/web_platform.h"
#include <assert.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioWebPlatform *p=NULL;assert(umi_studio_web_platform_create(&p)==UMI_STATUS_OK);assert(umi_studio_web_platform_web(p)!=NULL);assert(umi_studio_web_platform_browser(p)!=NULL);assert(umi_studio_web_platform_chart(p)!=NULL);assert(umi_studio_web_platform_frontend(p)!=NULL);umi_studio_web_platform_destroy(p);return 0;}
