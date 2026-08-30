/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_filter.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Filter Tests" for
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
    "tests_filter",
    "studio.tests.filter",
    "Filter Tests",
    "Testing",
    "Set Test Explorer search/filter text.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
