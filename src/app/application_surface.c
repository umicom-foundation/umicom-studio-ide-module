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

/*
 * Initialise studio application surface from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_application_surface_create(
    UmiStudioApplicationSurface **out_surface)
{
    return umi_studio_application_surface_create_for_audience(
        UMI_APPLICATION_COMPONENT_RECIPE_AUDIENCE_STANDARD, out_surface);
}

/*
 * Provide the studio application surface create for audience operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_application_surface_create_for_audience(
    UmiApplicationComponentRecipeAudience audience,
    UmiStudioApplicationSurface **out_surface)
{
    UmiStudioApplicationSurface *surface;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_surface == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_surface = NULL;
    surface = calloc(1U, sizeof(*surface));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (surface == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_application_presentation_product_surface_init_for_audience(
        UMI_STUDIO_APPLICATION_ID,
        audience,
        umi_studio_application_surface_controllers_register,
        surface,
        &surface->product);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_application_surface_destroy(surface);
        return status;
    }
    *out_surface = surface;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio application surface so the same storage can be
 * reused safely.
 */
void umi_studio_application_surface_destroy(
    UmiStudioApplicationSurface *surface)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (surface == NULL) return;
    umi_application_presentation_product_surface_dispose(&surface->product);
    free(surface);
}

/*
 * Provide the studio application surface refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_refresh(
    UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_refresh(
              &surface->product)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_activate(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_activate(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface deactivate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_deactivate(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_deactivate(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface focus operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_focus(
    UmiStudioApplicationSurface *surface,
    const char *component_id)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_focus(
              &surface->product, component_id)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface command operation used by this module and its
 * client applications.
 */
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

/*
 * Provide the studio application surface context changed operation used by this module and
 * its client applications.
 */
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

/*
 * Provide the studio application surface advance operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_advance(
    UmiStudioApplicationSurface *surface,
    uint32_t elapsed_seconds)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_advance(
              &surface->product, elapsed_seconds)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface set background operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_set_background(
    UmiStudioApplicationSurface *surface,
    int background)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_set_background(
              &surface->product, background)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiApplicationPresentationSurfaceSnapshot *out_snapshot)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_snapshot(
              &surface->product, out_snapshot)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio application surface runtime operation used by this module and its
 * client applications.
 */
UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime(UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_runtime(
              &surface->product)
        : NULL;
}

/*
 * Provide the studio application surface runtime const operation used by this module and
 * its client applications.
 */
const UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime_const(
    const UmiStudioApplicationSurface *surface)
{
    return surface != NULL
        ? umi_application_presentation_product_surface_runtime_const(
              &surface->product)
        : NULL;
}
