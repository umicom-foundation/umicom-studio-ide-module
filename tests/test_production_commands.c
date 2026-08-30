/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_production_commands.c
 *
 * PURPOSE:
 *   Verify the test production commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE production control-plane test | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include "umicom/studio/production_commands.h"
int main(void) {
    UmiStudioProductionCommandCatalogue commands;
    assert(umi_studio_production_commands_build(&commands) == UMI_STATUS_OK);
    assert(commands.count == umi_application_production_control_command_count());
    assert(commands.mutating_count > 0U);
    return 0;
}

