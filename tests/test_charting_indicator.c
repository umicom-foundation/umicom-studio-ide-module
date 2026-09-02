/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_charting_indicator.c
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
#include "umicom/studio/charting.h"
#include <assert.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioWebPlatform *p=NULL;assert(umi_studio_web_platform_create(&p)==UMI_STATUS_OK);/* Visit each bounded item once so every record receives the same rule. */ for(int i=0;i<5;++i)assert(umi_studio_charting_add_close(p,i,(double)i)==UMI_STATUS_OK);assert(umi_studio_charting_add_sma(p,3U)==UMI_STATUS_OK);umi_studio_web_platform_destroy(p);return 0;}
