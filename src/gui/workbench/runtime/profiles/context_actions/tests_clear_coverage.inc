/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/context_actions/tests_clear_coverage.inc
 *
 * PURPOSE:
 *   Place the canonical "Clear Coverage" operation into the tests
 *   advanced-action section. The command behavior remains registered in the
 *   authoritative Framework command registry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_CONTEXT_ACTION(
    "tests",
    "studio.tests.clear-coverage",
    200U)
