/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_generate_tests.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Generate Tests" for
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
    "ai_generate_tests",
    "studio.ai.generate-tests",
    "Generate Tests",
    "AI",
    "Plan tests for workspace code.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
