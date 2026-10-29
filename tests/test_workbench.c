/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_workbench.c
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
#include "umicom/studio/perspectives.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
UmiStudioBootstrap*b=NULL;UmiUiWorkbenchSnapshot s;assert(umi_studio_bootstrap_create(&b)==UMI_STATUS_OK);assert(umi_ui_workbench_activate_perspective(umi_studio_ui_workbench(umi_studio_bootstrap_ui(b)),UMI_STUDIO_PERSPECTIVE_BUILD)==UMI_STATUS_OK);assert(umi_ui_workbench_snapshot(umi_studio_ui_workbench(umi_studio_bootstrap_ui(b)),&s)==UMI_STATUS_OK);assert(strcmp(s.active_perspective,UMI_STUDIO_PERSPECTIVE_BUILD)==0);umi_studio_bootstrap_destroy(b);if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;}
