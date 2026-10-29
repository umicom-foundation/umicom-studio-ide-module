/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_view_models.c
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
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/view_models.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
UmiStudioBootstrap*b=NULL;UmiStudioViewModelSnapshot s;UmiStudioViewModels*m;assert(umi_studio_bootstrap_create(&b)==UMI_STATUS_OK);m=umi_studio_ui_view_models(umi_studio_bootstrap_ui(b));assert(umi_studio_view_models_refresh(m)==UMI_STATUS_OK);assert(umi_studio_view_models_snapshot(m,&s)==UMI_STATUS_OK);assert(s.view_count==5U);assert(umi_studio_view_models_find(m,"studio.view.editor")!=NULL);umi_studio_bootstrap_destroy(b);if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;}
