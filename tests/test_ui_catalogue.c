/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_ui_catalogue.c
 *
 * PURPOSE:
 *   Validate the Studio Framework UI composition and workbench integration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* These assertions perform setup as well as checks, including in release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include <assert.h>
#include <string.h>
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/ui_catalogue.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
UmiStudioBootstrap*b=NULL;UmiStudioUiCatalogue c;char text[2048];assert(umi_studio_bootstrap_create(&b)==UMI_STATUS_OK);assert(umi_studio_ui_catalogue_snapshot(umi_studio_bootstrap_ui(b),&c)==UMI_STATUS_OK);assert(c.perspectives==8U);assert(umi_studio_ui_catalogue_format(&c,text,sizeof(text))==UMI_STATUS_OK);assert(strstr(text,"Studio UI Catalogue")!=NULL);umi_studio_bootstrap_destroy(b);if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;}
