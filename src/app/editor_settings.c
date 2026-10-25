/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/editor_settings.c
 *
 * PURPOSE:
 *   Translate existing typed settings into Framework-owned indentation policy without a second
 * editor model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/editor_settings.h"
#include "umicom/document/indentation.h"
#include "umicom/studio/settings.h"
/* Read related preferences under one lock so a concurrent settings change
 * cannot produce a mixed width/spaces pair. Keep the previous sequential
 * reads for review; the shared settings snapshot replaces that assumption. */
#if 0
UmiStatus UmiStudioUiApplyEditorSettings(UmiStudioUi *ui) {
  if (ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiSettings *settings = umi_studio_services_settings(umi_studio_ui_services(ui));
  int64_t width = 0;
  int spaces = 0;
  UmiStatus status =
      umi_settings_get_integer(settings, UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, &width);
  if (status == UMI_STATUS_OK)
    status = umi_settings_get_boolean(settings, UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES, &spaces);
  if (status != UMI_STATUS_OK)
    return status;
  if (width < 1 || width > UMI_EDITOR_INDENTATION_MAXIMUM_WIDTH)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiEditorIndentation policy = {(uint32_t)width, spaces};
  return UmiDocumentCoordinatorSetIndentation(umi_studio_ui_documents(ui), &policy);
}

#endif
#include "umicom/studio/editor_preferences.h"
UmiStatus UmiStudioUiApplyEditorSettings(UmiStudioUi *ui)
{
    UmiEditorIndentation policy;
    UmiStatus status = UmiStudioUiReadEditorPreferences(ui, &policy);
    return status == UMI_STATUS_OK
        ? UmiDocumentCoordinatorSetIndentation(umi_studio_ui_documents(ui), &policy) : status;
}
