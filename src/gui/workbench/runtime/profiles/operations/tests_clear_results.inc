/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_clear_results.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Test Results" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "tests_clear_results",
    "studio.tests.clear-results",
    "Clear Test Results",
    "Testing",
    "Clear retained test results.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
