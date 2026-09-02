/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/federated_workspace_session.c
 *
 * PURPOSE:
 *   Supply Studio's declarative session identity to Framework services. All
 *   checkpoint, transaction and monitor-affinity logic remains in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/federated_workspace_session.h"

/*
 * Provide the studio federated workspace session begin operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_federated_workspace_session_begin(
    UmiDesktopSessionRecovery *recovery,
    UmiDesktopMonitorInteraction *monitor_interaction)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (recovery == NULL || monitor_interaction == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_desktop_monitor_interaction_capture_affinities(
        monitor_interaction);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    return umi_desktop_session_recovery_begin(
        recovery, "studio-desktop", "org.umicom.studio");
}

/*
 * Provide the studio federated workspace session end operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_federated_workspace_session_end(
    UmiDesktopSessionRecovery *recovery)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (recovery == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_desktop_session_recovery_mark_clean(recovery);
}
