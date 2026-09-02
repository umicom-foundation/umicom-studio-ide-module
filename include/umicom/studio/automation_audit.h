/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/automation_audit.h
 *
 * PURPOSE:
 *   Check that Studio actions, menus and toolbars are connected to real Umicom
 *   Framework commands before a user discovers an unresponsive control.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_AUTOMATION_AUDIT_H
#define UMICOM_STUDIO_AUTOMATION_AUDIT_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Summarise Studio's testable wiring. A disabled action is reported separately
 * because it may be waiting for a project, document or debug session and is not
 * necessarily broken. A missing command or action binding is always broken.
 */
typedef struct UmiStudioAutomationAudit {
    size_t action_count;
    size_t visible_action_count;
    size_t wired_action_count;
    size_t disabled_action_count;
    size_t missing_command_count;
    size_t menu_item_count;
    size_t missing_menu_action_count;
    size_t toolbar_item_count;
    size_t missing_toolbar_action_count;
    uint64_t action_revision;
    uint64_t menu_revision;
    uint64_t toolbar_revision;
    char first_problem_control[UMI_UI_ID_CAPACITY];
    char first_problem_reference[UMI_UI_ID_CAPACITY];
} UmiStudioAutomationAudit;

/** Inspect the live toolkit-neutral Studio workbench without opening a window. */
UmiStatus umi_studio_automation_audit_capture(
    UmiStudioUi *ui,
    UmiStudioAutomationAudit *out_audit);

/** Return OK only when every visible control reference is wired correctly. */
UmiStatus umi_studio_automation_audit_validate(
    const UmiStudioAutomationAudit *audit);

#ifdef __cplusplus
}
#endif

#endif
