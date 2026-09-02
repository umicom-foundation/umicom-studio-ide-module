/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_application_surface_controllers.c
 *
 * PURPOSE:
 *   Verify Studio contributes a controller for every panel in its standard
 *   Framework-owned workspace recipe.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>

#include "umicom/studio/application_surface_controllers.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiApplicationPresentationSurfaceRuntime runtime;
    assert(umi_application_presentation_surface_runtime_init(
               "org.umicom.workspace.studio.standard", &runtime) ==
           UMI_STATUS_OK);
    assert(umi_studio_application_surface_controllers_register(
               &runtime, NULL) == UMI_STATUS_OK);
    assert(runtime.controllers.count == runtime.session.item_count);
    return 0;
}
