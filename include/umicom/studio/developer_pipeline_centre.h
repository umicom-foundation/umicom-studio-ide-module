/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_pipeline_centre.h
 *
 * PURPOSE:
 *   Expose the reusable Framework developer pipeline as a Studio workbench
 *   centre with CMake planning and operation execution entry points.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This centre does not duplicate Framework operation state.  It borrows the
 * UmiDeveloperRuntime owned by the Studio developer workbench and provides the
 * small product-facing surface that command handlers, panels and future GTK4
 * views need.
 */
#ifndef UMICOM_STUDIO_DEVELOPER_PIPELINE_CENTRE_H
#define UMICOM_STUDIO_DEVELOPER_PIPELINE_CENTRE_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/build/delivery_platform.h"
#include "umicom/developer/developer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_BUILD_DELIVERY_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio developer pipeline centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperPipelineCentre UmiStudioDeveloperPipelineCentre;

/**
 * Represent the studio build delivery command contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioBuildDeliveryCommandContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *group_id;
    int order;
} UmiStudioBuildDeliveryCommandContribution;

/**
 * Represent the studio build delivery view contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioBuildDeliveryViewContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_view_id;
    const char *default_region;
    int order;
    int closable;
    int movable;
} UmiStudioBuildDeliveryViewContribution;

/**
 * Represent the studio developer pipeline centre snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioDeveloperPipelineCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char title[256];
    char summary[512];
    UmiDeveloperContextSnapshot context;
    UmiDeveloperPipelineSnapshot pipeline;
    UmiDeveloperJournalSnapshot journal;
    UmiBuildDeliveryPipelineSnapshot delivery;
    uint64_t revision;
    size_t item_count;
    size_t delivery_command_count;
    size_t delivery_view_count;
    int available;
    UmiDeveloperProjectWorkflowSnapshot last_project_workflow;
    int has_project_workflow;
} UmiStudioDeveloperPipelineCentreSnapshot;

/**
 * Initialise studio developer pipeline centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_developer_pipeline_centre_create(
    UmiDeveloperRuntime *runtime,
    UmiStudioDeveloperPipelineCentre **out_centre);
/**
 * Release or reset state held by studio developer pipeline centre so the same storage can
 * be reused safely.
 */
void umi_studio_developer_pipeline_centre_destroy(
    UmiStudioDeveloperPipelineCentre *centre);
/**
 * Provide the studio developer pipeline centre snapshot operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_snapshot(
    UmiStudioDeveloperPipelineCentre *centre,
    UmiStudioDeveloperPipelineCentreSnapshot *out_snapshot);
/**
 * Provide the studio developer pipeline centre prepare cmake operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_prepare_cmake(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperCMakePlanRequest *request,
    UmiDeveloperCMakePlanSnapshot *out_plan);
/**
 * Provide the studio developer pipeline centre execute next operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_execute_next(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperExecutor *executor,
    UmiDeveloperOperationSnapshot *out_operation,
    UmiDeveloperExecutionResult *out_result);
/**
 * Provide the studio developer pipeline centre execute next process operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_execute_next_process(
    UmiStudioDeveloperPipelineCentre *centre,
    UmiDeveloperOperationSnapshot *out_operation,
    UmiDeveloperExecutionResult *out_result);
/**
 * Provide the studio developer pipeline centre submit task operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_submit_task(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperTaskPlanRequest *request,
    UmiDeveloperTaskPlanSnapshot *out_plan);
/**
 * Provide the studio developer pipeline centre submit launch operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_submit_launch(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperLaunchPlanRequest *request,
    UmiDeveloperLaunchPlanSnapshot *out_plan);
/**
 * Provide the studio developer pipeline centre submit workflow operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_submit_workflow(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperWorkflowRequest *request,
    UmiDeveloperWorkflowSnapshot *out_workflow);
/**
 * Provide the studio developer pipeline centre prepare project workflow operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_prepare_project_workflow(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out_workflow);
/**
 * Provide the studio developer pipeline centre execute batch operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_execute_batch(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperExecutor *executor,
    const UmiDeveloperBatchExecutionRequest *request,
    UmiDeveloperBatchExecutionSnapshot *out_snapshot);
/**
 * Provide the studio developer pipeline centre execute batch process operation used by
 * this module and its client applications.
 */
UmiStatus umi_studio_developer_pipeline_centre_execute_batch_process(
    UmiStudioDeveloperPipelineCentre *centre,
    const UmiDeveloperBatchExecutionRequest *request,
    UmiDeveloperBatchExecutionSnapshot *out_snapshot);
/**
 * Provide the studio developer pipeline centre runtime operation used by this module and
 * its client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_pipeline_centre_runtime(
    UmiStudioDeveloperPipelineCentre *centre);
/**
 * Provide the studio developer pipeline centre delivery tasks operation used by this
 * module and its client applications.
 */
UmiBuildTaskRegistry *umi_studio_developer_pipeline_centre_delivery_tasks(
    UmiStudioDeveloperPipelineCentre *centre);
/**
 * Provide the studio developer pipeline centre deployment targets operation used by this
 * module and its client applications.
 */
UmiBuildDeploymentTargetRegistry *
umi_studio_developer_pipeline_centre_deployment_targets(
    UmiStudioDeveloperPipelineCentre *centre);
/**
 * Provide the studio developer pipeline centre delivery operation used by this module and
 * its client applications.
 */
UmiBuildDeliveryPipeline *umi_studio_developer_pipeline_centre_delivery(
    UmiStudioDeveloperPipelineCentre *centre);

/**
 * Return the number of records represented by studio build delivery command contribution
 * without changing their state.
 */
size_t umi_studio_build_delivery_command_contribution_count(void);
/**
 * Find studio build delivery command contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioBuildDeliveryCommandContribution *
umi_studio_build_delivery_command_contribution_at(size_t position);
/**
 * Find studio build delivery command contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioBuildDeliveryCommandContribution *
umi_studio_build_delivery_command_contribution_find(
    const char *framework_command_id);
/**
 * Return the number of records represented by studio build delivery view contribution
 * without changing their state.
 */
size_t umi_studio_build_delivery_view_contribution_count(void);
/**
 * Find studio build delivery view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioBuildDeliveryViewContribution *
umi_studio_build_delivery_view_contribution_at(size_t position);
/**
 * Find studio build delivery view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioBuildDeliveryViewContribution *
umi_studio_build_delivery_view_contribution_find(
    const char *framework_view_id);

#ifdef __cplusplus
}
#endif
#endif /* UMICOM_STUDIO_DEVELOPER_PIPELINE_CENTRE_H */
