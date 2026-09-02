/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_adapters.h
 * PURPOSE: Adapt approved Helix v2 actions to Studio-owned product boundaries.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* These flags are product policy.  Framework verifies approval;
 * Studio separately decides which side-effect categories are available. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_ADAPTERS_H
#define UMICOM_STUDIO_HELIX_AGENT_ADAPTERS_H

#include <stdint.h>
#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio helix agent adapter state data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioHelixAgentAdapterState {
    int allow_filesystem;
    int allow_build;
    int allow_test;
    int allow_review;
    int allow_source_control;
    uint32_t execution_count;
    UmiHelixActionKind last_kind;
} UmiStudioHelixAgentAdapterState;

/**
 * Initialise studio helix agent adapters from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_helix_agent_adapters_init(
    UmiStudioHelixAgentAdapterState *state);
/**
 * Initialise studio helix agent adapters from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_helix_agent_adapters_create(
    UmiStudioHelixAgentAdapterState *state,
    UmiHelixExecutionAdapter *out_adapter);

#ifdef __cplusplus
}
#endif

#endif
