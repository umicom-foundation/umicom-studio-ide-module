/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/observability_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Observability Report" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "observability_report",
    "studio.observability.report",
    "Observability Report",
    "Operations",
    "Inspect Framework observability state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
