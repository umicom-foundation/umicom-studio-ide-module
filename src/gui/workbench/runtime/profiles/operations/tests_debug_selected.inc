/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_debug_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Debug Selected Test" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "tests_debug_selected",
    "studio.tests.debug-selected",
    "Debug Selected Test",
    "Testing",
    "Debug the selected test.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
