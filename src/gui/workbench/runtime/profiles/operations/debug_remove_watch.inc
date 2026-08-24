/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_remove_watch.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Remove Watch" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_remove_watch",
    "studio.debug.remove-watch",
    "Remove Watch",
    "Debug",
    "Remove a watch by stable id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
