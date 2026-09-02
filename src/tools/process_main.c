/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/process_main.c
 *
 * PURPOSE:
 *   Run one trusted-workspace child process through the Framework supervisor
 *   and print its final state, exit code, duration and captured output.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/processes.h"
#include "umicom/studio/workspace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Provide the usage operation used by this module and its client applications. */
static void usage(void)
{
    (void)fprintf(stderr,
                  "Usage: umicom-studio-process ROOT [--timeout MS] -- PROGRAM [ARGS...]\n");
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioServices *services = NULL;
    UmiProcessRequest request;
    UmiProcessJobSnapshot snapshot;
    UmiProcessJobId job_id = 0U;
    UmiStatus status;
    uint32_t timeout_ms = 30000U;
    int separator = -1;
    int index;

    /* Apply this branch only when its contract condition is satisfied. */
    if (argc < 4) {
        usage();
        return 2;
    }
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 2; index < argc; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(argv[index], "--") == 0) {
            separator = index;
            break;
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(argv[index], "--timeout") == 0 && index + 1 < argc) {
            timeout_ms = (uint32_t)strtoul(argv[++index], NULL, 10);
        }
    }
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (separator < 0 || separator + 1 >= argc) {
        usage();
        return 2;
    }

    status = umi_studio_services_create(NULL, NULL, &services);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_workspace_open(services, argv[1], 1, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Workspace failed: %s\n", umi_status_text(status));
        umi_studio_services_destroy(services);
        return 1;
    }

    (void)memset(&request, 0, sizeof(request));
    request.program = argv[separator + 1];
    request.arguments = (const char *const *)&argv[separator + 2];
    request.argument_count = (size_t)(argc - separator - 2);
    request.capture_stdout = 1;
    request.capture_stderr = 1;
    request.timeout_ms = timeout_ms;
    request.poll_interval_ms = 10U;

    status = umi_studio_process_submit(services,
                                       "studio-process-command",
                                       &request,
                                       &job_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_process_wait(services, job_id, timeout_ms + 1000U);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_process_snapshot(services, job_id, &snapshot);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Process failed: %s\n", umi_status_text(status));
        umi_studio_services_destroy(services);
        return 1;
    }

    (void)printf("State: %s\n", umi_process_job_state_text(snapshot.state));
    (void)printf("Exit code: %d\n", snapshot.exit_code);
    (void)printf("Duration: %llu ms\n",
                 (unsigned long long)snapshot.duration_ms);
    /* Apply this branch only when its contract condition is satisfied. */
    if (snapshot.output[0] != '\0') (void)printf("%s", snapshot.output);

    umi_studio_services_destroy(services);
    return snapshot.state == UMI_PROCESS_JOB_SUCCEEDED ? 0 : 1;
}
