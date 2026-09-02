/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_themes.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace themes contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_THEMES_H
#define UMICOM_STUDIO_WORKSPACE_THEMES_H
#include "umicom/studio/professional_workspace.h"
/**
 * List the named studio workspace theme values accepted by this public contract.
 */
typedef enum UmiStudioWorkspaceTheme {
    UMI_STUDIO_WORKSPACE_THEME_SYSTEM = 1,
    UMI_STUDIO_WORKSPACE_THEME_LIGHT,
    UMI_STUDIO_WORKSPACE_THEME_DARK,
    UMI_STUDIO_WORKSPACE_THEME_HIGH_CONTRAST
} UmiStudioWorkspaceTheme;
/**
 * Perform studio workspace theme through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_workspace_theme_apply(UmiStudioProfessionalWorkspace *workspace,UmiStudioWorkspaceTheme theme,UmiUiDensity density,double font_scale);
#endif
