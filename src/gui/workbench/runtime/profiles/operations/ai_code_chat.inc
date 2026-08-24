/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_code_chat.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "AI Code Chat" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_code_chat",
    "studio.ai.code-chat",
    "AI Code Chat",
    "AI",
    "Plan a repository-aware coding conversation.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
