/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_service_admission.c
 *
 * PURPOSE:
 *   Verify direct Studio build entry points preserve active session ownership and cancellation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Private symbol aliases let this fixture inspect the real service owner without
 * adding a test-only API or replacing the linked product service. */
#define umi_studio_build_service_create Admission_umi_studio_build_service_create
#define umi_studio_build_service_destroy Admission_umi_studio_build_service_destroy
#define umi_studio_build_service_set_profile Admission_umi_studio_build_service_set_profile
#define umi_studio_build_service_bind_task_queue Admission_umi_studio_build_service_bind_task_queue
#define umi_studio_build_service_run Admission_umi_studio_build_service_run
#define umi_studio_build_service_prepare_default_graph Admission_umi_studio_build_service_prepare_default_graph
#define umi_studio_build_service_execute_next Admission_umi_studio_build_service_execute_next
#define umi_studio_build_service_execute_all Admission_umi_studio_build_service_execute_all
#define umi_studio_build_service_cancel Admission_umi_studio_build_service_cancel
#define umi_studio_build_service_retry Admission_umi_studio_build_service_retry
#define umi_studio_build_service_invalidate Admission_umi_studio_build_service_invalidate
#define umi_studio_build_service_graph Admission_umi_studio_build_service_graph
#define umi_studio_build_service_artifacts Admission_umi_studio_build_service_artifacts
#define umi_studio_build_service_record_artifact Admission_umi_studio_build_service_record_artifact
#define umi_studio_build_service_snapshot Admission_umi_studio_build_service_snapshot
#define umi_studio_build_service_history Admission_umi_studio_build_service_history
#define umi_studio_build_service_profile Admission_umi_studio_build_service_profile
#define umi_studio_build_service_workspace Admission_umi_studio_build_service_workspace
#define UmiStudioBuildSubmit Admission_UmiStudioBuildSubmit
#define UmiStudioBuildRunCurrent Admission_UmiStudioBuildRunCurrent
#define UmiStudioBuildRestartReview Admission_UmiStudioBuildRestartReview
#define UmiStudioBuildRestartRequest Admission_UmiStudioBuildRestartRequest
#define UmiStudioBuildRestartAdvance Admission_UmiStudioBuildRestartAdvance
#define UmiStudioBuildRestartRead Admission_UmiStudioBuildRestartRead
#define UmiStudioBuildCollect Admission_UmiStudioBuildCollect
#define UmiStudioBuildBusy Admission_UmiStudioBuildBusy
#define UmiStudioBuildProgress Admission_UmiStudioBuildProgress
#define UmiStudioBuildReadOutput Admission_UmiStudioBuildReadOutput
#define UmiStudioBuildArmLog Admission_UmiStudioBuildArmLog
#define UmiStudioBuildReadLog Admission_UmiStudioBuildReadLog
#define UmiStudioBuildOpenJobHistory Admission_UmiStudioBuildOpenJobHistory
#define UmiStudioBuildReadJobHistory Admission_UmiStudioBuildReadJobHistory
#define UmiStudioBuildCaptureJobs Admission_UmiStudioBuildCaptureJobs
#define UmiStudioBuildPruneFinishedJobs Admission_UmiStudioBuildPruneFinishedJobs
#define UmiStudioBuildReadDiagnostics Admission_UmiStudioBuildReadDiagnostics
#define UmiStudioBuildReadDiagnosticPage Admission_UmiStudioBuildReadDiagnosticPage

#include "umicom/studio/build.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); \
} } while (0)
static const char *state;
static int session_marker;
static UmiBuildProjectSession *controlled_session;
static unsigned execution_calls, cancel_calls;

static UmiStatus ObserveSession(UmiBuildProjectSession *session,
    UmiBuildProjectSessionSnapshot *out)
{
    CHECK(session == controlled_session);
    if (strcmp(state, "snapshot-error") == 0) return UMI_STATUS_IO_ERROR;
    memset(out, 0, sizeof(*out));
    out->active = strcmp(state, "active") == 0;
    out->completed_phase_count = strcmp(state, "uncollected") == 0 ? 1U : 0U;
    return UMI_STATUS_OK;
}
static UmiStatus ObserveRestart(UmiBuildProjectSession *session,
    UmiBuildRunRestartSnapshot *out)
{
    CHECK(session == controlled_session);
    if (strcmp(state, "restart-error") == 0) return UMI_STATUS_IO_ERROR;
    memset(out, 0, sizeof(*out));
    out->pending = strcmp(state, "restart") == 0;
    return UMI_STATUS_OK;
}
static void CancelSession(UmiBuildProjectSession *session)
{
    CHECK(session == controlled_session);
    ++cancel_calls;
}
static UmiStatus ExecutePhase(UmiBuildEngine *engine, UmiBuildPhase phase, UmiBuildResult *out)
{
    CHECK(engine != NULL);
    ++execution_calls;
    umi_build_result_init(out, 0U, phase, "controlled-execution");
    umi_build_result_finish(out, UMI_STATUS_NOT_IMPLEMENTED, -1, 0U);
    return UMI_STATUS_NOT_IMPLEMENTED;
}
static UmiStatus ExecuteNext(UmiBuildEngine *engine, UmiBuildResult *out)
{
    return ExecutePhase(engine, UMI_BUILD_PHASE_BUILD, out);
}
static UmiStatus ExecuteAll(UmiBuildEngine *engine, size_t maximum, size_t *count)
{
    CHECK(engine != NULL && maximum == 4U);
    ++execution_calls;
    if (count != NULL) *count = 0U;
    return UMI_STATUS_NOT_IMPLEMENTED;
}

/* Only session observations and execution are controlled. The profile, graph,
 * history, workspace, cancellation token and service lifecycle remain real.
 * Even a missing busy guard cannot launch a compiler through this fixture. */
#define umi_build_project_session_snapshot ObserveSession
#define UmiBuildRunRestartRead ObserveRestart
#define umi_build_project_session_cancel CancelSession
#define umi_build_engine_execute_phase ExecutePhase
#define umi_build_engine_execute_next ExecuteNext
#define umi_build_engine_execute_all ExecuteAll
#include "../src/app/build.c"
#undef umi_build_project_session_snapshot
#undef UmiBuildRunRestartRead
#undef umi_build_project_session_cancel
#undef umi_build_engine_execute_phase
#undef umi_build_engine_execute_next
#undef umi_build_engine_execute_all

static UmiStatus Invoke(UmiStudioBuildService *service, const char *operation,
    const UmiBuildProfile *profile, UmiBuildResult *result, size_t *count)
{
    if (strcmp(operation, "prepare") == 0)
        return umi_studio_build_service_prepare_default_graph(service, 1);
    if (strcmp(operation, "run") == 0)
        return umi_studio_build_service_run(service, UMI_BUILD_PHASE_BUILD, result);
    if (strcmp(operation, "next") == 0)
        return umi_studio_build_service_execute_next(service, result);
    if (strcmp(operation, "all") == 0)
        return umi_studio_build_service_execute_all(service, 4U, count);
    if (strcmp(operation, "retry") == 0)
        return umi_studio_build_service_retry(service, "work");
    if (strcmp(operation, "invalidate") == 0)
        return umi_studio_build_service_invalidate(service, "work", 7U);
    if (strcmp(operation, "profile") == 0)
        return umi_studio_build_service_set_profile(service, profile);
    umi_studio_build_service_cancel(service);
    return UMI_STATUS_OK;
}
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *operation = argv[1];
    state = argv[2];
    const char *operations[] = {"prepare", "run", "next", "all", "retry", "invalidate", "profile", "cancel"};
    const char *states[] = {"active", "uncollected", "restart", "snapshot-error", "restart-error", "idle"};
    int recognised_operation = 0, recognised_state = 0;
    for (size_t i = 0U; i < sizeof(operations) / sizeof(operations[0]); ++i)
        if (strcmp(operation, operations[i]) == 0) recognised_operation = 1;
    for (size_t i = 0U; i < sizeof(states) / sizeof(states[0]); ++i)
        if (strcmp(state, states[i]) == 0) recognised_state = 1;
    if (!recognised_operation || !recognised_state) return 2;
    UmiClock clock = umi_clock_system();
    UmiStudioBuildService *service = NULL;
    CHECK(umi_studio_build_service_create(".", &clock, &service) == UMI_STATUS_OK);
    UmiBuildResult *result = NULL, *saved = NULL, *latest = NULL;
    CHECK(umi_build_result_create(&result) == UMI_STATUS_OK);
    CHECK(umi_build_result_create(&saved) == UMI_STATUS_OK);
    CHECK(umi_build_result_create(&latest) == UMI_STATUS_OK);
    UmiStudioBuildService *service_before = malloc(sizeof(*service_before));
    UmiStudioBuildSnapshot *before = calloc(1U, sizeof(*before)), *after = calloc(1U, sizeof(*after));
    CHECK(service_before != NULL && before != NULL && after != NULL);
    UmiBuildProfile profile = service->profile;
    strcpy(profile.profile_id, "candidate-profile");
    if (strcmp(operation, "prepare") != 0) {
        UmiBuildGraphNodeSnapshot node;
        umi_build_graph_node_init(&node, "work", "Work", UMI_BUILD_PHASE_BUILD);
        node.maximum_attempts = 3U;
        CHECK(umi_build_graph_add_node(umi_studio_build_service_graph(service), &node) == UMI_STATUS_OK);
        CHECK(umi_build_graph_start(umi_studio_build_service_graph(service), "work") == UMI_STATUS_OK);
        CHECK(umi_build_graph_fail(umi_studio_build_service_graph(service), "work", UMI_STATUS_IO_ERROR, 9) == UMI_STATUS_OK);
    }
    umi_build_result_init(result, 70U, UMI_BUILD_PHASE_TEST, "retained");
    umi_build_result_finish(result, UMI_STATUS_IO_ERROR, 6, 12U);
    *saved = *result;
    CHECK(umi_build_history_append(service->history, result) == UMI_STATUS_OK);
    service->last_result = *result;
    service->has_last_result = 1;
    umi_build_engine_request_cancel(service->engine);
    uint64_t generation = umi_cancellation_token_generation(service->cancellation);
    controlled_session = (UmiBuildProjectSession *)(void *)&session_marker;
    service->project_session = controlled_session;
    CHECK(umi_studio_build_service_snapshot(service, before) == UMI_STATUS_OK);
    memcpy(service_before, service, sizeof(*service));
    size_t count = 99U;
    UmiStatus status = Invoke(service, operation, &profile, result, &count);
    int busy = strcmp(state, "idle") != 0;
    if (strcmp(operation, "cancel") == 0) {
        CHECK(status == UMI_STATUS_OK && cancel_calls == 1U && execution_calls == 0U);
        CHECK(umi_cancellation_token_is_requested(service->cancellation));
        CHECK(memcmp(result, saved, sizeof(*result)) == 0);
    } else if (busy) {
        CHECK(status == UMI_STATUS_BUSY && execution_calls == 0U && cancel_calls == 0U);
        CHECK(count == 99U && memcmp(result, saved, sizeof(*result)) == 0);
        CHECK(memcmp(service_before, service, sizeof(*service)) == 0);
        CHECK(umi_studio_build_service_snapshot(service, after) == UMI_STATUS_OK);
        CHECK(memcmp(before, after, sizeof(*before)) == 0);
        CHECK(umi_cancellation_token_generation(service->cancellation) == generation);
        CHECK(umi_build_history_latest(service->history, latest) == UMI_STATUS_OK);
        CHECK(memcmp(latest, saved, sizeof(*saved)) == 0);
    } else {
        int executes = strcmp(operation, "run") == 0 || strcmp(operation, "next") == 0 || strcmp(operation, "all") == 0;
        CHECK(status == (executes ? UMI_STATUS_NOT_IMPLEMENTED : UMI_STATUS_OK));
        CHECK(execution_calls == (executes ? 1U : 0U) && cancel_calls == 0U);
        if (strcmp(operation, "retry") == 0)
            CHECK(!umi_cancellation_token_is_requested(service->cancellation));
    }
    /* The marker is observed only by the controlled ports, never destroyed as
     * a real session. Clear that fixture-only borrow before normal destruction. */
    service->project_session = NULL;
    umi_studio_build_service_destroy(service);
    umi_clock_dispose(&clock);
    umi_build_result_destroy(result); umi_build_result_destroy(saved); umi_build_result_destroy(latest);
    free(service_before); free(before); free(after);
    return 0;
}
