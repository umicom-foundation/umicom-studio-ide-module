/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_repeat_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Repeat Selected Test" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "tests_repeat_selected",
    "studio.tests.repeat-selected",
    "Repeat Selected Test",
    "Testing",
    "Repeat the selected test.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
