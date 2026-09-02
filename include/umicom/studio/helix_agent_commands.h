/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_commands.h
 * PURPOSE: Expose controlled Helix v2 plan, approval, run and rollback commands.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Command handlers stay thin and call the same centre used by
 * native tools and future graphical workbench actions. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_COMMANDS_H
#define UMICOM_STUDIO_HELIX_AGENT_COMMANDS_H

#include "umicom/studio/helix_agent_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio helix agent plan operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_plan(UmiStudioHelixAgentCentre *centre,
                                      const char *operation_id,
                                      const char *objective,
                                      const char *plan_hash,
                                      const UmiHelixAction *action);
/**
 * Provide the studio helix agent approve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_approve(UmiStudioHelixAgentCentre *centre,
                                         const char *approver,
                                         const char *reason);
/**
 * Provide the studio helix agent reject operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_reject(UmiStudioHelixAgentCentre *centre,
                                        const char *approver,
                                        const char *reason);
/**
 * Perform studio helix agent through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_helix_agent_run(UmiStudioHelixAgentCentre *centre,
                                     char *out_evidence,
                                     size_t capacity);
/**
 * Provide the studio helix agent add compensation operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_add_compensation(
    UmiStudioHelixAgentCentre *centre,
    const UmiHelixAction *action);
/**
 * Provide the studio helix agent begin rollback operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_begin_rollback(
    UmiStudioHelixAgentCentre *centre,
    int human_approved);
/**
 * Provide the studio helix agent run rollback operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_run_rollback(
    UmiStudioHelixAgentCentre *centre,
    char *out_evidence,
    size_t capacity);
/**
 * Provide the studio helix agent status operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_status(UmiStudioHelixAgentCentre *centre,
                                        char *out_text,
                                        size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
