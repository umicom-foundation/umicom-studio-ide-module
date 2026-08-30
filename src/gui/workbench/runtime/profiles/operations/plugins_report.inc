/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/plugins_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Plug-in Report" for
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
    "plugins_report",
    "studio.plugins.report",
    "Plug-in Report",
    "Extensions",
    "Inspect registered plug-ins and contributions.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
