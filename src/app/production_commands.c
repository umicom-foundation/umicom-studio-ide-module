/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/production_commands.c
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/production_commands.h"
#include <string.h>
/*
 * Provide the studio production commands build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_production_commands_build(
    UmiStudioProductionCommandCatalogue *out_catalogue)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_catalogue == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_catalogue, 0, sizeof(*out_catalogue));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_application_production_control_command_count();
         ++index) {
        const UmiApplicationProductionControlCommand *command =
            umi_application_production_control_command_at(index);
        UmiStudioProductionCommandContribution *entry;
        /* Use the shared build helper when it is available from the parent composition. */
        if (command == NULL || out_catalogue->count >= 32U)
            return UMI_STATUS_CAPACITY_EXCEEDED;
        entry = &out_catalogue->entries[out_catalogue->count];
        entry->command = command;
        entry->menu_path = strcmp(command->category, "Workspace") == 0
            ? "Workspace/Recovery"
            : (strcmp(command->category, "Quality") == 0
               ? "Quality/Production" : "Application/Production");
        entry->view_id = "studio.production.centre";
        out_catalogue->mutating_count += (size_t)command->mutates_state;
        out_catalogue->count += 1U;
    }
    return UMI_STATUS_OK;
}

