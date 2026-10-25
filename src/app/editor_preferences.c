/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/editor_preferences.c
 *
 * PURPOSE:
 *   Keep Studio preference composition thin while Framework owns validation, atomic settings and
 * document policy.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/editor_preferences.h"
#include "umicom/document/indentation.h"
#include "umicom/platform/settings_changes.h"
#include "umicom/studio/settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
UmiStatus UmiStudioUiReadEditorPreferences(UmiStudioUi *ui, UmiEditorIndentation *out) {
  if (ui == NULL || out == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  const char *keys[] = {UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH,
                        UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES};
  UmiSettingValue values[2];
  UmiStatus status = UmiSettingsReadValues(umi_studio_services_settings(umi_studio_ui_services(ui)),
                                           keys, 2U, values);
  if (status != UMI_STATUS_OK)
    return status;
  /* Schema values are canonical. Bound parsing independently so this
   * consumer also rejects incompatible schema changes without truncation. */
  char *end = NULL;
  unsigned long width = strtoul(values[0].text, &end, 10);
  if (end == values[0].text || *end != '\0' || width < 1U ||
      width > UMI_EDITOR_INDENTATION_MAXIMUM_WIDTH)
    return UMI_STATUS_INVALID_STATE;
  if (strcmp(values[1].text, "true") != 0 && strcmp(values[1].text, "false") != 0)
    return UMI_STATUS_INVALID_STATE;
  UmiEditorIndentation result = {(uint32_t)width, strcmp(values[1].text, "true") == 0};
  *out = result;
  return UMI_STATUS_OK;
}
UmiStatus UmiStudioUiUpdateEditorPreferences(UmiStudioUi *ui, const UmiEditorIndentation *reviewed,
                                             const UmiEditorIndentation *candidate) {
  if (ui == NULL || UmiEditorIndentationValidate(reviewed) != UMI_STATUS_OK ||
      UmiEditorIndentationValidate(candidate) != UMI_STATUS_OK)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiEditorIndentation expected = *reviewed, next = *candidate, previous;
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  UmiStatus status = UmiDocumentCoordinatorGetIndentation(documents, &previous);
  if (status != UMI_STATUS_OK)
    return status;
  /* Document policy and this UI are owner-thread state. Set performs busy
   * admission without notifications. If the checked settings write loses
   * a race, restore the prior policy before returning; no UI refresh occurred. */
  status = UmiDocumentCoordinatorSetIndentation(documents, &next);
  if (status != UMI_STATUS_OK)
    return status;
  char beforeWidth[16], afterWidth[16];
  (void)snprintf(beforeWidth, sizeof beforeWidth, "%u", expected.tab_width);
  (void)snprintf(afterWidth, sizeof afterWidth, "%u", next.tab_width);
  const UmiSettingChange changes[] = {
      {UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, beforeWidth, afterWidth},
      {UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES, expected.insert_spaces ? "true" : "false",
       next.insert_spaces ? "true" : "false"}};
  status = UmiSettingsApplyChanges(umi_studio_services_settings(umi_studio_ui_services(ui)),
                                   changes, 2U);
  if (status != UMI_STATUS_OK) {
    UmiStatus restored = UmiDocumentCoordinatorSetIndentation(documents, &previous);
    if (restored != UMI_STATUS_OK)
      return UMI_STATUS_INTERNAL_ERROR;
  }
  return status;
}
