/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/automation_audit.c
 *
 * PURPOSE:
 *   Audit Studio control wiring through Framework models. This check is
 *   headless, which makes it suitable for local tests and continuous builds.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/automation_audit.h"

#include <stdio.h>
#include <string.h>

/* Preserve the first failure so a short test report names a useful control. */
static void automation_record_first_problem(
    UmiStudioAutomationAudit *audit,
    const char *control_id,
    const char *reference_id)
{
    if (audit == NULL || audit->first_problem_control[0] != '\0') return;

    (void)snprintf(
        audit->first_problem_control,
        sizeof(audit->first_problem_control),
        "%s",
        control_id != NULL ? control_id : "");
    (void)snprintf(
        audit->first_problem_reference,
        sizeof(audit->first_problem_reference),
        "%s",
        reference_id != NULL ? reference_id : "");
}

/* Inspect every visible action and verify that its command exists. */
static UmiStatus automation_audit_actions(
    UmiUiActionModel *actions,
    UmiCommandRegistry *commands,
    UmiStudioAutomationAudit *audit)
{
    size_t index;

    if (actions == NULL || commands == NULL || audit == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    audit->action_count = umi_ui_action_model_count(actions);
    audit->action_revision = umi_ui_action_model_revision(actions);

    for (index = 0U; index < audit->action_count; ++index) {
        UmiUiActionSnapshot action;
        UmiStatus status = umi_ui_action_model_at(actions, index, &action);

        if (status != UMI_STATUS_OK) return status;
        if (!action.visible) continue;

        audit->visible_action_count += 1U;
        if (action.command_id[0] == '\0' ||
            !umi_command_registry_contains(commands, action.command_id)) {
            audit->missing_command_count += 1U;
            automation_record_first_problem(
                audit,
                action.action_id,
                action.command_id);
            continue;
        }

        audit->wired_action_count += 1U;
        if (!action.enabled ||
            !umi_command_registry_is_enabled(
                commands,
                action.command_id,
                action.argument)) {
            audit->disabled_action_count += 1U;
        }
    }

    return UMI_STATUS_OK;
}

/* Check that each non-separator menu row points to an existing action. */
static UmiStatus automation_audit_menus(
    UmiUiMenuModel *menus,
    UmiUiActionModel *actions,
    UmiStudioAutomationAudit *audit)
{
    size_t index;
    size_t count;

    if (menus == NULL || actions == NULL || audit == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Read the model only after its pointer has passed the boundary check. */
    count = umi_ui_menu_model_count(menus);
    audit->menu_revision = umi_ui_menu_model_revision(menus);
    for (index = 0U; index < count; ++index) {
        UmiUiMenuSnapshot menu_item;
        UmiUiActionSnapshot action;
        UmiStatus status = umi_ui_menu_model_at(menus, index, &menu_item);

        if (status != UMI_STATUS_OK) return status;
        if (menu_item.separator) continue;

        audit->menu_item_count += 1U;
        if (menu_item.action_id[0] == '\0' ||
            umi_ui_action_model_find(
                actions,
                menu_item.action_id,
                &action) != UMI_STATUS_OK) {
            audit->missing_menu_action_count += 1U;
            automation_record_first_problem(
                audit,
                menu_item.item_id,
                menu_item.action_id);
        }
    }

    return UMI_STATUS_OK;
}

/* Check that each non-separator toolbar button points to an existing action. */
static UmiStatus automation_audit_toolbars(
    UmiUiToolbarModel *toolbars,
    UmiUiActionModel *actions,
    UmiStudioAutomationAudit *audit)
{
    size_t index;
    size_t count;

    if (toolbars == NULL || actions == NULL || audit == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Read the model only after its pointer has passed the boundary check. */
    count = umi_ui_toolbar_model_count(toolbars);
    audit->toolbar_revision = umi_ui_toolbar_model_revision(toolbars);
    for (index = 0U; index < count; ++index) {
        UmiUiToolbarSnapshot toolbar_item;
        UmiUiActionSnapshot action;
        UmiStatus status = umi_ui_toolbar_model_at(
            toolbars,
            index,
            &toolbar_item);

        if (status != UMI_STATUS_OK) return status;
        if (toolbar_item.separator) continue;

        audit->toolbar_item_count += 1U;
        if (toolbar_item.action_id[0] == '\0' ||
            umi_ui_action_model_find(
                actions,
                toolbar_item.action_id,
                &action) != UMI_STATUS_OK) {
            audit->missing_toolbar_action_count += 1U;
            automation_record_first_problem(
                audit,
                toolbar_item.item_id,
                toolbar_item.action_id);
        }
    }

    return UMI_STATUS_OK;
}

/* Capture action, menu and toolbar wiring from Studio's headless workbench models. */
UmiStatus umi_studio_automation_audit_capture(
    UmiStudioUi *ui,
    UmiStudioAutomationAudit *out_audit)
{
    UmiUiWorkbench *workbench;
    UmiUiActionModel *actions;
    UmiStatus status;

    if (ui == NULL || out_audit == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_audit, 0, sizeof(*out_audit));

    workbench = umi_studio_ui_workbench(ui);
    if (workbench == NULL) return UMI_STATUS_INVALID_STATE;

    actions = umi_ui_workbench_actions(workbench);
    status = automation_audit_actions(
        actions,
        umi_ui_workbench_commands(workbench),
        out_audit);
    if (status != UMI_STATUS_OK) return status;

    status = automation_audit_menus(
        umi_ui_workbench_menus(workbench),
        actions,
        out_audit);
    if (status != UMI_STATUS_OK) return status;

    return automation_audit_toolbars(
        umi_ui_workbench_toolbars(workbench),
        actions,
        out_audit);
}

/* Fail the release gate when a visible control has no executable reference. */
UmiStatus umi_studio_automation_audit_validate(
    const UmiStudioAutomationAudit *audit)
{
    if (audit == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    if (audit->visible_action_count == 0U || audit->menu_item_count == 0U ||
        audit->toolbar_item_count == 0U) {
        return UMI_STATUS_INVALID_STATE;
    }

    if (audit->missing_command_count != 0U ||
        audit->missing_menu_action_count != 0U ||
        audit->missing_toolbar_action_count != 0U) {
        return UMI_STATUS_NOT_FOUND;
    }

    return UMI_STATUS_OK;
}
