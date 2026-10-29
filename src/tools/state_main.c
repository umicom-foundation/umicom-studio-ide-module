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
#include <stdlib.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
/* Preserve the former console entry point for review. The active entry point
 * adds copied service discovery and reports incomplete checked cleanup. */
#if 0
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
#endif

/* Keep identifiers on one output line even when a plug-in supplies control
 * characters. Formatting belongs to this console adapter; discovery is shared. */
static void PrintServiceToken(const char *text)
{
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != 0U; ++cursor) {
        if (*cursor < 0x20U || *cursor == 0x7fU || *cursor == (unsigned char)'\\')
            (void)printf("\\x%02x", (unsigned)*cursor);
        else (void)putchar((int)*cursor);
    }
}
static UmiStatus PrintServices(UmiStudioBootstrap *bootstrap)
{
    UmiServiceSnapshot *items = calloc(UMI_SERVICE_REGISTRY_MAX, sizeof(*items));
    if (items == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    size_t total = 0U;
    UmiStatus status = UmiStudioStateCopyServices(
        bootstrap, items, UMI_SERVICE_REGISTRY_MAX, &total);
    if (status == UMI_STATUS_OK) {
        (void)printf("Registered services: %zu\n", total);
        (void)puts("Service\tProvider\tVersion\tFlags");
        for (size_t index = 0U; index < total; ++index) {
            PrintServiceToken(items[index].service_id);
            (void)putchar('\t');
            PrintServiceToken(items[index].provider_module_id);
            (void)printf("\t%u.%u.%u\t0x%x\n",
                (unsigned)items[index].service_version.major,
                (unsigned)items[index].service_version.minor,
                (unsigned)items[index].service_version.patch,
                (unsigned)items[index].flags);
        }
    }
    free(items);
    return status;
}
/* Command discovery reuses Framework's copied prefix query. It deliberately
 * does not ask whether commands are enabled or run their handlers. */
static UmiStatus PrintCommands(UmiStudioBootstrap *bootstrap, const char *prefix)
{
    UmiCommandSnapshot *items = calloc(UMI_COMMAND_REGISTRY_MAX, sizeof(*items));
    if (items == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    size_t total = 0U;
    UmiStatus status = umi_command_registry_find_prefix(
        umi_studio_bootstrap_command_registry(bootstrap), prefix,
        items, UMI_COMMAND_REGISTRY_MAX, &total);
    if (status == UMI_STATUS_OK) {
        (void)printf("Matching commands: %zu\n", total);
        (void)puts("Command\tTitle\tCategory\tFlags");
        for (size_t index = 0U; index < total; ++index) {
            PrintServiceToken(items[index].command_id);
            (void)putchar('\t');
            PrintServiceToken(items[index].title);
            (void)putchar('\t');
            PrintServiceToken(items[index].category);
            (void)printf("\t0x%x\n", (unsigned)items[index].flags);
        }
    }
    free(items);
    return status;
}

static int ReleaseStudio(UmiStudioBootstrap **bootstrap)
{
    UmiStatus status = UmiStudioBootstrapRelease(bootstrap);
    if (status == UMI_STATUS_OK) return 0;
    (void)fprintf(stderr, "Studio cleanup remains incomplete: %s\n", umi_status_text(status));
    return 1;
}

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
        (void)ReleaseStudio(&bootstrap);
        return 1;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (argc == 2 && strcmp(argv[1], "--services") == 0) {
        status = PrintServices(bootstrap);
        if (status != UMI_STATUS_OK) {
            (void)fprintf(stderr, "Could not copy Studio services: %s\n", umi_status_text(status));
            exit_code = 1;
        }
    } else if ((argc == 2 || argc == 3) && strcmp(argv[1], "--commands") == 0) {
        status = PrintCommands(bootstrap, argc == 3 ? argv[2] : "");
        if (status != UMI_STATUS_OK) {
            (void)fprintf(stderr, "Could not copy Studio commands: %s\n", umi_status_text(status));
            exit_code = 1;
        }
    } else if (argc >= 3 && strcmp(argv[1], "--execute") == 0) {
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
    if (ReleaseStudio(&bootstrap) != 0) exit_code = 1;
    return exit_code;
}
