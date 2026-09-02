/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/build_main.c
 *
 * PURPOSE:
 *   Provide a native C23 command for inspecting and executing Studio build profiles.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/services.h"

#include <stdio.h>
#include <string.h>

/* Provide the parse phase operation used by this module and its client applications. */
static int parse_phase(const char *value, UmiBuildPhase *out_phase)
{
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(value, "configure") == 0) *out_phase = UMI_BUILD_PHASE_CONFIGURE;
    else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(value, "build") == 0) *out_phase = UMI_BUILD_PHASE_BUILD;
    else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(value, "test") == 0) *out_phase = UMI_BUILD_PHASE_TEST;
    else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(value, "clean") == 0) *out_phase = UMI_BUILD_PHASE_CLEAN;
    else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(value, "run") == 0) *out_phase = UMI_BUILD_PHASE_RUN;
    /* Use this fallback path when the earlier condition does not apply. */
    else return 0;
    return 1;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioBuildService *service;
    UmiStudioBuildSnapshot snapshot;
    UmiBuildResult result;
    UmiStatus status;
    int exit_code = 0;

    status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_bootstrap_start(bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Studio build service startup failed: %s\n",
                      umi_status_text(status));
        umi_studio_bootstrap_destroy(bootstrap);
        return 1;
    }
    service = umi_studio_services_build(
        umi_studio_bootstrap_services(bootstrap));

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (argc == 2 && strcmp(argv[1], "--graph") == 0) {
        UmiBuildGraphNodeSnapshot node;
        size_t index;
        status = umi_studio_build_service_prepare_default_graph(service, 1);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK && status != UMI_STATUS_ALREADY_EXISTS) {
            exit_code = 1;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            (void)umi_studio_build_service_snapshot(service, &snapshot);
            (void)printf("Build graph: %zu nodes, %zu dependencies, progress %u.%02u%%\n",
                         snapshot.graph.node_count,
                         snapshot.graph.dependency_count,
                         snapshot.graph.progress_basis_points / 100U,
                         snapshot.graph.progress_basis_points % 100U);
            /* Visit each bounded item once so every record receives the same rule. */
            for (index = 0U; index < snapshot.graph.node_count; ++index) {
                /* Apply this branch only when its contract condition is satisfied. */
                if (umi_build_graph_at(umi_studio_build_service_graph(service),
                                       index, &node) == UMI_STATUS_OK) {
                    (void)printf("  %-10s %-10s %s\n", node.node_id,
                                 umi_build_node_state_text(node.state),
                                 umi_build_phase_text(node.phase));
                }
            }
        }
    } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (argc == 2 && strcmp(argv[1], "--execute-graph") == 0) {
        size_t executed = 0U;
        status = umi_studio_build_service_prepare_default_graph(service, 0);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_ALREADY_EXISTS) status = UMI_STATUS_OK;
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK)
            status = umi_studio_build_service_execute_all(service, 0U,
                                                          &executed);
        (void)printf("Executed nodes: %zu\nStatus: %s\n", executed,
                     umi_status_text(status));
        exit_code = status == UMI_STATUS_OK ? 0 : 1;
    } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (argc == 3 && strcmp(argv[1], "--phase") == 0) {
        UmiBuildPhase phase;
        /* Apply this branch only when its contract condition is satisfied. */
        if (!parse_phase(argv[2], &phase)) {
            (void)fprintf(stderr, "Unknown build phase: %s\n", argv[2]);
            exit_code = 1;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            status = umi_studio_build_service_run(service, phase, &result);
            (void)printf("%s: %s\nExit code: %d\nDiagnostics: %zu\n",
                         umi_build_phase_text(phase),
                         umi_status_text(status),
                         result.exit_code,
                         result.diagnostics.count);
            exit_code = status == UMI_STATUS_OK && result.exit_code == 0
                ? 0 : 1;
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        status = umi_studio_build_service_snapshot(service, &snapshot);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Build profile: %s\nSource: %s\nBuild: %s\n"
                         "History: %zu\nArtifacts: %zu\nGraph nodes: %zu\n"
                         "Next operation: %llu\n",
                         snapshot.profile_id,
                         snapshot.source_root,
                         snapshot.build_directory,
                         snapshot.history_count,
                         snapshot.artifact_count,
                         snapshot.graph.node_count,
                         (unsigned long long)snapshot.next_operation_id);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            exit_code = 1;
        }
    }
    umi_studio_bootstrap_destroy(bootstrap);
    return exit_code;
}
