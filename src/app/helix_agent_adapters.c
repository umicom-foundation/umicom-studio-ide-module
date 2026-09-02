/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/helix_agent_adapters.c
 * PURPOSE: Implement policy-aware Studio callbacks for Helix v2 actions.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/helix_agent_adapters.h"

#include <stdio.h>
#include <string.h>

/* Provide the execute action operation used by this module and its client applications. */
static UmiStatus execute_action(void *context, const UmiHelixAction *action,
                                char *out_evidence, size_t capacity)
{
    UmiStudioHelixAgentAdapterState *state = context;
    int allowed = 0;
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state == NULL || action == NULL || out_evidence == NULL || capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Select the behaviour associated with the requested command or state value. */
    switch (action->kind) {
        case UMI_HELIX_ACTION_READ: allowed = 1; break;
        case UMI_HELIX_ACTION_FILESYSTEM: allowed = state->allow_filesystem; break;
        case UMI_HELIX_ACTION_BUILD: allowed = state->allow_build; break;
        case UMI_HELIX_ACTION_TEST: allowed = state->allow_test; break;
        case UMI_HELIX_ACTION_REVIEW: allowed = state->allow_review; break;
        case UMI_HELIX_ACTION_SOURCE_CONTROL:
            allowed = state->allow_source_control;
            break;
        default: return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(out_evidence, capacity,
        "Studio %s adapter %s action %s for %s",
        umi_helix_action_kind_text(action->kind),
        allowed ? "accepted" : "blocked", action->action_id, action->target);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= capacity) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (!allowed) return UMI_STATUS_PERMISSION_DENIED;
    state->execution_count++;
    state->last_kind = action->kind;
    return UMI_STATUS_OK;
}

/*
 * Initialise studio helix agent adapters from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_helix_agent_adapters_init(
    UmiStudioHelixAgentAdapterState *state)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state != NULL) {
        (void)memset(state, 0, sizeof(*state));
        state->allow_filesystem = 1;
        state->allow_build = 1;
        state->allow_test = 1;
        state->allow_review = 1;
    }
}

/*
 * Initialise studio helix agent adapters from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_helix_agent_adapters_create(
    UmiStudioHelixAgentAdapterState *state,
    UmiHelixExecutionAdapter *out_adapter)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (state == NULL || out_adapter == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_adapter, 0, sizeof(*out_adapter));
    out_adapter->context = state;
    out_adapter->read = execute_action;
    out_adapter->filesystem = execute_action;
    out_adapter->build = execute_action;
    out_adapter->test = execute_action;
    out_adapter->review = execute_action;
    out_adapter->source_control = execute_action;
    return UMI_STATUS_OK;
}
