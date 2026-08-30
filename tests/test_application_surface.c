/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_application_surface.c
 *
 * PURPOSE:
 *   Verify Studio starts its standard Framework presentation recipe and makes
 *   all ten reusable application panels observable through one snapshot.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>

#include "umicom/studio/application_surface.h"

int main(void)
{
    UmiStudioApplicationSurface *surface = NULL;
    UmiApplicationPresentationSurfaceSnapshot snapshot;
    assert(umi_studio_application_surface_create(&surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_snapshot(surface, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.panel_count == 10U);
    assert(snapshot.visible_count == 10U);
    assert(snapshot.attention_count == 0U);
    umi_studio_application_surface_destroy(surface);
    return 0;
}
