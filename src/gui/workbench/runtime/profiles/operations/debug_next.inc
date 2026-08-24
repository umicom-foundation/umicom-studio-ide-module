/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_next.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Step Over" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_next",
    "studio.debug.next",
    "Step Over",
    "Debug",
    "Step over in a debugger thread.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_THREAD)
