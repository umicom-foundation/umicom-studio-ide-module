/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/application_surface_commands.c
 *
 * PURPOSE:
 *   Implement memorable Studio navigation helpers over generic Framework
 *   component identities and presentation runtime operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/application_surface_commands.h"

/*
 * Provide the studio application surface open editor operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_surface_open_editor(
    UmiStudioApplicationSurface *surface)
{
    return umi_studio_application_surface_focus(
        surface, "umicom.development.editor");
}

/*
 * Provide the studio application surface open explorer operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_open_explorer(
    UmiStudioApplicationSurface *surface)
{
    return umi_studio_application_surface_focus(
        surface, "umicom.development.explorer");
}

/*
 * Provide the studio application surface open designer operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_open_designer(
    UmiStudioApplicationSurface *surface)
{
    return umi_studio_application_surface_focus(
        surface, "umicom.development.designer");
}

/*
 * Provide the studio application surface open ai chat operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_open_ai_chat(
    UmiStudioApplicationSurface *surface)
{
    return umi_studio_application_surface_focus(surface, "umicom.ai.chat");
}

/*
 * Provide the studio application surface refresh diagnostics operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_application_surface_refresh_diagnostics(
    UmiStudioApplicationSurface *surface)
{
    UmiApplicationPresentationSurfaceRuntime *runtime =
        umi_studio_application_surface_runtime(surface);
    return runtime != NULL
        ? umi_application_presentation_surface_runtime_refresh(
              runtime, "umicom.shared.diagnostics")
        : UMI_STATUS_INVALID_ARGUMENT;
}
