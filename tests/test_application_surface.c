/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_application_surface.c
 *
 * PURPOSE:
 *   Verify Studio starts its standard Framework presentation recipe and makes
 *   every reusable application panel observable through one snapshot.
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

#include "umicom/studio/application_surface.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioApplicationSurface *surface = NULL;
    UmiApplicationPresentationSurfaceSnapshot snapshot;
    size_t expected_visible = 0U;
    size_t index;
    const UmiApplicationComponentRecipe *standard =
        umi_application_component_recipe_catalogue_find(
            "org.umicom.workspace.studio.standard");
    assert(standard != NULL);
    /* Derive visibility from the recipe so optional panels can be added safely. */
    for (index = 0U; index < standard->slot_count; ++index) {
        if (standard->slots[index].visible) expected_visible += 1U;
    }
    assert(umi_studio_application_surface_create(&surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_snapshot(surface, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.panel_count == standard->slot_count);
    assert(snapshot.visible_count == expected_visible);
    assert(snapshot.attention_count == 0U);
    umi_studio_application_surface_destroy(surface);

    surface = NULL;
    assert(umi_studio_application_surface_create_for_audience(
               UMI_APPLICATION_COMPONENT_RECIPE_AUDIENCE_FOCUS,
               &surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_snapshot(surface, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.recipe_id,
                  "org.umicom.workspace.studio.focus") == 0);
    assert(snapshot.panel_count > 0U);
    assert(snapshot.panel_count < standard->slot_count);
    umi_studio_application_surface_destroy(surface);

    surface = NULL;
    assert(umi_studio_application_surface_create_for_audience(
               UMI_APPLICATION_COMPONENT_RECIPE_AUDIENCE_LEARNING,
               &surface) == UMI_STATUS_OK);
    assert(umi_studio_application_surface_snapshot(surface, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.recipe_id,
                  "org.umicom.workspace.studio.learning") == 0);
    umi_studio_application_surface_destroy(surface);
    return 0;
}
