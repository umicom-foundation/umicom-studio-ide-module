/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/replay.c
 *
 * PURPOSE:
 *   Implement filtered deterministic Studio replay by adapting the durable journal to the Framework replay contract and dispatching replay-marked messages.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/replay.h"

#include <string.h>

/*
 * Perform replay through the module contract so client applications do not duplicate its
 * policy.
 */
static UmiStatus replay_dispatch(const UmiMessageEnvelope *message,
                                 void *user_data)
{
    UmiStudioServices *services = (UmiStudioServices *)user_data;
    UmiMessageEnvelope replayed = *message;
    size_t deliveries = 0U;
    UmiStatus status;
    replayed.flags |= UMI_MESSAGE_FLAG_REPLAY;
    status = umi_dispatcher_dispatch(umi_studio_services_dispatcher(services),
                                     &replayed,
                                     &deliveries);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        umi_message_metrics_increment(umi_studio_services_message_metrics(services),
                                      UMI_MESSAGE_METRIC_REPLAYED);
    }
    return status;
}

/*
 * Provide the studio replay request default operation used by this module and its client
 * applications.
 */
UmiStudioReplayRequest umi_studio_replay_request_default(void)
{
    UmiStudioReplayRequest request;
    (void)memset(&request, 0, sizeof(request));
    request.maximum_sequence = UINT64_MAX;
    request.maximum_messages = SIZE_MAX;
    return request;
}

/* Provide the studio replay operation used by this module and its client applications. */
UmiStatus umi_studio_replay(UmiStudioServices *services,
                            const UmiStudioReplayRequest *request,
                            size_t *out_replayed)
{
    UmiReplayOptions options;
    UmiReplaySource source;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || request == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    options = umi_replay_options_default();
    options.minimum_sequence = request->minimum_sequence;
    options.maximum_sequence = request->maximum_sequence;
    options.correlation_id = request->correlation_id;
    options.maximum_messages = request->maximum_messages;
    source = umi_message_store_replay_source(
        umi_studio_services_journal(services)->messages);
    return umi_replay_execute(&source,
                              &options,
                              replay_dispatch,
                              services,
                              out_replayed);
}
