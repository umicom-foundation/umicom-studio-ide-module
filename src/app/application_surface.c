/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/application_surface.c
 *
 * PURPOSE:
 *   Own Studio's thin composition over the reusable Framework presentation
 *   runtime and deterministic headless host.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/application_surface.h"

#include <stdlib.h>

#include "umicom/studio/application_surface_controllers.h"

struct UmiStudioApplicationSurface {
    UmiApplicationPresentationProductSurface product;
};

UmiStatus umi_studio_application_surface_create(
    UmiStudioApplicationSurface **out_surface)
{
    return umi_studio_application_surface_create_for_audience(
        UMI_APPLICATION_COMPONENT_RECIPE_AUDIENCE_STANDARD, out_surface);
}

UmiStatus umi_studio_application_surface_create_for_audience(
    UmiApplicationComponentRecipeAudience audience,
    UmiStudioApplicationSurface **out_surface)
{
    UmiStudioApplicationSurface *surface;
    UmiStatus status;
    if (out_surface == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_surface = NULL;
    surface = calloc(1U, sizeof(*surface));
    if (surface == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_application_presentation_product_surface_init_for_audience(
        UMI_STUDIO_APPLICATION_ID,
        audience,
        umi_studio_application_surface_controllers_register,
        surface,
        &surface->product);
    if (status != UMI_STATUS_OK) {
        umi_studio_application_surface_destroy(surface);
        return status;
    }
    *out_surface = surface;
    return UMI_STATUS_OK;
}

void umi_studio_application_surface_destroy(
    UmiStudioApplicationSurface *surface)
{
    if (surface == NULL) return;
    umi_application_presentation_product_surface_dispose(&surface->product);
    free(surface);
}

UmiStatus umi_studio_application_surface_refresh(
    UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_refresh(
              &surface->product)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_activate(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_activate(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_deactivate(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_deactivate(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_focus(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_focus(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_command(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *command_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_command(
              &surface->product, component_id, command_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_context_changed(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *context_value)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_context_changed(
              &surface->product, component_id, context_value)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_advance(
    UmiStudioApplicationSurface *surface,
    uint32_t elapsed_seconds)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_advance(
              &surface->product, elapsed_seconds)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_set_background(
    UmiStudioApplicationSurface *surface,
    int background)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_set_background(
              &surface->product, background)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiApplicationPresentationSurfaceSnapshot *out_snapshot)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_snapshot(
              &surface->product, out_snapshot)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime(UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_runtime(
              &surface->product)
        : NULL;
}

const UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime_const(
    const UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_runtime_const(
              &surface->product)
        : NULL;
}
