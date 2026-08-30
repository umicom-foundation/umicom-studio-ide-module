/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_select.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Test" for
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
    "tests_select",
    "studio.tests.select",
    "Select Test",
    "Testing",
    "Select a visible test by stable id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEST)
