/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_ui_headless.c
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
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
UmiStudioBootstrap*b=NULL;UmiUiHeadlessSnapshot s;assert(umi_studio_bootstrap_create(&b)==UMI_STATUS_OK);assert(umi_studio_ui_render_headless(umi_studio_bootstrap_ui(b),&s)==UMI_STATUS_OK);assert(strstr(s.text,"Umicom Studio IDE")!=NULL);assert(s.pane_count>=8U);umi_studio_bootstrap_destroy(b);if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;}
