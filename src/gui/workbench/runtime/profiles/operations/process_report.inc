/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/process_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Process Report" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "process_report",
    "studio.process.report",
    "Process Report",
    "Processes",
    "Inspect supervised processes.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
