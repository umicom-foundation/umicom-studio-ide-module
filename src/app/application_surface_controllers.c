/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/application_surface_controllers.c
 *
 * PURPOSE:
 *   Supply honest initial guidance for Studio panels and translate product
 *   refresh and command events into shared Framework surface state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/application_surface_controllers.h"

#include <stdio.h>
#include <string.h>

static const char *empty_guidance(const char *component_id)
{
    if (strcmp(component_id, "umicom.development.editor") == 0)
        return "Open or create a file to begin editing.";
    if (strcmp(component_id, "umicom.development.explorer") == 0)
        return "Open a folder or project to browse its files.";
    if (strcmp(component_id, "umicom.development.designer") == 0)
        return "Open a UI document to begin visual design.";
    if (strcmp(component_id, "umicom.development.build") == 0)
        return "Run a build to see tasks and output here.";
    if (strcmp(component_id, "umicom.development.debug") == 0)
        return "Start a debug session to inspect program state.";
    if (strcmp(component_id, "umicom.development.testing") == 0)
        return "Open a project to discover and run its tests.";
    if (strcmp(component_id, "umicom.development.source-control") == 0)
        return "Open a Git repository to inspect its changes.";
    if (strcmp(component_id, "umicom.development.terminal") == 0)
        return "Create a terminal session when command-line work is needed.";
    if (strcmp(component_id, "umicom.ai.chat") == 0)
        return "Start a conversation about the current workspace.";
    return "No problems have been reported.";
}

static UmiStatus studio_controller(
    void *context,
    const UmiApplicationPresentationPanelPlacement *placement,
    UmiApplicationPresentationSurfaceEvent event,
    const char *payload,
    UmiApplicationPresentationSurfaceUpdate *out_update)
{
    const char *component_id;
    (void)context;
    if (placement == NULL || placement->panel == NULL || out_update == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    component_id = placement->panel->component_id;
    (void)memset(out_update, 0, sizeof(*out_update));
    if (event == UMI_APPLICATION_PRESENTATION_EVENT_UNMOUNT ||
        event == UMI_APPLICATION_PRESENTATION_EVENT_DEACTIVATE) {
        out_update->state = UMI_APPLICATION_PRESENTATION_STATE_DORMANT;
        return UMI_STATUS_OK;
    }
    if (event == UMI_APPLICATION_PRESENTATION_EVENT_COMMAND) {
        out_update->state = UMI_APPLICATION_PRESENTATION_STATE_READY;
        (void)snprintf(out_update->message, sizeof(out_update->message),
                       "Studio accepted command: %s",
                       payload != NULL ? payload : "");
        return UMI_STATUS_OK;
    }
    out_update->state = UMI_APPLICATION_PRESENTATION_STATE_EMPTY;
    (void)snprintf(out_update->message, sizeof(out_update->message), "%s",
                   empty_guidance(component_id));
    if (strcmp(component_id, "umicom.shared.diagnostics") == 0) {
        out_update->state = UMI_APPLICATION_PRESENTATION_STATE_READY;
        (void)snprintf(out_update->badge, sizeof(out_update->badge), "0");
    }
    return UMI_STATUS_OK;
}

UmiStatus umi_studio_application_surface_controllers_register(
    UmiApplicationPresentationSurfaceRuntime *runtime,
    void *context)
{
    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_application_presentation_surface_runtime_register_controller_for_all(
        runtime, studio_controller, context);
}
