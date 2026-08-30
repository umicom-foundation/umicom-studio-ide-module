/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/session_save.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Save Studio Session" for
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
    "session_save",
    "studio.session.save",
    "Save Studio Session",
    "Session",
    "Persist Studio session state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
