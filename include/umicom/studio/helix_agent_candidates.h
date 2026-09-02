/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_candidates.h
 * PURPOSE: Expose candidate scoring and comparison through Studio composition.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Studio does not invent a separate ranking algorithm; it uses
 * Framework's deterministic eligible-candidate selection contract. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_CANDIDATES_H
#define UMICOM_STUDIO_HELIX_AGENT_CANDIDATES_H

#include "umicom/studio/helix_agent_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add studio helix agent candidate only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_helix_agent_candidate_add(
    UmiStudioHelixAgentCentre *centre,
    const char *candidate_id,
    double fitness,
    const char *provenance_hash);
/**
 * Provide the studio helix agent candidate select operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_candidate_select(
    UmiStudioHelixAgentCentre *centre,
    UmiHelixCandidateScore *out_candidate);

#ifdef __cplusplus
}
#endif

#endif
