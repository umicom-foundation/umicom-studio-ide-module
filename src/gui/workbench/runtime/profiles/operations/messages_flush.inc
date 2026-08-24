/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/messages_flush.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Flush Integration Outbox" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "messages_flush",
    "studio.messages.flush",
    "Flush Integration Outbox",
    "Messaging",
    "Flush pending Integration Fabric outbox messages.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
