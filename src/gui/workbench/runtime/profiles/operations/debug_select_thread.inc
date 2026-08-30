/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_select_thread.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Thread" for
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
    "debug_select_thread",
    "studio.debug.select-thread",
    "Select Thread",
    "Debug",
    "Select a debugger thread.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
