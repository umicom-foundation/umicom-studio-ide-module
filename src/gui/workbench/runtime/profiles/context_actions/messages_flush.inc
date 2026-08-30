/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/context_actions/messages_flush.inc
 *
 * PURPOSE:
 *   Place the canonical "Flush Integration Outbox" operation into the workspace
 *   advanced-action section. The command behavior remains registered in the
 *   authoritative Framework command registry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_CONTEXT_ACTION(
    "workspace",
    "studio.messages.flush",
    151U)
