/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_patch_apply.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Apply AI Patch" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_patch_apply",
    "studio.ai.patch-apply",
    "Apply AI Patch",
    "AI",
    "Apply an approved Framework AI patch by id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
