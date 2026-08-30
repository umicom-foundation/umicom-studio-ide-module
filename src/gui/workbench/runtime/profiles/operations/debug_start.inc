/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_start.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Start Debugging" for
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
    "debug_start",
    "studio.debug.start",
    "Start Debugging",
    "Debug",
    "Start the active debug configuration.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
