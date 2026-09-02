/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/include/umicom/studio/session_state.h
 *
 * PURPOSE:
 *   Serialize and restore workspace/document session state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#ifndef UMICOM_STUDIO_SESSION_STATE_H
#define UMICOM_STUDIO_SESSION_STATE_H

#include <stddef.h>

#include "umicom/base/status.h"
#include "umicom/studio/document_set.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_SESSION_TEXT_CAPACITY 32768U

/**
 * Represent the studio session state data shared with callers of this public contract.
 */
typedef struct UmiStudioSessionState {
    char workspace_path[UMI_STUDIO_WORKSPACE_PATH_CAPACITY];
    UmiStudioDocumentSet documents;
} UmiStudioSessionState;

/**
 * Initialise studio session state from caller-provided values so later operations receive
 * a known state.
 */
void umi_studio_session_state_init(UmiStudioSessionState *state);

/**
 * Provide the studio session state set workspace operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_state_set_workspace(
    UmiStudioSessionState *state,
    const char *workspace_path
);

/**
 * Write studio session state in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_session_state_encode(
    const UmiStudioSessionState *state,
    char *output,
    size_t output_capacity
);

/**
 * Read studio session state into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_session_state_decode(
    const char *text,
    UmiStudioSessionState *state
);

#ifdef __cplusplus
}
#endif

#endif
