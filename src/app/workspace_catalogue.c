/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/workspace_catalogue.c
 *
 * PURPOSE:
 *   Populate Studio from the Framework-owned reusable component and window
 *   catalogue. Studio does not maintain private component definitions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_catalogue.h"

#include "umicom/desktop/ui_bridge.h"
#include "umicom/ai_ui/assistant_windows.h"

UmiStatus umi_studio_workspace_catalogue_seed(
    UmiStudioProfessionalWorkspace *workspace)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    UmiStatus status;
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_desktop_seed_window_catalogue(&model->windows);
    if (status == UMI_STATUS_OK) {
        /* Studio opts into the same assistant tools available to other apps. */
        status = umi_ai_ui_assistant_windows_register(&model->windows);
    }
    return status;
}
