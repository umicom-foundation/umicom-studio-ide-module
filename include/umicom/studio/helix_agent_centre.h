/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_centre.h
 * PURPOSE: Own Studio's Framework-backed Helix Agent Runtime v2 composition.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* The centre owns product policy and adapters.  All operation,
 * approval, retry and evidence rules stay in reusable Framework services. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_CENTRE_H
#define UMICOM_STUDIO_HELIX_AGENT_CENTRE_H

#include "umicom/studio/helix_agent_adapters.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio helix agent centre config data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioHelixAgentCentreConfig {
    uint32_t maximum_attempts;
    double minimum_fitness;
    int require_human_approval;
    int allow_filesystem;
    int allow_build;
    int allow_test;
    int allow_review;
    int allow_source_control;
} UmiStudioHelixAgentCentreConfig;

/**
 * Represent the studio helix agent centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioHelixAgentCentre UmiStudioHelixAgentCentre;

/**
 * Provide the studio helix agent centre config default operation used by this module and
 * its client applications.
 */
UmiStudioHelixAgentCentreConfig umi_studio_helix_agent_centre_config_default(void);
/**
 * Initialise studio helix agent centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_helix_agent_centre_create(
    const UmiStudioHelixAgentCentreConfig *config,
    UmiStudioHelixAgentCentre **out_centre);
/**
 * Release or reset state held by studio helix agent centre so the same storage can be
 * reused safely.
 */
void umi_studio_helix_agent_centre_destroy(UmiStudioHelixAgentCentre *centre);
/**
 * Provide the studio helix agent centre runtime operation used by this module and its
 * client applications.
 */
UmiHelixOrchestrator *umi_studio_helix_agent_centre_runtime(
    UmiStudioHelixAgentCentre *centre);
/**
 * Provide the studio helix agent centre adapter state operation used by this module and
 * its client applications.
 */
const UmiStudioHelixAgentAdapterState *umi_studio_helix_agent_centre_adapter_state(
    const UmiStudioHelixAgentCentre *centre);

#ifdef __cplusplus
}
#endif

#endif
