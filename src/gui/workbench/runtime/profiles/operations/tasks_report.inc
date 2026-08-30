/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tasks_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Task Report" for
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
    "tasks_report",
    "studio.tasks.report",
    "Task Report",
    "Tasks",
    "Show the current task report.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
