/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/messages_main.c
 *
 * PURPOSE:
 *   Provide a native Studio Integration Fabric command for typed publication, subscriptions, delivery and operational reporting.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/messages.h"

#include <stdio.h>
#include <string.h>

/* Provide the print message operation used by this module and its client applications. */
static UmiStatus print_message(const UmiMessageEnvelope *message, void *user_data)
{
    (void)user_data;
    (void)printf("[%s][%s] %s\n",
                 umi_message_kind_text(message->kind),
                 message->name,
                 message->payload != NULL ? message->payload : "");
    return UMI_STATUS_OK;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServices *services;
    UmiStatus status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return 1;
    services = umi_studio_bootstrap_services(bootstrap);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (argc >= 4 && strcmp(argv[1], "publish") == 0) {
        UmiSubscription subscription = umi_subscription_all();
        UmiStudioPublishRequest request = umi_studio_publish_request_default();
        uint64_t subscription_id = 0U;
        size_t deliveries = 0U;
        request.name = argv[2];
        request.payload = argv[3];
        status = umi_studio_messages_subscribe(services,
                                               &subscription,
                                               print_message,
                                               NULL,
                                               &subscription_id);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_studio_messages_publish(services,
                                                 &request,
                                                 NULL,
                                                 &deliveries);
        }
        (void)umi_studio_messages_unsubscribe(services, subscription_id);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Deliveries: %zu\n", deliveries);
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        UmiStudioMessageReport report;
        status = umi_studio_messages_report(services, &report);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Schemas: %zu\nTopics: %zu\nJournal: %zu\nHistory: %zu\nOutbox pending: %zu\nDead letters: %zu\nDispatched: %llu\n",
                         report.schemas,
                         report.topics,
                         report.journal_messages,
                         report.durable_messages,
                         report.outbox.pending,
                         report.dead_letters,
                         (unsigned long long)report.dispatcher.dispatched);
        }
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : 1;
}
