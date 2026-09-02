/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/state_main.c
 *
 * PURPOSE:
 *   Provide a native C23 operational-state command for inspecting Studio's
 *   Framework registries and executing canonical Studio commands without GTK4.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/state.h"

#include <stdio.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStatus status;
    int exit_code = 0;

    status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "Could not create Studio: %s\n",
                      umi_status_text(status));
        return 1;
    }

    status = umi_studio_bootstrap_start(bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "Could not start Studio: %s\n",
                      umi_status_text(status));
        umi_studio_bootstrap_destroy(bootstrap);
        return 1;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (argc >= 3 && strcmp(argv[1], "--execute") == 0) {
        char message[512];
        const char *argument = argc >= 4 ? argv[3] : "";
        status = umi_command_registry_execute(
            umi_studio_bootstrap_command_registry(bootstrap),
            argv[2],
            argument,
            message,
            sizeof(message)
        );
        /* Apply this branch only when its contract condition is satisfied. */
        if (message[0] != '\0') {
            (void)printf("%s\n", message);
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            (void)fprintf(stderr,
                          "Command failed: %s\n",
                          umi_status_text(status));
            exit_code = 1;
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        UmiStudioStateReport report;
        char text[1024];

        status = umi_studio_state_capture(bootstrap, &report);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_studio_state_format(&report,
                                             text,
                                             sizeof(text));
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("%s", text);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            (void)fprintf(stderr,
                          "Could not capture Studio state: %s\n",
                          umi_status_text(status));
            exit_code = 1;
        }
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_bootstrap_stop(bootstrap) != UMI_STATUS_OK) {
        exit_code = 1;
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return exit_code;
}
