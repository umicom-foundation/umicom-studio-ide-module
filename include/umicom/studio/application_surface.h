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

#define UMI_STUDIO_STANDARD_RECIPE_ID "org.umicom.workspace.studio.standard"

typedef struct UmiStudioApplicationSurface UmiStudioApplicationSurface;

UmiStatus umi_studio_application_surface_create(
    UmiStudioApplicationSurface **out_surface);
void umi_studio_application_surface_destroy(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_refresh(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_focus(
    UmiStudioApplicationSurface *surface,
    const char *component_id);
UmiStatus umi_studio_application_surface_command(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *command_id);
UmiStatus umi_studio_application_surface_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiApplicationPresentationSurfaceSnapshot *out_snapshot);
UmiApplicationPresentationSurfaceRuntime *
umi_studio_application_surface_runtime(UmiStudioApplicationSurface *surface);

#ifdef __cplusplus
}
#endif

#endif
