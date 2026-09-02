/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/application_surface_controllers.h
 *
 * PURPOSE:
 *   Register Studio guidance and state controllers for every component in the
 *   standard Framework-owned IDE workspace recipe.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_APPLICATION_SURFACE_CONTROLLERS_H
#define UMICOM_STUDIO_APPLICATION_SURFACE_CONTROLLERS_H

#include "umicom/application/presentation/presentation.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add studio application surface controllers only after its inputs and available capacity
 * have been checked.
 */
UmiStatus umi_studio_application_surface_controllers_register(
    UmiApplicationPresentationSurfaceRuntime *runtime,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
