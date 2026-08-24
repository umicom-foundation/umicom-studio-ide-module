/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_refresh_health.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Refresh AI Health" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_refresh_health",
    "studio.ai.refresh-health",
    "Refresh AI Health",
    "AI",
    "Refresh configured AI runtime health.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
