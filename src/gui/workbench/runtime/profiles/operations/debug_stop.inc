/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_stop.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Stop Debugging" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_stop",
    "studio.debug.stop",
    "Stop Debugging",
    "Debug",
    "Stop the active debugger session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
