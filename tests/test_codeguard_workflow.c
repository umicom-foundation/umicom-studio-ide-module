/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_codeguard_workflow.c
 *
 * PURPOSE:
 *   Exercise shared quality reviews through real isolated Studio service and workspace owners.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/codeguard_workflow.h"
#include "umicom/studio/workspace.h"
#include "studio_test_state.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%d: %s\n",__LINE__,#x); exit(1); } } while (0)
/* Link-time faults affect only the selected review boundary; real Studio
 * construction, queues, scanner and graph operations remain in the test. */
#ifdef UMICOM_STUDIO_CODEGUARD_FAULTS
enum { REVIEW_FAULT_NONE, REVIEW_FAULT_CREATE, REVIEW_FAULT_POLL,
       REVIEW_FAULT_CANCEL, REVIEW_FAULT_DESTROY };
static int review_fault;
static unsigned fault_count;
static int FailReview(int site)
{
    if (review_fault != site) return 0;
    review_fault = REVIEW_FAULT_NONE;
    ++fault_count;
    return 1;
}
UmiStatus __real_UmiCodeGuardReviewCreate(const char *,UmiCodeGuardReview **);
UmiStatus __wrap_UmiCodeGuardReviewCreate(const char *root,UmiCodeGuardReview **out)
{
    if(FailReview(REVIEW_FAULT_CREATE)) { *out=NULL; return UMI_STATUS_OUT_OF_MEMORY; }
    return __real_UmiCodeGuardReviewCreate(root,out);
}
UmiStatus __real_UmiCodeGuardReviewPoll(UmiCodeGuardReview *,uint64_t,UmiCodeGuardReviewSnapshot *);
UmiStatus __wrap_UmiCodeGuardReviewPoll(UmiCodeGuardReview *review,uint64_t context,UmiCodeGuardReviewSnapshot *out)
{
    if(FailReview(REVIEW_FAULT_POLL)) return UMI_STATUS_IO_ERROR;
    return __real_UmiCodeGuardReviewPoll(review,context,out);
}
UmiStatus __real_UmiCodeGuardReviewCancel(UmiCodeGuardReview *);
UmiStatus __wrap_UmiCodeGuardReviewCancel(UmiCodeGuardReview *review)
{
    if(FailReview(REVIEW_FAULT_CANCEL)) return UMI_STATUS_IO_ERROR;
    return __real_UmiCodeGuardReviewCancel(review);
}
UmiStatus __real_UmiCodeGuardReviewDestroy(UmiCodeGuardReview **);
UmiStatus __wrap_UmiCodeGuardReviewDestroy(UmiCodeGuardReview **review)
{
    if(FailReview(REVIEW_FAULT_DESTROY)) return UMI_STATUS_IO_ERROR;
    return __real_UmiCodeGuardReviewDestroy(review);
}
#endif
static UmiMutex *gate_mutex;
static UmiCondition *gate_condition;
static int gate_entered, gate_release;
static UmiStatus BlockWorker(UmiTaskContext *context,void *data)
{
    (void)context; (void)data;
    UmiStatus status=umi_mutex_lock(gate_mutex);
    if(status!=UMI_STATUS_OK)return status;
    gate_entered=1;
    (void)umi_condition_broadcast(gate_condition);
    while(!gate_release && status==UMI_STATUS_OK)
        status=umi_condition_wait_for(gate_condition,gate_mutex,5000U);
    (void)umi_mutex_unlock(gate_mutex);
    return status;
}
static UmiTask *OccupyQueue(UmiTaskQueue *queue)
{
    CHECK(umi_mutex_create(&gate_mutex)==UMI_STATUS_OK);
    CHECK(umi_condition_create(&gate_condition)==UMI_STATUS_OK);
    UmiTaskConfig config={.label="controlled blocker",.function=BlockWorker};
    UmiTask *task=NULL;
    CHECK(umi_task_create(&config,&task)==UMI_STATUS_OK);
    CHECK(umi_task_queue_submit(queue,task)==UMI_STATUS_OK);
    CHECK(umi_mutex_lock(gate_mutex)==UMI_STATUS_OK);
    while(!gate_entered)
        CHECK(umi_condition_wait_for(gate_condition,gate_mutex,5000U)==UMI_STATUS_OK);
    CHECK(umi_mutex_unlock(gate_mutex)==UMI_STATUS_OK);
    return task;
}
static void ReleaseQueue(UmiTaskQueue *queue,UmiTask *task)
{
    CHECK(umi_mutex_lock(gate_mutex)==UMI_STATUS_OK);
    gate_release=1;
    CHECK(umi_condition_broadcast(gate_condition)==UMI_STATUS_OK);
    CHECK(umi_mutex_unlock(gate_mutex)==UMI_STATUS_OK);
    CHECK(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
    CHECK(umi_task_result(task)==UMI_STATUS_OK);
    umi_task_destroy(task);
    umi_condition_destroy(gate_condition);
    umi_mutex_destroy(gate_mutex);
}

static UmiCodeGuardReviewSnapshot Poll(UmiStudioServices *services)
{
    UmiCodeGuardReviewSnapshot snapshot;
    CHECK(UmiStudioCodeGuardPoll(services,&snapshot)==UMI_STATUS_OK);
    return snapshot;
}
static UmiCodeGuardReviewSnapshot WaitPoll(UmiStudioServices *services)
{
    CHECK(umi_task_queue_wait_idle(umi_studio_services_task_queue(services),10000U)==UMI_STATUS_OK);
    return Poll(services);
}
static int WrongThread(void *data)
{
    UmiStudioServices *services=data;
    uint64_t request=88U;
    UmiCodeGuardReviewSnapshot out,saved;
    memset(&out,0x5a,sizeof out);
    memcpy(&saved,&out,sizeof saved);
    CHECK(UmiStudioCodeGuardBegin(services,NULL,&request)==UMI_STATUS_INVALID_STATE);
    CHECK(request==88U);
    CHECK(UmiStudioCodeGuardPoll(services,&out)==UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(&out,&saved,sizeof out)==0);
    CHECK(UmiStudioCodeGuardSnapshot(services,&out)==UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(&out,&saved,sizeof out)==0);
    CHECK(UmiStudioCodeGuardCancel(services)==UMI_STATUS_INVALID_STATE);
    CHECK(UmiStudioCodeGuardResult(services)==NULL);
    return 0;
}
static UmiStatus WorkerRelease(UmiTaskContext *context,void *data)
{
    (void)context;
    UmiStudioServices *owner=data;
    UmiStudioServices *borrow=owner;
    CHECK(UmiStudioServicesStopBackground(owner)==UMI_STATUS_INVALID_STATE);
    CHECK(UmiStudioServicesRelease(&borrow)==UMI_STATUS_INVALID_STATE && borrow==owner);
    CHECK(WrongThread(owner)==0);
    return UMI_STATUS_OK;
}
int main(int argc,char **argv)
{
    CHECK(argc==2);
    const char *name=argv[1];
    const char *cases[]={"arguments","closed","idle","success","untrusted","empty","budget","cancel-terminal","close-terminal","reopen-terminal","switch-terminal","trust-terminal","graph-terminal","snapshot-stale","retry","queue-stopped","profile-copy","cwd","wrong-thread","wrong-first-thread","idle-cancel","shutdown","shutdown-terminal","worker-release","busy","queued-cancel"
#ifdef UMICOM_STUDIO_CODEGUARD_FAULTS
        ,"create-failure","poll-before-admission","cancel-shutdown-failure","release-failure"
#endif
    };
    size_t selected=0U;
    while(selected<sizeof cases/sizeof cases[0] && strcmp(name,cases[selected])) ++selected;
    if(selected==sizeof cases/sizeof cases[0]) return 2;
    StudioTestState state={0};
    CHECK(StudioTestStateBegin(&state));
    char *project=g_build_filename(state.directory,"project",NULL);
    char *other=g_build_filename(state.directory,"other",NULL);
    char *source=g_build_filename(project,"sample.c",NULL);
    CHECK(project!=NULL && other!=NULL && source!=NULL);
    CHECK(g_mkdir(project,0700)==0 && g_mkdir(other,0700)==0);
    CHECK(g_file_set_contents(source,"void sample(void) {\n    malloc(4);\n}\n",-1,NULL));

    /* Disable unrelated persistence/network automation and use one worker so
     * a controlled blocker deterministically keeps a review queued. */
    UmiSettings *settings=NULL;
    CHECK(umi_studio_settings_create(&settings)==UMI_STATUS_OK);
    CHECK(umi_settings_set_integer(settings,UMI_STUDIO_SETTING_BUILD_PARALLEL_JOBS,1)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AUTO_SAVE,0)==UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings,umi_studio_settings_default_path())==UMI_STATUS_OK);
    umi_settings_destroy(settings);
    UmiStudioServices *services=NULL;
    UmiStudioServicesOptions options={0};
    CHECK(umi_studio_services_create_with_options(NULL,NULL,&options,&services)==UMI_STATUS_OK);
    UmiTaskQueue *queue=umi_studio_services_task_queue(services);
    UmiCodeGuardProfile profile=umi_codeguard_profile_default();
    profile.scan_architecture=0;
    profile.scan_source_names=0;
    profile.scan_duplicates=0;
    profile.max_line_length=SIZE_MAX;
    uint64_t request=88U;
    if(strcmp(name,"arguments")==0) {
        UmiCodeGuardReviewSnapshot out,saved;
        memset(&out,0x5a,sizeof out);
        memcpy(&saved,&out,sizeof saved);
        CHECK(UmiStudioCodeGuardBegin(NULL,&profile,&request)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioCodeGuardBegin(services,&profile,NULL)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(request==88U);
        CHECK(UmiStudioCodeGuardPoll(NULL,&out)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioCodeGuardSnapshot(NULL,&out)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(memcmp(&out,&saved,sizeof out)==0);
        CHECK(UmiStudioCodeGuardPoll(services,NULL)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioCodeGuardSnapshot(services,NULL)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioCodeGuardCancel(NULL)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioCodeGuardResult(NULL)==NULL);
    } else if(strcmp(name,"closed")==0) {
        CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_INVALID_STATE);
        CHECK(request==88U);
        UmiCodeGuardReviewSnapshot snapshot=Poll(services);
        CHECK(!snapshot.pending && !snapshot.has_report && snapshot.status==UMI_STATUS_NOT_FOUND);
    } else {
        CHECK(UmiStudioServicesPrepareWorkspace(services,project,strcmp(name,"untrusted")!=0)==UMI_STATUS_OK);
        UmiWorkspaceGraphSnapshot opened;
        CHECK(umi_workspace_graph_snapshot(umi_studio_services_workspace(services),&opened)==UMI_STATUS_OK);
        CHECK(opened.open && opened.trusted==(strcmp(name,"untrusted")!=0));
        if(strcmp(name,"idle")==0) {
            UmiCodeGuardReviewSnapshot snapshot=Poll(services);
            CHECK(!snapshot.pending && !snapshot.has_report && snapshot.state==UMI_TASK_CREATED);
            CHECK(UmiStudioCodeGuardCancel(services)==UMI_STATUS_OK);
            CHECK(UmiStudioCodeGuardResult(services)==NULL);
        } else if(strcmp(name,"wrong-first-thread")==0) {
            UmiThread *thread=NULL;
            CHECK(umi_thread_start(WrongThread,services,&thread)==UMI_STATUS_OK);
            int code=-1;
            CHECK(umi_thread_join(thread,&code)==UMI_STATUS_OK && code==0);
            umi_thread_destroy(thread);
            CHECK(umi_task_queue_stats(queue).submitted==0U);
        } else {
            CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK && request==1U);
            UmiCodeGuardReviewSnapshot first=WaitPoll(services);
            CHECK(first.has_report && first.status==UMI_STATUS_OK && !first.pending);
            const UmiCodeGuardResult *retained=UmiStudioCodeGuardResult(services);
            const UmiCodeGuardFinding *finding=umi_codeguard_result_at(retained,0U);
            CHECK(retained!=NULL && finding!=NULL && first.summary.total>0U);
#ifdef UMICOM_STUDIO_CODEGUARD_FAULTS
            if(strcmp(name,"create-failure")==0) {
                CHECK(UmiStudioServicesPrepareWorkspace(services,other,1)==UMI_STATUS_OK);
                review_fault=REVIEW_FAULT_CREATE;
                request=88U;
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OUT_OF_MEMORY);
                CHECK(request==88U && fault_count==1U);
                CHECK(UmiStudioCodeGuardResult(services)==NULL);
                CHECK(UmiStudioServicesPrepareWorkspace(services,project,1)==UMI_STATUS_OK);
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK && request==2U);
                CHECK(WaitPoll(services).status==UMI_STATUS_OK);
            } else if(strcmp(name,"poll-before-admission")==0) {
                uint64_t submitted=umi_task_queue_stats(queue).submitted;
                review_fault=REVIEW_FAULT_POLL;
                request=88U;
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_IO_ERROR);
                CHECK(request==88U && fault_count==1U);
                CHECK(umi_task_queue_stats(queue).submitted==submitted);
                CHECK(UmiStudioCodeGuardResult(services)==retained);
                CHECK(umi_codeguard_result_at(retained,0U)==finding);
            } else if(strcmp(name,"cancel-shutdown-failure")==0) {
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK);
                CHECK(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
                review_fault=REVIEW_FAULT_CANCEL;
                CHECK(UmiStudioServicesStopBackground(services)==UMI_STATUS_IO_ERROR);
                UmiTaskQueueShutdownSnapshot shutdown;
                CHECK(UmiTaskQueueCaptureShutdown(queue,&shutdown)==UMI_STATUS_OK);
                CHECK(shutdown.phase==UMI_TASK_QUEUE_ACCEPTING && fault_count==1U);
                CHECK(UmiStudioServicesStopBackground(services)==UMI_STATUS_OK);
                UmiCodeGuardReviewSnapshot result=Poll(services);
                CHECK(result.status==UMI_STATUS_CANCELLED && result.has_report && !result.pending);
                CHECK(umi_codeguard_result_at(retained,0U)==finding);
            } else if(strcmp(name,"release-failure")==0) {
                UmiStudioServices *owner=services;
                review_fault=REVIEW_FAULT_DESTROY;
                CHECK(UmiStudioServicesRelease(&services)==UMI_STATUS_IO_ERROR);
                CHECK(services==owner && fault_count==1U);
                UmiTaskQueueShutdownSnapshot shutdown;
                CHECK(UmiTaskQueueCaptureShutdown(queue,&shutdown)==UMI_STATUS_OK);
                CHECK(shutdown.phase==UMI_TASK_QUEUE_STOPPED);
                CHECK(UmiStudioCodeGuardResult(services)==retained);
                CHECK(umi_codeguard_result_at(retained,0U)==finding);
            } else
#endif
            if(strcmp(name,"wrong-thread")==0) {
                UmiThread *thread=NULL;
                CHECK(umi_thread_start(WrongThread,services,&thread)==UMI_STATUS_OK);
                int code=-1;
                CHECK(umi_thread_join(thread,&code)==UMI_STATUS_OK && code==0);
                umi_thread_destroy(thread);
            } else if(strcmp(name,"worker-release")==0) {
                UmiTaskConfig config={.label="worker release refusal",.function=WorkerRelease,.user_data=services};
                UmiTask *task=NULL;
                CHECK(umi_task_create(&config,&task)==UMI_STATUS_OK);
                CHECK(umi_task_queue_submit(queue,task)==UMI_STATUS_OK);
                CHECK(umi_task_wait(task,10000U)==UMI_STATUS_OK);
                CHECK(umi_task_result(task)==UMI_STATUS_OK);
                umi_task_destroy(task);
                UmiTaskQueueShutdownSnapshot shutdown;
                CHECK(UmiTaskQueueCaptureShutdown(queue,&shutdown)==UMI_STATUS_OK);
                CHECK(shutdown.phase==UMI_TASK_QUEUE_ACCEPTING);
            } else if(strcmp(name,"idle-cancel")==0) {
                CHECK(UmiStudioCodeGuardCancel(services)==UMI_STATUS_OK);
                UmiCodeGuardReviewSnapshot snapshot=Poll(services);
                CHECK(!snapshot.cancel_requested && snapshot.status==UMI_STATUS_OK && snapshot.has_report);
            } else if(strcmp(name,"queue-stopped")==0) {
                CHECK(umi_task_queue_shutdown(queue,0)==UMI_STATUS_OK);
                request=88U;
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_INVALID_STATE);
                CHECK(request==88U);
                CHECK(UmiStudioCodeGuardResult(services)==retained);
                CHECK(umi_codeguard_result_at(retained,0U)==finding);
            } else {
                if(strcmp(name,"empty")==0) profile.scan_c=0;
                if(strcmp(name,"budget")==0 || strcmp(name,"retry")==0) profile.max_file_bytes=1U;
                UmiTask *blocker=NULL;
                if(strcmp(name,"busy")==0 || strcmp(name,"queued-cancel")==0 ||
                   strcmp(name,"profile-copy")==0 || strcmp(name,"cwd")==0) blocker=OccupyQueue(queue);
                CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK && request==2U);
                if(strcmp(name,"profile-copy")==0) {
                    /* The scheduled job owns its options, not this stack value. */
                    profile.max_file_bytes=1U;
                    profile.scan_c=0;
                }
                if(strcmp(name,"cwd")==0) CHECK(g_chdir(other)==0);
                if(blocker!=NULL) {
                    UmiCodeGuardReviewSnapshot queued;
                    CHECK(UmiStudioCodeGuardSnapshot(services,&queued)==UMI_STATUS_OK);
                    CHECK(queued.pending && queued.has_report);
                    uint64_t rejected=77U;
                    CHECK(UmiStudioCodeGuardBegin(services,&profile,&rejected)==UMI_STATUS_BUSY);
                    CHECK(rejected==77U);
                    if(strcmp(name,"queued-cancel")==0)
                        CHECK(UmiStudioCodeGuardCancel(services)==UMI_STATUS_OK);
                    ReleaseQueue(queue,blocker);
                } else if(strcmp(name,"shutdown")==0) {
                    /* Completion may race shutdown; cancellation must suppress
                     * publication even when the worker has already succeeded. */
                    CHECK(UmiStudioServicesStopBackground(services)==UMI_STATUS_OK);
                } else CHECK(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
                if(strcmp(name,"cancel-terminal")==0)
                    CHECK(UmiStudioCodeGuardCancel(services)==UMI_STATUS_OK);
                if(strcmp(name,"shutdown-terminal")==0)
                    CHECK(UmiStudioServicesStopBackground(services)==UMI_STATUS_OK);
                if(strcmp(name,"close-terminal")==0)
                    CHECK(umi_studio_services_close_workspace(services)==UMI_STATUS_OK);
                if(strcmp(name,"reopen-terminal")==0) {
                    CHECK(umi_studio_services_close_workspace(services)==UMI_STATUS_OK);
                    CHECK(UmiStudioServicesPrepareWorkspace(services,project,1)==UMI_STATUS_OK);
                }
                if(strcmp(name,"switch-terminal")==0 || strcmp(name,"snapshot-stale")==0)
                    CHECK(UmiStudioServicesPrepareWorkspace(services,other,1)==UMI_STATUS_OK);
                if(strcmp(name,"trust-terminal")==0)
                    CHECK(umi_studio_workspace_set_trusted(services,0)==UMI_STATUS_OK);
                if(strcmp(name,"graph-terminal")==0)
                    CHECK(umi_workspace_graph_add_project(umi_studio_services_workspace(services),
                        "manual","Manual","manual",UMI_PROJECT_GENERIC,NULL)==UMI_STATUS_OK);
                int stale=strcmp(name,"close-terminal")==0 || strcmp(name,"reopen-terminal")==0 ||
                    strcmp(name,"switch-terminal")==0 || strcmp(name,"snapshot-stale")==0 ||
                    strcmp(name,"trust-terminal")==0 || strcmp(name,"graph-terminal")==0;
                if(stale) {
                    UmiCodeGuardReviewSnapshot cached;
                    CHECK(UmiStudioCodeGuardSnapshot(services,&cached)==UMI_STATUS_OK);
                    CHECK(!cached.has_report && cached.summary.total==0U && cached.pending);
                    CHECK(UmiStudioCodeGuardResult(services)==NULL);
                }
                UmiCodeGuardReviewSnapshot result=Poll(services);
                CHECK(!result.pending);
                if(stale) {
                    CHECK(result.status==UMI_STATUS_INVALID_STATE && result.stale && !result.has_report);
                    CHECK(result.operation_status==UMI_STATUS_OK);
                    if(strcmp(name,"switch-terminal")==0) {
                        CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK && request==1U);
                        result=WaitPoll(services);
                        CHECK(result.has_report && result.status==UMI_STATUS_OK && result.summary.total==0U);
                        CHECK(result.context_id!=first.context_id);
                    }
                } else if(strcmp(name,"budget")==0 || strcmp(name,"retry")==0) {
                    CHECK(result.status==UMI_STATUS_CAPACITY_EXCEEDED && result.has_report);
                    CHECK(UmiStudioCodeGuardResult(services)==retained);
                    CHECK(umi_codeguard_result_at(retained,0U)==finding);
                    if(strcmp(name,"retry")==0) {
                        profile.max_file_bytes=1024U;
                        CHECK(UmiStudioCodeGuardBegin(services,&profile,&request)==UMI_STATUS_OK && request==3U);
                        result=WaitPoll(services);
                        CHECK(result.status==UMI_STATUS_OK && result.has_report);
                    }
                } else if(strcmp(name,"cancel-terminal")==0 || strcmp(name,"queued-cancel")==0 ||
                          strcmp(name,"shutdown")==0 || strcmp(name,"shutdown-terminal")==0) {
                    CHECK(result.status==UMI_STATUS_CANCELLED && result.has_report);
                    CHECK(UmiStudioCodeGuardResult(services)==retained);
                    CHECK(umi_codeguard_result_at(retained,0U)==finding);
                } else {
                    CHECK(result.status==UMI_STATUS_OK && result.has_report);
                    CHECK(result.summary.total==(strcmp(name,"empty")==0 ? 0U : first.summary.total));
                    CHECK(UmiStudioCodeGuardResult(services)==retained);
                }
            }
        }
    }
    CHECK(UmiStudioServicesRelease(&services)==UMI_STATUS_OK && services==NULL);
    CHECK(StudioTestStateEnd(&state));
    g_free(source);
    g_free(other);
    g_free(project);
    return 0;
}
