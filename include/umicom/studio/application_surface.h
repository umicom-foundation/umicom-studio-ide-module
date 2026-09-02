/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/application_surface.h
 *
 * PURPOSE:
 *   Compose Studio's standard workspace from the Framework-owned application
 *   presentation runtime instead of duplicating panel lifecycle logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_APPLICATION_SURFACE_H
#define UMICOM_STUDIO_APPLICATION_SURFACE_H

#include "umicom/application/presentation/presentation.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_APPLICATION_ID "org.umicom.studio"
#define UMI_STUDIO_STANDARD_RECIPE_ID "org.umicom.workspace.studio.standard"

/**
 * Represent the studio application surface data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioApplicationSurface UmiStudioApplicationSurface;

/**
 * Initialise studio application surface from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_application_surface_create(
    UmiStudioApplicationSurface **out_surface);
/**
 * Provide the studio application surface create for audience operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_application_surface_create_for_audience(
    UmiApplicationComponentRecipeAudience audience,
    UmiStudioApplicationSurface **out_surface);
/**
 * Release or reset state held by studio application surface so the same storage can be
 * reused safely.
 */
void umi_studio_application_surface_destroy(
    UmiStudioApplicationSurface *surface);
/**
 * Provide the studio application surface refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_refresh(
    UmiStudioApplicationSurface *surface);
/**
 * Provide the studio application surface activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_activate(
    UmiStudioApplicationSurface *surface,
    const char *component_id);
/**
 * Provide the studio application surface deactivate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_deactivate(
    UmiStudioApplicationSurface *surface,
    const char *component_id);
/**
 * Provide the studio application surface focus operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_focus(
    UmiStudioApplicationSurface *surface,
    const char *component_id);
/**
 * Provide the studio application surface command operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_command(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *command_id);
/**
 * Provide the studio application surface context changed operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_context_changed(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *context_value);
/**
 * Provide the studio application surface advance operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_advance(
    UmiStudioApplicationSurface *surface,
    uint32_t elapsed_seconds);
/**
 * Provide the studio application surface set background operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_set_background(
    UmiStudioApplicationSurface *surface,
    int background);
/**
 * Provide the studio application surface snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiApplicationPresentationSurfaceSnapshot *out_snapshot);
/**
 * Provide the studio application surface runtime operation used by this module and its
 * client applications.
 */
UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime(UmiStudioApplicationSurface *surface);
/**
 * Provide the studio application surface runtime const operation used by this module and
 * its client applications.
 */
const UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime_const(
    const UmiStudioApplicationSurface *surface);

#ifdef __cplusplus
}
#endif

#endif
