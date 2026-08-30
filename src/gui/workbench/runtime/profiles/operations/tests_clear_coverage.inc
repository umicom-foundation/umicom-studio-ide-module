/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_clear_coverage.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Coverage" for
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
    "tests_clear_coverage",
    "studio.tests.clear-coverage",
    "Clear Coverage",
    "Testing",
    "Clear retained coverage data.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
