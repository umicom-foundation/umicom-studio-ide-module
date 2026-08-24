/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_complete_code.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Complete Code" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_complete_code",
    "studio.ai.complete-code",
    "Complete Code",
    "AI",
    "Plan context-aware code completion.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
