/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_application_surface_commands.c
 *
 * PURPOSE:
 *   Verify Studio's friendly navigation helpers focus canonical Framework
 *   components instead of depending on GTK widgets or local panel indexes.
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

#include "umicom/studio/application_surface_commands.h"

int main(void)
{
    UmiStudioApplicationSurface *surface = NULL;
    UmiApplicationPresentationSurfaceSnapshot snapshot;
    assert(umi_studio_application_surface_create(&surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_open_ai_chat(surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_refresh_diagnostics(surface) ==
           UMI_STATUS_OK);
    assert(umi_studio_application_surface_snapshot(surface, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.focused_component_id, "umicom.ai.chat") == 0);
    umi_studio_application_surface_destroy(surface);
    return 0;
}
