/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/workspace_themes.c
 *
 * PURPOSE:
 *   Implement the workspace themes behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Workspace themes v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/workspace_themes.h"
/*
 * Perform studio workspace theme through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_workspace_theme_apply(UmiStudioProfessionalWorkspace *workspace,UmiStudioWorkspaceTheme theme,UmiUiDensity density,double font_scale)
{
    UmiUiWorkspaceCustomisation *model = umi_studio_professional_workspace_model(workspace);
    UmiUiThemeProfile profile;
    UmiUiThemeMode mode;
    const char *theme_id;
    const char *name;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Select the behaviour associated with the requested command or state value. */
    switch (theme) {
        case UMI_STUDIO_WORKSPACE_THEME_SYSTEM: mode = UMI_UI_THEME_MODE_SYSTEM; theme_id = "umicom-system"; name = "Umicom System"; break;
        case UMI_STUDIO_WORKSPACE_THEME_LIGHT: mode = UMI_UI_THEME_MODE_LIGHT; theme_id = "umicom-light"; name = "Umicom Light"; break;
        case UMI_STUDIO_WORKSPACE_THEME_DARK: mode = UMI_UI_THEME_MODE_DARK; theme_id = "umicom-dark"; name = "Umicom Dark"; break;
        case UMI_STUDIO_WORKSPACE_THEME_HIGH_CONTRAST: mode = UMI_UI_THEME_MODE_HIGH_CONTRAST; theme_id = "umicom-high-contrast"; name = "Umicom High Contrast"; break;
        default: return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_ui_theme_profile_init(&profile,theme_id,name,mode,density);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    profile.font_scale = font_scale;
    return umi_ui_workspace_customisation_set_theme(model,&profile);
}
