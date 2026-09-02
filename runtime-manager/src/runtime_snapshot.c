/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/src/runtime_snapshot.c
 *
 * PURPOSE:
 *   Build summary counts used by the Application Hub dashboard.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#include "umicom/studio/runtime_snapshot.h"

#include <string.h>

/*
 * Provide the studio runtime snapshot build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_runtime_snapshot_build(
    const UmiStudioRuntimeManager *manager,
    UmiStudioRuntimeSnapshot *snapshot)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(snapshot, 0, sizeof(*snapshot));
    snapshot->total = manager->count;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < manager->count; ++index) {
        const UmiStudioRuntimeEntry *entry = &manager->entries[index];
        UmiIntegrationApplicationState state =
            umi_studio_runtime_manager_state(manager, entry->application.id);

        /* Apply this branch only when its contract condition is satisfied. */
        if (entry->installed) {
            ++snapshot->installed;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (entry->favourite) {
            ++snapshot->favourites;
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (entry->category > UMI_STUDIO_RUNTIME_CATEGORY_ALL &&
            entry->category < UMI_STUDIO_RUNTIME_CATEGORY_COUNT) {
            ++snapshot->by_category[entry->category];
        }

        /* Select the behaviour associated with the requested command or state value. */
        switch (state) {
            case UMI_INTEGRATION_APP_RUNNING:
                ++snapshot->running;
                break;
            case UMI_INTEGRATION_APP_STOPPED:
                ++snapshot->stopped;
                break;
            case UMI_INTEGRATION_APP_FAILED:
                ++snapshot->failed;
                break;
            default:
                break;
        }
    }

    return UMI_STATUS_OK;
}
