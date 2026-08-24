/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_run_all.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run All Tests" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "tests_run_all",
    "studio.tests.run-all",
    "Run All Tests",
    "Testing",
    "Run all visible tests.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
