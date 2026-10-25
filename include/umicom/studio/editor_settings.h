/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/editor_settings.h
 *
 * PURPOSE:
 *   Apply Studio's typed editor preferences to the shared document service.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_EDITOR_SETTINGS_H
#define UMICOM_STUDIO_EDITOR_SETTINGS_H
#include "umicom/studio/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Copy validated tab width and spaces preference from the existing settings
 * service. Does not persist settings, alter source text or run a native view.
 * Called at UI creation and refresh; clients can call after changing settings. */
UmiStatus UmiStudioUiApplyEditorSettings(UmiStudioUi *ui);
#ifdef __cplusplus
}
#endif
#endif
