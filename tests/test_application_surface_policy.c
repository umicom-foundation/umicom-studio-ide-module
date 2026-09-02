/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_application_surface_policy.c
 *
 * PURPOSE:
 *   Verify Studio exposes its Framework-owned runtime policy and delegates
 *   timing, background, shared-context and checkpoint decisions correctly.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/application_surface_policy.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioApplicationSurface *surface = NULL;
    UmiStudioApplicationSurfacePolicySnapshot policy;
    assert(umi_studio_application_surface_create(&surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_policy_snapshot(surface, &policy) ==
           UMI_STATUS_OK);
    assert(strcmp(policy.recipe_id, UMI_STUDIO_STANDARD_RECIPE_ID) == 0);
    assert(strcmp(policy.checkpoint_policy, "periodic") == 0);
    assert(strcmp(policy.background_policy, "reduced") == 0);
    assert(policy.checkpoint_interval_seconds == 30U);
    assert(policy.context_enabled_panels > 0U);
    assert(policy.share_context);
    assert(umi_studio_application_surface_policy_context_changed(
               surface, "umicom.development.editor", "file:main.c") ==
           UMI_STATUS_OK);
    assert(umi_studio_application_surface_policy_set_background(surface, 1) ==
           UMI_STATUS_OK);
    assert(umi_studio_application_surface_policy_advance(surface, 4U) ==
           UMI_STATUS_OK);
    assert(!umi_studio_application_surface_policy_checkpoint_due(
        surface, 29U, 1));
    assert(umi_studio_application_surface_policy_checkpoint_due(
        surface, 30U, 0));
    umi_studio_application_surface_destroy(surface);
    return 0;
}
