/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/data_main.c
 *
 * PURPOSE:
 *   Provide a native Studio Data Server inspection and key/value command for integrity, migration and persistence checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/data.h"

#include <stdio.h>
#include <string.h>

/* Provide the fail operation used by this module and its client applications. */
static int fail(UmiStatus status)
{
    (void)fprintf(stderr, "Data command failed: %s\n", umi_status_text(status));
    return 1;
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
    if (status != UMI_STATUS_OK) return fail(status);
    services = umi_studio_bootstrap_services(bootstrap);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (argc >= 4 && strcmp(argv[1], "set") == 0) {
        status = umi_studio_data_transaction_set(services, argv[2], argv[3]);
    } else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (argc >= 3 && strcmp(argv[1], "get") == 0) {
        char value[4096];
        status = umi_studio_data_get(services, argv[2], value, sizeof(value));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) (void)printf("%s\n", value);
    } else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (argc >= 3 && strcmp(argv[1], "remove") == 0) {
        status = umi_studio_data_remove(services, argv[2]);
    } else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (argc >= 2 && strcmp(argv[1], "integrity") == 0) {
        UmiStudioDataReport report;
        status = umi_studio_data_report(services, &report);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Backend: %s\nPath: %s\nRecords: %zu\nSchema: %u\nIntegrity: %s\n",
                         report.backend_name,
                         report.path[0] != '\0' ? report.path : "memory",
                         report.records,
                         report.schema_version,
                         umi_status_text(report.integrity_status));
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        (void)printf("Usage: umicom-studio-data integrity | set KEY VALUE | get KEY | remove KEY\n");
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : fail(status);
}
