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
    UmiApplicationPresentationSurfaceRuntime runtime;
    UmiApplicationPresentationHeadlessSurfaceHost headless;
};

UmiStatus umi_studio_application_surface_create(
    UmiStudioApplicationSurface **out_surface)
{
    UmiStudioApplicationSurface *surface;
    UmiStatus status;
    if (out_surface == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_surface = NULL;
    surface = calloc(1U, sizeof(*surface));
    if (surface == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_application_presentation_surface_runtime_init(
        UMI_STUDIO_STANDARD_RECIPE_ID, &surface->runtime);
    if (status == UMI_STATUS_OK) {
        umi_application_presentation_headless_surface_host_init(
            &surface->headless);
        status = umi_application_presentation_surface_runtime_bind_host(
            &surface->runtime,
            umi_application_presentation_headless_surface_host_interface(
                &surface->headless));
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_application_surface_controllers_register(
            &surface->runtime, surface);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_application_presentation_surface_runtime_start(
            &surface->runtime);
    }
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
    (void)umi_application_presentation_surface_runtime_stop(&surface->runtime);
    free(surface);
}

UmiStatus umi_studio_application_surface_refresh(
    UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_surface_runtime_refresh_all(
              &surface->runtime)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_focus(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_surface_runtime_focus(
              &surface->runtime, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_command(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *command_id)
{
    return surface != NULL
        ? umi_application_presentation_surface_runtime_command(
              &surface->runtime, component_id, command_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiApplicationPresentationSurfaceSnapshot *out_snapshot)
{
    return surface != NULL
        ? umi_application_presentation_surface_runtime_snapshot(
              &surface->runtime, out_snapshot)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime(UmiStudioApplicationSurface *surface)
{
    return surface != NULL ? &surface->runtime : NULL;
}

const UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime_const(
    const UmiStudioApplicationSurface *surface)
{
    return surface != NULL ? &surface->runtime : NULL;
}
