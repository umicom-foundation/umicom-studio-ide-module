/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_pipeline_centre_admission.c
 *
 * PURPOSE:
 *   Verify Studio delegation admission and complete composite snapshot publication with controlled Framework observation boundaries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Keep fixture setup and checks active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "umicom/studio/developer_pipeline_centre.h"

static unsigned delegate_calls, runtime_reads, delivery_reads;
static const char *last_delegate;
static UmiStatus delegate_status = UMI_STATUS_OK;
static int snapshot_fault, allocation_failure;
static UmiDeveloperRuntime *expected_runtime;
static UmiDeveloperRuntimeSnapshot runtime_observation;
static UmiBuildDeliveryPipelineSnapshot delivery_observation;

#define STUB(name, parameters, output) \
    static UmiStatus name parameters { \
        (void)request; assert(runtime == expected_runtime); ++delegate_calls; last_delegate = #name; \
        if (delegate_status == UMI_STATUS_OK && output != NULL) \
            memset(output, 0x31, sizeof(*output)); \
        return delegate_status; \
    }

STUB(PrepareCmake, (UmiDeveloperRuntime *runtime, const UmiDeveloperCMakePlanRequest *request,
    UmiDeveloperCMakePlanSnapshot *out), out)
STUB(SubmitTask, (UmiDeveloperRuntime *runtime, const UmiDeveloperTaskPlanRequest *request,
    UmiDeveloperTaskPlanSnapshot *out), out)
STUB(SubmitLaunch, (UmiDeveloperRuntime *runtime, const UmiDeveloperLaunchPlanRequest *request,
    UmiDeveloperLaunchPlanSnapshot *out), out)
STUB(SubmitWorkflow, (UmiDeveloperRuntime *runtime, const UmiDeveloperWorkflowRequest *request,
    UmiDeveloperWorkflowSnapshot *out), out)
STUB(SubmitProject, (UmiDeveloperRuntime *runtime, const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out), out)
static UmiStatus ExecuteBatch(UmiDeveloperRuntime *runtime, const UmiDeveloperExecutor *executor,
    const UmiDeveloperBatchExecutionRequest *request, UmiDeveloperBatchExecutionSnapshot *out)
{
    (void)executor; (void)request;
    assert(runtime == expected_runtime); ++delegate_calls; last_delegate = "ExecuteBatch";
    if (delegate_status == UMI_STATUS_OK && out != NULL) memset(out, 0x31, sizeof(*out));
    return delegate_status;
}
STUB(ExecuteBatchProcess, (UmiDeveloperRuntime *runtime,
    const UmiDeveloperBatchExecutionRequest *request, UmiDeveloperBatchExecutionSnapshot *out), out)
#undef STUB

static UmiStatus ExecuteNext(UmiDeveloperRuntime *runtime, const UmiDeveloperExecutor *executor,
    UmiDeveloperOperationSnapshot *out, UmiDeveloperExecutionResult *result)
{
    (void)executor;
    assert(runtime == expected_runtime);
    ++delegate_calls; last_delegate = "ExecuteNext";
    if (delegate_status == UMI_STATUS_OK) {
        if (out != NULL) memset(out, 0x31, sizeof(*out));
        if (result != NULL) memset(result, 0x31, sizeof(*result));
    }
    return delegate_status;
}
static UmiStatus RuntimeSnapshot(UmiDeveloperRuntime *runtime, UmiDeveloperRuntimeSnapshot *out)
{
    assert(runtime == expected_runtime);
    ++runtime_reads;
    *out = runtime_observation;
    return snapshot_fault == 1 ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
}
static UmiStatus DeliverySnapshot(const UmiBuildDeliveryPipeline *delivery, UmiBuildDeliveryPipelineSnapshot *out)
{
    (void)delivery;
    ++delivery_reads;
    *out = delivery_observation;
    return snapshot_fault == 2 ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
}
static void *SnapshotAllocate(size_t count, size_t size)
{ return allocation_failure ? NULL : calloc(count, size); }

/* Exercise the actual composition with controlled child boundaries. These
 * stubs never invoke an executor or claim to qualify Framework model behavior. */
#define umi_developer_runtime_submit_cmake_plan PrepareCmake
#define umi_developer_runtime_execute_next ExecuteNext
#define umi_developer_task_plan_submit SubmitTask
#define umi_developer_launch_plan_submit SubmitLaunch
#define umi_developer_workflow_submit SubmitWorkflow
#define umi_developer_project_workflow_submit SubmitProject
#define umi_developer_batch_execute ExecuteBatch
#define umi_developer_batch_execute_process ExecuteBatchProcess
#define umi_developer_runtime_snapshot RuntimeSnapshot
#define umi_build_delivery_pipeline_snapshot DeliverySnapshot
#define calloc SnapshotAllocate
#include "../src/app/developer_pipeline_centre.c"
#undef calloc
#undef umi_build_delivery_pipeline_snapshot
#undef umi_developer_runtime_snapshot
#undef umi_developer_batch_execute_process
#undef umi_developer_batch_execute
#undef umi_developer_project_workflow_submit
#undef umi_developer_workflow_submit
#undef umi_developer_launch_plan_submit
#undef umi_developer_task_plan_submit
#undef umi_developer_runtime_execute_next
#undef umi_developer_runtime_submit_cmake_plan

typedef struct Outputs {
    UmiDeveloperCMakePlanSnapshot cmake;
    UmiDeveloperTaskPlanSnapshot task;
    UmiDeveloperLaunchPlanSnapshot launch;
    UmiDeveloperWorkflowSnapshot workflow;
    UmiDeveloperProjectWorkflowSnapshot project;
    UmiDeveloperOperationSnapshot operation;
    UmiDeveloperExecutionResult result;
    UmiDeveloperBatchExecutionSnapshot batch;
} Outputs;

static UmiStatus Call(UmiStudioDeveloperPipelineCentre *centre, const char *route, Outputs *out)
{
    UmiDeveloperCMakePlanRequest cmake = {0};
    UmiDeveloperTaskPlanRequest task = {0};
    UmiDeveloperLaunchPlanRequest launch = {0};
    UmiDeveloperWorkflowRequest workflow = {0};
    UmiDeveloperProjectWorkflowRequest project = {0};
    UmiDeveloperBatchExecutionRequest batch = {0};
    UmiDeveloperExecutor executor = {0};
    if (!strcmp(route, "cmake"))
        return umi_studio_developer_pipeline_centre_prepare_cmake(centre, &cmake, &out->cmake);
    if (!strcmp(route, "task"))
        return umi_studio_developer_pipeline_centre_submit_task(centre, &task, &out->task);
    if (!strcmp(route, "launch"))
        return umi_studio_developer_pipeline_centre_submit_launch(centre, &launch, &out->launch);
    if (!strcmp(route, "workflow"))
        return umi_studio_developer_pipeline_centre_submit_workflow(centre, &workflow, &out->workflow);
    if (!strcmp(route, "project"))
        return umi_studio_developer_pipeline_centre_prepare_project_workflow(centre, &project, &out->project);
    if (!strcmp(route, "next"))
        return umi_studio_developer_pipeline_centre_execute_next(centre, &executor, &out->operation, &out->result);
    if (!strcmp(route, "next-process"))
        return umi_studio_developer_pipeline_centre_execute_next_process(centre, &out->operation, &out->result);
    if (!strcmp(route, "batch"))
        return umi_studio_developer_pipeline_centre_execute_batch(centre, &executor, &batch, &out->batch);
    assert(!strcmp(route, "batch-process"));
    return umi_studio_developer_pipeline_centre_execute_batch_process(centre, &batch, &out->batch);
}

static void Mutation(UmiStudioDeveloperPipelineCentre *centre, const char *route, const char *mode)
{
    Outputs *out = malloc(sizeof(*out)), *before = malloc(sizeof(*before));
    assert(out != NULL && before != NULL);
    memset(out, 0x5a, sizeof(*out)); memcpy(before, out, sizeof(*out));
    centre->revision = !strcmp(mode, "exhausted") ? UINT64_MAX : UINT64_MAX - 1U;
    uint64_t previous = centre->revision;
    delegate_status = !strcmp(mode, "nested-error") ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
    UmiStatus status = Call(centre, route, out);
    if (!strcmp(mode, "exhausted")) {
        assert(status == UMI_STATUS_INVALID_STATE && delegate_calls == 0U);
        assert(centre->revision == previous);
        assert(memcmp(out, before, sizeof(*out)) == 0);
    } else {
        assert(delegate_calls == 1U && status == delegate_status);
        const char *expected = !strcmp(route, "cmake") ? "PrepareCmake" :
            !strcmp(route, "task") ? "SubmitTask" : !strcmp(route, "launch") ? "SubmitLaunch" :
            !strcmp(route, "workflow") ? "SubmitWorkflow" : !strcmp(route, "project") ? "SubmitProject" :
            !strcmp(route, "batch") ? "ExecuteBatch" : !strcmp(route, "batch-process") ?
            "ExecuteBatchProcess" : "ExecuteNext";
        assert(!strcmp(last_delegate, expected));
        if (status == UMI_STATUS_OK) {
            assert(centre->revision == UINT64_MAX);
            assert(memcmp(out, before, sizeof(*out)) != 0);
            if (!strcmp(route, "project")) {
                assert(centre->has_project_workflow == 1);
                assert(memcmp(&centre->last_project_workflow, &out->project, sizeof(out->project)) == 0);
            }
        } else {
            assert(centre->revision == previous && centre->has_project_workflow == 0);
            assert(memcmp(out, before, sizeof(*out)) == 0);
        }
    }
    free(out); free(before);
}

static void Snapshot(UmiStudioDeveloperPipelineCentre *centre, const char *mode)
{
    memset(&runtime_observation, 0, sizeof(runtime_observation));
    memset(&delivery_observation, 0, sizeof(delivery_observation));
    runtime_observation.revision = 2U;
    delivery_observation.revision = 3U;
    strcpy(runtime_observation.context.project_id, "observed-project");
    runtime_observation.pipeline.operation_count = 2U;
    runtime_observation.journal.entry_count = 3U;
    delivery_observation.tasks.planned_count = 4U;
    delivery_observation.artifacts.artifact_count = 5U;
    delivery_observation.deployment.plan.step_count = 6U;
    centre->has_project_workflow = 1;
    strcpy(centre->last_project_workflow.workflow.workflow_id, "accepted-workflow");
    int success = !strcmp(mode, "success") || !strcmp(mode, "boundary");
    UmiStatus expected = UMI_STATUS_CAPACITY_EXCEEDED;
    if (!strcmp(mode, "runtime-error")) { snapshot_fault = 1; expected = UMI_STATUS_IO_ERROR; }
    else if (!strcmp(mode, "delivery-error")) { snapshot_fault = 2; expected = UMI_STATUS_IO_ERROR; }
    else if (!strcmp(mode, "allocation")) { allocation_failure = 1; expected = UMI_STATUS_OUT_OF_MEMORY; }
    else if (!strcmp(mode, "revision-first")) centre->revision = UINT64_MAX;
    else if (!strcmp(mode, "revision-second")) centre->revision = UINT64_MAX - 4U;
    else if (!strcmp(mode, "count-first")) runtime_observation.pipeline.operation_count = SIZE_MAX;
    else if (!strcmp(mode, "count-last")) delivery_observation.deployment.plan.step_count = SIZE_MAX;
    else if (!strcmp(mode, "boundary")) centre->revision = UINT64_MAX - 5U;
    else if (!strcmp(mode, "null-output") || !strcmp(mode, "null-centre")) expected = UMI_STATUS_INVALID_ARGUMENT;
    else assert(success);
    UmiStudioDeveloperPipelineCentreSnapshot *out = malloc(sizeof(*out)), *before = malloc(sizeof(*before));
    assert(out != NULL && before != NULL);
    memset(out, 0x5a, sizeof(*out)); memcpy(before, out, sizeof(*out));
    UmiStatus status = umi_studio_developer_pipeline_centre_snapshot(
        !strcmp(mode, "null-centre") ? NULL : centre,
        !strcmp(mode, "null-output") ? NULL : out);
    if (success) {
        assert(status == UMI_STATUS_OK && out->available == 1);
        assert(out->revision == (!strcmp(mode, "boundary") ? UINT64_MAX : 6U));
        assert(out->item_count == 20U && out->has_project_workflow == 1);
        assert(!strcmp(out->context.project_id, "observed-project"));
        assert(!strcmp(out->last_project_workflow.workflow.workflow_id, "accepted-workflow"));
        assert(out->delivery_command_count == umi_studio_build_delivery_command_contribution_count());
        assert(out->delivery_view_count == umi_studio_build_delivery_view_contribution_count());
    } else {
        assert(status == expected);
        assert(memcmp(out, before, sizeof(*out)) == 0);
    }
    if (allocation_failure || expected == UMI_STATUS_INVALID_ARGUMENT)
        assert(runtime_reads == 0U && delivery_reads == 0U);
    else if (snapshot_fault == 1)
        assert(runtime_reads == 1U && delivery_reads == 0U);
    else assert(runtime_reads == 1U && delivery_reads == 1U);
    allocation_failure = 0;
    free(out); free(before);
}

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    UmiStudioDeveloperPipelineCentre *centre = NULL;
    assert(umi_developer_runtime_create_owned(&expected_runtime) == UMI_STATUS_OK);
    assert(umi_studio_developer_pipeline_centre_create(expected_runtime, &centre) == UMI_STATUS_OK);
    if (!strcmp(argv[1], "snapshot")) Snapshot(centre, argv[2]);
    else Mutation(centre, argv[1], argv[2]);
    umi_studio_developer_pipeline_centre_destroy(centre);
    umi_developer_runtime_destroy(expected_runtime);
    return 0;
}
