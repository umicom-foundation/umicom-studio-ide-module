/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_add_watch.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Add Watch" for
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
    "debug_add_watch",
    "studio.debug.add-watch",
    "Add Watch",
    "Debug",
    "Add a watch expression.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
