/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/build.h
 *
 * PURPOSE:
 *   Expose Studio build profiles, execution, diagnostics and history through Framework build services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_BUILD_H
#define UMICOM_STUDIO_BUILD_H

#include <stddef.h>

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio build service data shared with callers of this public contract.
 */
typedef struct UmiStudioBuildService UmiStudioBuildService;

/**
 * Represent the studio build snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioBuildSnapshot {
    char source_root[UMI_BUILD_PATH_CAPACITY];
    char build_directory[UMI_BUILD_PATH_CAPACITY];
    char profile_id[UMI_BUILD_ID_CAPACITY];
    size_t history_count;
    uint64_t next_operation_id;
    UmiBuildPhase last_phase;
    UmiBuildState last_state;
    UmiStatus last_status;
    int last_exit_code;
    size_t diagnostic_count;
    UmiBuildGraphSnapshot graph;
    size_t artifact_count;
    UmiBuildWorkspaceSnapshot workspace;
} UmiStudioBuildSnapshot;

/**
 * Initialise studio build service from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_build_service_create(const char *source_root,
                                           UmiClock *clock,
                                           UmiStudioBuildService **out_service);
/**
 * Release or reset state held by studio build service so the same storage can be reused
 * safely.
 */
void umi_studio_build_service_destroy(UmiStudioBuildService *service);
/**
 * Provide the studio build service set profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_set_profile(
    UmiStudioBuildService *service,
    const UmiBuildProfile *profile
);
/**
 * Provide the studio build service bind task queue operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_bind_task_queue(
    UmiStudioBuildService *service,
    UmiTaskQueue *task_queue
);
/**
 * Perform studio build service through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_build_service_run(UmiStudioBuildService *service,
                                       UmiBuildPhase phase,
                                       UmiBuildResult *out_result);
/**
 * Provide the studio build service prepare default graph operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_build_service_prepare_default_graph(
    UmiStudioBuildService *service,
    int include_run);
/**
 * Provide the studio build service execute next operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_execute_next(
    UmiStudioBuildService *service,
    UmiBuildResult *out_result);
/**
 * Provide the studio build service execute all operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_execute_all(
    UmiStudioBuildService *service,
    size_t maximum_nodes,
    size_t *out_executed_count);
/**
 * Provide the studio build service cancel operation used by this module and its client
 * applications.
 */
void umi_studio_build_service_cancel(UmiStudioBuildService *service);
/**
 * Provide the studio build service retry operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_retry(UmiStudioBuildService *service,
                                         const char *node_id);
/**
 * Provide the studio build service invalidate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_invalidate(UmiStudioBuildService *service,
                                              const char *node_id,
                                              uint64_t input_revision);
/**
 * Provide the studio build service graph operation used by this module and its client
 * applications.
 */
UmiBuildGraph *umi_studio_build_service_graph(UmiStudioBuildService *service);
/**
 * Provide the studio build service artifacts operation used by this module and its client
 * applications.
 */
UmiBuildArtifactIndex *umi_studio_build_service_artifacts(
    UmiStudioBuildService *service);
/**
 * Provide the studio build service record artifact operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_build_service_record_artifact(
    UmiStudioBuildService *service,
    const UmiBuildArtifactSnapshot *artifact);
/**
 * Provide the studio build service snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_service_snapshot(
    const UmiStudioBuildService *service,
    UmiStudioBuildSnapshot *out_snapshot
);
/**
 * Provide the studio build service history operation used by this module and its client
 * applications.
 */
UmiBuildHistory *umi_studio_build_service_history(
    UmiStudioBuildService *service
);
/**
 * Provide the studio build service profile operation used by this module and its client
 * applications.
 */
const UmiBuildProfile *umi_studio_build_service_profile(
    const UmiStudioBuildService *service
);
/**
 * Provide the studio build service workspace operation used by this module and its client
 * applications.
 */
UmiBuildWorkspace *umi_studio_build_service_workspace(
    UmiStudioBuildService *service
);

#ifdef __cplusplus
}
#endif

#endif
