/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_clear_output.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Test Output" for
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
    "tests_clear_output",
    "studio.tests.clear-output",
    "Clear Test Output",
    "Testing",
    "Clear retained test output.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
