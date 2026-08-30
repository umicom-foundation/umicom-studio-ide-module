/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_save_session.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Save AI Session" for
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
    "ai_save_session",
    "studio.ai.save-session",
    "Save AI Session",
    "AI",
    "Persist an AI session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
