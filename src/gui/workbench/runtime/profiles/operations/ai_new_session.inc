/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_new_session.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "New AI Session" for
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
    "ai_new_session",
    "studio.ai.new-session",
    "New AI Session",
    "AI",
    "Create an AI session using the supplied session id/title contract.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
