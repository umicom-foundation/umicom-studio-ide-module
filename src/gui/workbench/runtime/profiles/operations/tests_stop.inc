/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_stop.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Stop Tests" for
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
    "tests_stop",
    "studio.tests.stop",
    "Stop Tests",
    "Testing",
    "Stop the active test operation.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
