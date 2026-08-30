/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_discover.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Discover Tests" for
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
    "tests_discover",
    "studio.tests.discover",
    "Discover Tests",
    "Testing",
    "Discover tests in the configured build directory.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
