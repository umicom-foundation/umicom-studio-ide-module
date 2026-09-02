/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/language_main.c
 *
 * PURPOSE:
 *   Provide a native C23 command for Studio Language Server Protocol state and requests.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/language.h"
#include "umicom/studio/services.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioLanguageService *service;
    UmiStudioLanguageSnapshot snapshot;
    UmiStatus status;
    int exit_code = 0;

    status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_bootstrap_start(bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Studio language service startup failed: %s\n",
                      umi_status_text(status));
        umi_studio_bootstrap_destroy(bootstrap);
        return 1;
    }
    service = umi_studio_services_language(
        umi_studio_bootstrap_services(bootstrap));
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (argc == 2 && strcmp(argv[1], "--initialize") == 0) {
        int64_t request_id = 0;
        status = umi_studio_language_service_initialize(service, 0, &request_id);
        (void)printf("Initialise request: %lld\nStatus: %s\n",
                     (long long)request_id, umi_status_text(status));
        exit_code = status == UMI_STATUS_OK ? 0 : 1;
    } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (argc == 5 && strcmp(argv[1], "--open") == 0) {
        status = umi_studio_language_service_open(service,
                                                  argv[2],
                                                  argv[3],
                                                  1,
                                                  argv[4]);
        (void)printf("Open document: %s\n", umi_status_text(status));
        exit_code = status == UMI_STATUS_OK ? 0 : 1;
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        status = umi_studio_language_service_snapshot(service, &snapshot);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Client state: %d\nInitialized: %s\n"
                         "Open documents: %zu\nQueued messages: %zu\n",
                         (int)snapshot.client_state,
                         snapshot.initialized ? "yes" : "no",
                         snapshot.open_documents,
                         snapshot.queued_messages);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            exit_code = 1;
        }
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return exit_code;
}
