/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/resilience_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Resilience Report" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "resilience_report",
    "studio.resilience.report",
    "Resilience Report",
    "Operations",
    "Inspect supervised component resilience state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
