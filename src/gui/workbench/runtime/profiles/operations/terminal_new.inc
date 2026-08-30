/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_new.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "New Terminal" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_new",
    "studio.terminal.new",
    "New Terminal",
    "Terminal",
    "Create another terminal session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
