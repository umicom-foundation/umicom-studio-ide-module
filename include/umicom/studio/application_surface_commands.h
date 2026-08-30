/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/application_surface_commands.h
 *
 * PURPOSE:
 *   Offer simple product-facing commands for common Studio panel navigation
 *   while the Framework remains responsible for session state and dispatch.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_APPLICATION_SURFACE_COMMANDS_H
#define UMICOM_STUDIO_APPLICATION_SURFACE_COMMANDS_H

#include "umicom/studio/application_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

UmiStatus umi_studio_application_surface_open_editor(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_open_explorer(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_open_designer(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_open_ai_chat(
    UmiStudioApplicationSurface *surface);
UmiStatus umi_studio_application_surface_refresh_diagnostics(
    UmiStudioApplicationSurface *surface);

#ifdef __cplusplus
}
#endif

#endif
