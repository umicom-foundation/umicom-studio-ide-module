/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/recovery_purge.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Purge Recovery Data" for
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
    "recovery_purge",
    "studio.recovery.purge",
    "Purge Recovery Data",
    "Recovery",
    "Remove retained Studio recovery data.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
