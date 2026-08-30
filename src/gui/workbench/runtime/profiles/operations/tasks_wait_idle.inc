/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/tasks_wait_idle.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Wait for Tasks" for
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
    "tasks_wait_idle",
    "studio.tasks.wait-idle",
    "Wait for Tasks",
    "Tasks",
    "Wait for the task queue to become idle; optional timeout in ms.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_OPTIONAL)
