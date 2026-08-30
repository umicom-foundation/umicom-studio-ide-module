/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/watcher_scan.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Scan Workspace Changes" for
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
    "watcher_scan",
    "studio.watcher.scan",
    "Scan Workspace Changes",
    "Workspace",
    "Run one watcher scan.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
