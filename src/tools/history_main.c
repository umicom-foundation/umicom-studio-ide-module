/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/history_main.c
 *
 * PURPOSE:
 *   Provide a native Studio command for appending and inspecting persistent operational history records.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/history.h"

#include <stdio.h>
#include <string.h>

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
    if (argc >= 4 && strcmp(argv[1], "append") == 0) {
        status = umi_studio_history_append(services,
                                           argv[2],
                                           UMI_STATUS_OK,
                                           argv[3],
                                           0U,
                                           NULL);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        size_t index;
        size_t count = umi_studio_history_count(services);
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < count; ++index) {
            UmiStudioHistoryRecord record;
            status = umi_studio_history_read(services, index, &record);
            /* Preserve the original failure result so the caller can respond to the correct cause. */
            if (status != UMI_STATUS_OK) break;
            (void)printf("%llu [%s] %s: %s\n",
                         (unsigned long long)record.sequence,
                         record.category,
                         umi_status_text(record.status),
                         record.message);
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (count == 0U) (void)printf("No Studio history records\n");
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : 1;
}
