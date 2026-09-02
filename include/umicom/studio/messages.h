/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/messages.h
 *
 * PURPOSE:
 *   Declare Studio Integration Fabric schemas, typed publication, subscriptions, outbox delivery, dead-letter handling and operational message reports.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_MESSAGES_H
#define UMICOM_STUDIO_MESSAGES_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_SCHEMA_EVENT "umicom.studio.event.v1"
#define UMI_STUDIO_SCHEMA_COMMAND "umicom.studio.command.v1"
#define UMI_STUDIO_SCHEMA_HISTORY "umicom.studio.history.v1"
#define UMI_STUDIO_SCHEMA_WORKFLOW "umicom.studio.workflow.v1"

/**
 * Represent the studio publish request data shared with callers of this public contract.
 */
typedef struct UmiStudioPublishRequest {
    UmiMessageKind kind;
    const char *schema_id;
    const char *name;
    const char *payload;
    const char *destination;
    const char *partition_key;
    uint32_t flags;
    uint64_t correlation_id;
    uint64_t causation_id;
} UmiStudioPublishRequest;

/**
 * Represent the studio message report data shared with callers of this public contract.
 */
typedef struct UmiStudioMessageReport {
    size_t schemas;
    size_t topics;
    size_t durable_messages;
    size_t journal_messages;
    UmiDispatcherStats dispatcher;
    UmiInboxStats inbox;
    UmiOutboxStats outbox;
    size_t dead_letters;
    UmiMessageMetrics metrics;
} UmiStudioMessageReport;

/**
 * Provide the studio publish request default operation used by this module and its client
 * applications.
 */
UmiStudioPublishRequest umi_studio_publish_request_default(void);
/**
 * Provide the studio messages subscribe operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_messages_subscribe(UmiStudioServices *services,
                                        const UmiSubscription *subscription,
                                        UmiMessageHandler handler,
                                        void *user_data,
                                        uint64_t *out_subscription_id);
/**
 * Provide the studio messages unsubscribe operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_messages_unsubscribe(UmiStudioServices *services,
                                          uint64_t subscription_id);
/**
 * Provide the studio messages publish operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_messages_publish(UmiStudioServices *services,
                                      const UmiStudioPublishRequest *request,
                                      uint64_t *out_sequence,
                                      size_t *out_deliveries);
/**
 * Provide the studio messages flush outbox operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_messages_flush_outbox(UmiStudioServices *services,
                                           size_t maximum_records,
                                           size_t *out_delivered,
                                           size_t *out_failed);
/**
 * Provide the studio messages report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_messages_report(UmiStudioServices *services,
                                     UmiStudioMessageReport *out_report);

#ifdef __cplusplus
}
#endif

#endif
