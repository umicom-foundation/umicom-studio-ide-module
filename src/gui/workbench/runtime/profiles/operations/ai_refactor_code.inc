/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_refactor_code.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Refactor Code" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_refactor_code",
    "studio.ai.refactor-code",
    "Refactor Code",
    "AI",
    "Plan a governed refactoring.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
