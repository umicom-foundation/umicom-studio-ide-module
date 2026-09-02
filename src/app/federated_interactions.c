/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/federated_interactions.c
 *
 * PURPOSE:
 *   Contribute only Studio window/context declarations. Routing, delivery,
 *   refresh, acknowledgement and application isolation remain in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/federated_interactions.h"

#include <stdio.h>
#include <string.h>

typedef struct UmiStudioFederatedContextDefinition {
    const char *window_id;
    const char *context_type;
    int refresh_on_delivery;
} UmiStudioFederatedContextDefinition;

static const UmiStudioFederatedContextDefinition DEFINITIONS[] = {
    {"project-explorer", "project", 0},
    {"project-explorer", "file", 0},
    {"project-explorer", "symbol", 0},
    {"output", "run", 1},
    {"output", "diagnostic", 1}
};

/*
 * Add studio federated interactions only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_federated_interactions_register(
    UmiDesktopContextSynchronizer *synchronizer)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (synchronizer == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(DEFINITIONS) / sizeof(DEFINITIONS[0]);
         ++index) {
        UmiDesktopContextSubscription subscription;
        UmiStatus status;
        (void)memset(&subscription, 0, sizeof(subscription));
        subscription.structure_size = (uint32_t)sizeof(subscription);
        (void)snprintf(subscription.window_id,
                       sizeof(subscription.window_id), "%s",
                       DEFINITIONS[index].window_id);
        (void)snprintf(subscription.context_type,
                       sizeof(subscription.context_type), "%s",
                       DEFINITIONS[index].context_type);
        subscription.refresh_on_delivery =
            DEFINITIONS[index].refresh_on_delivery;
        status = umi_desktop_context_synchronizer_subscribe(
            synchronizer, &subscription);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}
