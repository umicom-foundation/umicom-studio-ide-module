/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_patch_revert.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Revert AI Patch" for
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
    "ai_patch_revert",
    "studio.ai.patch-revert",
    "Revert AI Patch",
    "AI",
    "Revert a Framework AI patch by id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
