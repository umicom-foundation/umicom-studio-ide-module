/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/messages_replay.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Replay Durable Messages" for
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
    "messages_replay",
    "studio.messages.replay",
    "Replay Durable Messages",
    "Messaging",
    "Replay durable messages using the default replay request.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
