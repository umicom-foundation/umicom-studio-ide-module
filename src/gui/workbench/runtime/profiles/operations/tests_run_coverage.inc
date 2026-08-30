/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_run_coverage.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run Test Coverage" for
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
    "tests_run_coverage",
    "studio.tests.run-coverage",
    "Run Test Coverage",
    "Testing",
    "Run the selected test coverage workflow.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
