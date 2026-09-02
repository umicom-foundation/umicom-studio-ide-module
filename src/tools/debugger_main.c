/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/debugger_main.c
 *
 * PURPOSE:
 *   Provide a native C23 command for Studio Debug Adapter Protocol state and breakpoints.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/debugger.h"
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
    UmiStudioDebuggerService *service;
    UmiStudioDebuggerSnapshot snapshot;
    UmiStatus status;
    int exit_code = 0;

    status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_bootstrap_start(bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Studio debugger startup failed: %s\n",
                      umi_status_text(status));
        umi_studio_bootstrap_destroy(bootstrap);
        return 1;
    }
    service = umi_studio_services_debugger(
        umi_studio_bootstrap_services(bootstrap));
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (argc >= 2 && strcmp(argv[1], "--initialize") == 0) {
        int64_t request_id = 0;
        const char *adapter = argc >= 3 ? argv[2] : "umicom.debug";
        status = umi_studio_debugger_service_initialize(service,
                                                         adapter,
                                                         &request_id);
        (void)printf("Initialise request: %lld\nStatus: %s\n",
                     (long long)request_id, umi_status_text(status));
        exit_code = status == UMI_STATUS_OK ? 0 : 1;
    } else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (argc >= 4 && strcmp(argv[1], "--breakpoint") == 0) {
        int line = atoi(argv[3]);
        int column = argc >= 5 ? atoi(argv[4]) : 1;
        status = umi_studio_debugger_service_add_breakpoint(service,
                                                             argv[2],
                                                             line,
                                                             column);
        (void)printf("Breakpoint: %s\n", umi_status_text(status));
        exit_code = status == UMI_STATUS_OK ? 0 : 1;
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        status = umi_studio_debugger_service_snapshot(service, &snapshot);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Client state: %d\nInitialized: %s\n"
                         "Breakpoints: %zu\nQueued messages: %zu\n",
                         (int)snapshot.client_state,
                         snapshot.initialized ? "yes" : "no",
                         snapshot.breakpoint_count,
                         snapshot.queued_messages);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            exit_code = 1;
        }
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return exit_code;
}
