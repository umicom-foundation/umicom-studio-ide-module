/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/replay.h
 *
 * PURPOSE:
 *   Declare deterministic replay of Studio durable journal messages through the shared Framework dispatcher with sequence and correlation filters.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_REPLAY_H
#define UMICOM_STUDIO_REPLAY_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio replay request data shared with callers of this public contract.
 */
typedef struct UmiStudioReplayRequest {
    uint64_t minimum_sequence;
    uint64_t maximum_sequence;
    uint64_t correlation_id;
    size_t maximum_messages;
} UmiStudioReplayRequest;

/**
 * Provide the studio replay request default operation used by this module and its client
 * applications.
 */
UmiStudioReplayRequest umi_studio_replay_request_default(void);
/**
 * Provide the studio replay operation used by this module and its client applications.
 */
UmiStatus umi_studio_replay(UmiStudioServices *services,
                            const UmiStudioReplayRequest *request,
                            size_t *out_replayed);

#ifdef __cplusplus
}
#endif

#endif
