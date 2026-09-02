/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/federated_workspace_session.h
 *
 * PURPOSE:
 *   Declare Studio's session identity at the Framework composition boundary.
 *   Framework owns recovery journals, layout history and monitor affinity.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FEDERATED_WORKSPACE_SESSION_H
#define UMICOM_STUDIO_FEDERATED_WORKSPACE_SESSION_H

#include "umicom/desktop/monitor_interaction.h"
#include "umicom/desktop/session_recovery.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio federated workspace session begin operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_federated_workspace_session_begin(
    UmiDesktopSessionRecovery *recovery,
    UmiDesktopMonitorInteraction *monitor_interaction);
/**
 * Provide the studio federated workspace session end operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_federated_workspace_session_end(
    UmiDesktopSessionRecovery *recovery);

#ifdef __cplusplus
}
#endif

#endif
