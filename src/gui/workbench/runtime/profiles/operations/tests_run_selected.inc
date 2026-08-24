/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tests_run_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run Selected Tests" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "tests_run_selected",
    "studio.tests.run-selected",
    "Run Selected Tests",
    "Testing",
    "Run the selected Test Explorer item.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
