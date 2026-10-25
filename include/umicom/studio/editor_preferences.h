/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/editor_preferences.h
 *
 * PURPOSE:
 *   Review and apply a coherent pair of editor preferences through Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_EDITOR_PREFERENCES_H
#define UMICOM_STUDIO_EDITOR_PREFERENCES_H
#include "umicom/editor/indentation.h"
#include "umicom/studio/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/** Copy a coherent pair of settings. On failure output is untouched. */
UmiStatus UmiStudioUiReadEditorPreferences(UmiStudioUi *ui, UmiEditorIndentation *out);
/** Owner-thread operation. Apply a validated candidate only when the stored
 * pair still matches the reviewed pair. No source text, history, files or
 * native widgets are changed. Preferences last for the current session.
 * The host refreshes native tab presentation after a successful return.
 * Neither this function nor the settings repository invokes application callbacks. */
UmiStatus UmiStudioUiUpdateEditorPreferences(UmiStudioUi *ui, const UmiEditorIndentation *reviewed,
                                             const UmiEditorIndentation *candidate);
#ifdef __cplusplus
}
#endif
#endif
