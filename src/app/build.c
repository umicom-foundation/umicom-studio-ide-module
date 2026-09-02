/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/build.c
 *
 * PURPOSE:
 *   Implement Studio build profiles, execution, diagnostic capture and retained history.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/build.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioBuildService {
    UmiBuildProfile profile;
    UmiBuildHistory *history;
    UmiBuildEngine *engine;
    UmiBuildArtifactIndex *artifacts;
    UmiBuildWorkspace *workspace;
    UmiCancellationToken *cancellation;
    UmiBuildResult last_result;
    int has_last_result;
};

/* Provide the copy text operation used by this module and its client applications. */
static UmiStatus copy_text(char *destination,
                           size_t capacity,
                           const char *source)
{
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    length = strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length + 1U > capacity) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    (void)memcpy(destination, source, length + 1U);
    return UMI_STATUS_OK;
}

/*
 * Initialise studio build service from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_build_service_create(const char *source_root,
                                           UmiClock *clock,
                                           UmiStudioBuildService **out_service)
{
    UmiStudioBuildService *service;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source_root == NULL || source_root[0] == '\0' || clock == NULL ||
        out_service == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_service = NULL;
    service = (UmiStudioBuildService *)calloc(1U, sizeof(*service));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    umi_build_profile_init(&service->profile);
    status = umi_build_profile_set(&service->profile,
                                   "studio.development",
                                   source_root,
                                   "build/umicom-development");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(service->profile.generator,
                           sizeof(service->profile.generator),
                           "Ninja");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(service->profile.configuration,
                           sizeof(service->profile.configuration),
                           "Debug");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(service->profile.install_directory,
                           sizeof(service->profile.install_directory),
                           "build/umicom-development/install");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
#ifdef _WIN32
        status = copy_text(service->profile.run_program,
                           sizeof(service->profile.run_program),
                           "build/umicom-development/bin/umicom-studio-ide.exe");
#else
        status = copy_text(service->profile.run_program,
                           sizeof(service->profile.run_program),
                           "build/umicom-development/bin/umicom-studio-ide");
#endif
    }
    service->profile.parallel_jobs = 4U;
    service->profile.timeout_ms = 0U;
    service->profile.build_testing = 1;
    service->profile.strict_warnings = 1;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_build_history_create(64U, &service->history);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_cancellation_token_create(&service->cancellation);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiBuildEngineConfig engine_config;
        (void)memset(&engine_config, 0, sizeof(engine_config));
        engine_config.profile = service->profile;
        engine_config.history = service->history;
        engine_config.clock = clock;
        engine_config.cancellation = service->cancellation;
        status = umi_build_engine_create(&engine_config, &service->engine);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_build_artifact_index_create(&service->artifacts);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiBuildWorkspaceBindings bindings;
        umi_build_workspace_bindings_init(&bindings);
        bindings.graph = umi_build_engine_graph(service->engine);
        bindings.history = service->history;
        bindings.artifacts = service->artifacts;
        bindings.profile = &service->profile;
        status = umi_build_workspace_create(&bindings, &service->workspace);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_build_service_destroy(service);
        return status;
    }
    *out_service = service;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio build service so the same storage can be reused
 * safely.
 */
void umi_studio_build_service_destroy(UmiStudioBuildService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_build_workspace_destroy(service->workspace);
    umi_build_artifact_index_destroy(service->artifacts);
    umi_build_engine_destroy(service->engine);
    umi_cancellation_token_destroy(service->cancellation);
    umi_build_history_destroy(service->history);
    free(service);
}

/*
 * Provide the studio build service bind task queue operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_bind_task_queue(
    UmiStudioBuildService *service,
    UmiTaskQueue *task_queue)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || task_queue == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    return umi_build_workspace_set_task_queue(service->workspace, task_queue);
}

/*
 * Provide the studio build service set profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_set_profile(
    UmiStudioBuildService *service,
    const UmiBuildProfile *profile)
{
    UmiStatus status;
    char message[256];
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || profile == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_profile_validate(profile, message, sizeof(message));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_build_engine_set_profile(service->engine, profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) service->profile = *profile;
    return status;
}

/*
 * Provide the studio build service prepare default graph operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_build_service_prepare_default_graph(
    UmiStudioBuildService *service,
    int include_run)
{
    UmiBuildGraph *graph;
    UmiBuildExecutionPolicy policy;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    graph = umi_build_engine_graph(service->engine);
    umi_build_execution_policy_init(&policy);
    policy.default_timeout_ms = service->profile.timeout_ms;
    policy.maximum_parallel_jobs = umi_build_policy_safe_parallel_jobs(
        service->profile.parallel_jobs, 0U, 0U);
    return umi_build_plan_populate_standard(graph, &policy, include_run);
}

/*
 * Provide the studio build service execute next operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_execute_next(
    UmiStudioBuildService *service,
    UmiBuildResult *out_result)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_result == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_engine_execute_next(service->engine, out_result);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_NOT_FOUND) {
        service->last_result = *out_result;
        service->has_last_result = 1;
        (void)umi_build_workspace_select_latest_operation(service->workspace);
    }
    return status;
}

/*
 * Provide the studio build service execute all operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_execute_all(
    UmiStudioBuildService *service,
    size_t maximum_nodes,
    size_t *out_executed_count)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_engine_execute_all(service->engine, maximum_nodes,
                                          out_executed_count);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_build_history_latest(service->history, &service->last_result) ==
        UMI_STATUS_OK) {
        service->has_last_result = 1;
        (void)umi_build_workspace_select_latest_operation(service->workspace);
    }
    return status;
}

/*
 * Provide the studio build service cancel operation used by this module and its client
 * applications.
 */
void umi_studio_build_service_cancel(UmiStudioBuildService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service != NULL) umi_build_engine_request_cancel(service->engine);
}

/*
 * Provide the studio build service retry operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_retry(UmiStudioBuildService *service,
                                         const char *node_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_engine_retry(service->engine, node_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        (void)umi_build_workspace_select_node(service->workspace, node_id);
    return status;
}

/*
 * Provide the studio build service invalidate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_invalidate(UmiStudioBuildService *service,
                                              const char *node_id,
                                              uint64_t input_revision)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_engine_invalidate(service->engine, node_id,
                                         input_revision);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        (void)umi_build_workspace_select_node(service->workspace, node_id);
    return status;
}

/*
 * Provide the studio build service graph operation used by this module and its client
 * applications.
 */
UmiBuildGraph *umi_studio_build_service_graph(UmiStudioBuildService *service)
{
    return service != NULL ? umi_build_engine_graph(service->engine) : NULL;
}

/*
 * Provide the studio build service artifacts operation used by this module and its client
 * applications.
 */
UmiBuildArtifactIndex *umi_studio_build_service_artifacts(
    UmiStudioBuildService *service)
{
    return service != NULL ? service->artifacts : NULL;
}

/*
 * Provide the studio build service record artifact operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_record_artifact(
    UmiStudioBuildService *service,
    const UmiBuildArtifactSnapshot *artifact)
{
    return service != NULL
        ? umi_build_artifact_index_upsert(service->artifacts, artifact)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Perform studio build service through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_build_service_run(UmiStudioBuildService *service,
                                       UmiBuildPhase phase,
                                       UmiBuildResult *out_result)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_result == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_build_engine_execute_phase(service->engine, phase, out_result);
    service->last_result = *out_result;
    service->has_last_result = 1;
    (void)umi_build_workspace_select_latest_operation(service->workspace);
    return status;
}

/*
 * Provide the studio build service snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_snapshot(
    const UmiStudioBuildService *service,
    UmiStudioBuildSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    (void)copy_text(out_snapshot->source_root,
                    sizeof(out_snapshot->source_root),
                    service->profile.source_directory);
    (void)copy_text(out_snapshot->build_directory,
                    sizeof(out_snapshot->build_directory),
                    service->profile.build_directory);
    (void)copy_text(out_snapshot->profile_id,
                    sizeof(out_snapshot->profile_id),
                    service->profile.profile_id);
    out_snapshot->history_count = umi_build_history_count(service->history);
    {
        UmiBuildEngineSnapshot engine_snapshot;
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_build_engine_snapshot(service->engine, &engine_snapshot) ==
            UMI_STATUS_OK)
            out_snapshot->next_operation_id = engine_snapshot.next_operation_id;
    }
    (void)umi_build_graph_snapshot(umi_build_engine_graph(service->engine),
                                   &out_snapshot->graph);
    out_snapshot->artifact_count =
        umi_build_artifact_index_count(service->artifacts);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (service->has_last_result) {
        out_snapshot->last_phase = service->last_result.phase;
        out_snapshot->last_state = service->last_result.state;
        out_snapshot->last_status = service->last_result.status;
        out_snapshot->last_exit_code = service->last_result.exit_code;
        out_snapshot->diagnostic_count =
            service->last_result.diagnostics.count;
    }
    return umi_build_workspace_snapshot(service->workspace,
                                        &out_snapshot->workspace);
}

/*
 * Provide the studio build service history operation used by this module and its client
 * applications.
 */
UmiBuildHistory *umi_studio_build_service_history(UmiStudioBuildService *service)
{
    return service != NULL ? service->history : NULL;
}

/*
 * Provide the studio build service profile operation used by this module and its client
 * applications.
 */
const UmiBuildProfile *umi_studio_build_service_profile(
    const UmiStudioBuildService *service)
{
    return service != NULL ? &service->profile : NULL;
}

/*
 * Provide the studio build service workspace operation used by this module and its client
 * applications.
 */
UmiBuildWorkspace *umi_studio_build_service_workspace(
    UmiStudioBuildService *service)
{
    return service != NULL ? service->workspace : NULL;
}
