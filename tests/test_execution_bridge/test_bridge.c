/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_execution_bridge/test_bridge.c
 * PURPOSE: Exercise publication with real CTest, task and record implementations.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/test_execution.h"
#include "umicom/test_platform/ctest.h"
#include "umicom/test_platform/execution.h"
#include "umicom/test_platform/output.h"
#include "umicom/testing/runner.h"
#include "umicom/platform/threading.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while (0)

/* Only the outer service/workspace composition is substituted. Public record
 * types, record storage, metadata import, tasks, process execution and JUnit
 * checking below are the production Framework implementations. */
typedef struct UmiTestPlatformService {
    UmiTestPlatformItemRegistry *items;
    UmiTestPlatformSuiteRegistry *suites;
    UmiTestPlatformDiscoveryRegistry *discoveries;
    UmiTestPlatformResultRegistry *results;
    UmiTestPlatformOutputRegistry *outputs;
    UmiTestPlatformOperationController operation;
} UmiTestPlatformService;
typedef struct UmiTestWorkspace { unsigned refreshes; } UmiTestWorkspace;
struct UmiStudioTestService {
    UmiTestPlatformService *platform;
    UmiTestWorkspace *workspace;
    UmiTestRunSummary last_summary;
    struct { uint64_t revision; } explorer;
    UmiTaskQueue *discoveryQueue;
    int discoveryPending;

    UmiCtestJob *executionJob;
    UmiTaskQueue *executionQueue;
    UmiStudioTestRunContext executionContext;
    uint64_t executionOwner;
    uint64_t executionItemsRevision;
    uint64_t executionDiscoveryRevision;
    uint64_t executionResultsRevision;
    uint64_t executionOutputRevision;
    uint64_t executionSequence;
    size_t executionPublished;
    int executionPending;
    int executionStale;

};
static int UmiStudioTestDiscoveryPending(const UmiStudioTestService *s) { return s->discoveryPending; }
static UmiTestPlatformItemRegistry *umi_test_platform_service_item(UmiTestPlatformService *s) { return s->items; }
static UmiTestPlatformDiscoveryRegistry *umi_test_platform_service_discovery(UmiTestPlatformService *s) { return s->discoveries; }
static UmiTestPlatformResultRegistry *umi_test_platform_service_result(UmiTestPlatformService *s) { return s->results; }
static UmiTestPlatformOutputRegistry *umi_test_platform_service_output(UmiTestPlatformService *s) { return s->outputs; }
static UmiTestPlatformOperationController *umi_test_platform_service_operation(UmiTestPlatformService *s) { return &s->operation; }
static UmiStatus umi_test_platform_service_begin_operation(UmiTestPlatformService *s, const UmiTestPlatformOperationPlan *p)
{
    if (s->operation.running) return UMI_STATUS_BUSY;
    s->operation.running=1; s->operation.stop_requested=0;
    s->operation.completed_execution_count=0;
    s->operation.planned_execution_count=p->selection.count * (p->repeat_count?p->repeat_count:1U);
    return UMI_STATUS_OK;
}
static UmiStatus umi_test_platform_service_request_stop(UmiTestPlatformService *s)
{ s->operation.stop_requested=1; return UMI_STATUS_OK; }
static void umi_test_platform_service_finish_operation(UmiTestPlatformService *s) { s->operation.running=0; }
void umi_test_platform_operation_mark_completed(UmiTestPlatformOperationController *o) { ++o->completed_execution_count; }
static UmiStatus umi_test_workspace_refresh(UmiTestWorkspace *w) { ++w->refreshes; return UMI_STATUS_OK; }
static void copy_text(char *out, size_t cap, const char *s)
{
    if(out==NULL||cap==0U)return;
    if(s==NULL)s="";
    size_t length=strlen(s);if(length>=cap)length=cap-1U;
    memcpy(out,s,length);out[length]='\0';
}
static int platform_outcome(UmiTestState state)
{
    switch(state){
    case UMI_TEST_STATE_PASSED:return UMI_TEST_PLATFORM_OUTCOME_PASSED;
    case UMI_TEST_STATE_FAILED:return UMI_TEST_PLATFORM_OUTCOME_FAILED;
    case UMI_TEST_STATE_SKIPPED:return UMI_TEST_PLATFORM_OUTCOME_SKIPPED;
    case UMI_TEST_STATE_CANCELLED:return UMI_TEST_PLATFORM_OUTCOME_CANCELLED;
    case UMI_TEST_STATE_TIMED_OUT:return UMI_TEST_PLATFORM_OUTCOME_TIMED_OUT;
    default:return UMI_TEST_PLATFORM_OUTCOME_NOT_RUN;}
}
static int refuseResult;
static UmiStatus UpsertResult(UmiTestPlatformResultRegistry *registry, const UmiTestPlatformResultSnapshot *result)
{ return refuseResult ? UMI_STATUS_IO_ERROR : umi_test_platform_result_registry_upsert(registry,result); }
#define umi_test_platform_result_registry_upsert UpsertResult
#include "../../src/app/test_execution.inc"
#undef umi_test_platform_result_registry_upsert

static UmiCtestJobSnapshot Progress(UmiStudioTestService *service)
{ UmiCtestJobSnapshot s;CHECK(UmiStudioTestRunSnapshot(service,&s)==UMI_STATUS_OK);return s; }
static void AwaitCompleted(UmiStudioTestService *service,size_t count)
{
    for(unsigned n=0;n<10000U;++n){if(Progress(service).completed>=count)return;umi_thread_sleep_ms(1U);}
    CHECK(0);
}
static void AwaitTerminal(UmiStudioTestService *service)
{
    for(unsigned n=0;n<10000U;++n){if(Progress(service).state>=UMI_TASK_SUCCEEDED)return;umi_thread_sleep_ms(1U);}
    CHECK(0);
}
static UmiStatus Finish(UmiStudioTestService *service,UmiStudioTestRunContext *context,char *message)
{
    UmiStatus status=UMI_STATUS_BUSY;
    for(unsigned n=0;n<10000U;++n){
        status=UmiStudioTestRunPoll(service,context,message,512U);
        if(status!=UMI_STATUS_BUSY)return status;
        umi_thread_sleep_ms(1U);
    }
    CHECK(0);return status;
}
static void Add(UmiTestPlatformService *service,UmiTestPlatformOperationPlan *plan,const char *name)
{
    UmiTestPlatformItemSnapshot item;
    for(size_t n=0;n<umi_test_platform_item_registry_count(service->items);++n){
        CHECK(umi_test_platform_item_registry_at(service->items,n,&item)==UMI_STATUS_OK);
        if(strcmp(item.name,name)==0){
            CHECK(plan->selection.count<UMI_TEST_PLATFORM_SELECTION_CAPACITY);
            copy_text(plan->selection.item_ids[plan->selection.count++],128U,item.id);return;
        }
    }
    CHECK(0);
}
static UmiStatus Start(UmiStudioTestService *service,UmiTaskQueue *queue,
    UmiStudioTestRunContext *context,UmiTestPlatformOperationPlan *plan)
{
    UmiStatus status=UmiStudioTestRunArm(service,queue,context);
    if(status==UMI_STATUS_OK){UmiTestPlatformExecutionSummary s={0};status=UmiStudioExecutionStart(service,plan,&s);}
    UmiStudioTestRunDisarm(service);return status;
}
static UmiStatus Blocking(UmiTaskContext *context,void *data)
{
    atomic_int *release=data;
    while(!atomic_load(release)){
        if(umi_task_context_is_cancelled(context))return UMI_STATUS_CANCELLED;
        umi_thread_sleep_ms(1U);
    }
    return UMI_STATUS_OK;
}
int main(int argc,char **argv)
{
    CHECK(argc==3);const char *mode=argv[1];
    UmiTestPlatformService platform={0};UmiTestWorkspace workspace={0};
    UmiStudioTestService service={0};service.platform=&platform;service.workspace=&workspace;
    CHECK(umi_test_platform_item_registry_create(&platform.items)==UMI_STATUS_OK);
    CHECK(umi_test_platform_suite_registry_create(&platform.suites)==UMI_STATUS_OK);
    CHECK(umi_test_platform_discovery_registry_create(&platform.discoveries)==UMI_STATUS_OK);
    CHECK(umi_test_platform_result_registry_create(&platform.results)==UMI_STATUS_OK);
    CHECK(umi_test_platform_output_registry_create(&platform.outputs)==UMI_STATUS_OK);
    UmiTestPlatformCtestImportOptions import={0};
    copy_text(import.project_id,sizeof(import.project_id),"notes");
    copy_text(import.suite_id,sizeof(import.suite_id),"notes.ctest");
    copy_text(import.build_directory,sizeof(import.build_directory),argv[2]);
    copy_text(import.configuration,sizeof(import.configuration),"Debug");
    const char *json="{\"tests\":[{\"name\":\"notes.pass\"},{\"name\":\"notes.wait\"},{\"name\":\"notes.fail\"},"
        "{\"name\":\"notes.skip\"},{\"name\":\"notes.after\"},{\"name\":\"notes.large\"},"
        "{\"name\":\"notes.disabled\",\"properties\":[{\"name\":\"DISABLED\",\"value\":true}]}]}";
    UmiTestPlatformCtestImportSummary imported;
    CHECK(umi_test_platform_ctest_parse_json(json,&import,platform.items,platform.suites,platform.discoveries,&imported)==UMI_STATUS_OK);
    UmiTaskQueueConfig config={0};config.worker_count=1U;config.capacity=4U;
    UmiTaskQueue *queue=NULL;CHECK(umi_task_queue_create(&config,&queue)==UMI_STATUS_OK);
    UmiStudioTestRunContext context={0};context.trusted=1;context.workspace_generation=10U;
    copy_text(context.source_root,sizeof(context.source_root),"Umicom Notes source");
    copy_text(context.build_root,sizeof(context.build_root),argv[2]);
    copy_text(context.configuration,sizeof(context.configuration),"Debug");
    UmiTestPlatformOperationPlan *plan=calloc(1U,sizeof(*plan));CHECK(plan!=NULL);
    plan->kind=UMI_TEST_PLATFORM_OPERATION_RUN_SELECTED;
    Add(&platform,plan,strcmp(mode,"disabled")==0?"notes.disabled":strcmp(mode,"skipped")==0?"notes.skip":
        strcmp(mode,"failure")==0||strcmp(mode,"stop-failure")==0?"notes.fail":
        strcmp(mode,"retained-result")==0?"notes.large":"notes.pass");
    if(strcmp(mode,"repeat")==0)plan->repeat_count=3U;
    if(strcmp(mode,"publication-budget")==0)plan->repeat_count=17U;
    if(strcmp(mode,"partial-stop")==0){Add(&platform,plan,"notes.wait");Add(&platform,plan,"notes.after");}
    if(strcmp(mode,"stop-failure")==0){plan->stop_on_failure=1;Add(&platform,plan,"notes.after");}
    char message[512]={0};UmiTestPlatformResultSnapshot result={0};
    /* Old history remains owned by the shared registries across new runs. */
    copy_text(result.id,sizeof(result.id),"previous");result.sequence=40U;result.outcome=UMI_TEST_PLATFORM_OUTCOME_PASSED;
    CHECK(umi_test_platform_result_registry_upsert(platform.results,&result)==UMI_STATUS_OK);
    service.last_summary.passed=7U;
    if(strcmp(mode,"invalid-kind")==0){plan->kind=(UmiTestPlatformOperationKind)99;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_INVALID_ARGUMENT);goto done;}
    if(strcmp(mode,"stale-selection")==0){plan->selection.source_revision=UINT64_MAX;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_INVALID_STATE);goto done;}
    if(strcmp(mode,"invalid-context")==0){
        context.trusted=0;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_PERMISSION_DENIED);goto done;
    }
    if(strcmp(mode,"busy-operation")==0){platform.operation.running=1;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_BUSY);goto done;}
    if(strcmp(mode,"wrong-root")==0){copy_text(context.build_root,sizeof(context.build_root),"different build");CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_INVALID_STATE);goto done;}
    if(strcmp(mode,"wrong-configuration")==0){copy_text(context.configuration,sizeof(context.configuration),"Release");CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_INVALID_STATE);goto done;}
    if(strcmp(mode,"missing-item")==0){copy_text(plan->selection.item_ids[0],128U,"missing");CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_NOT_FOUND);goto done;}
    if(strcmp(mode,"invalid-plan")==0){plan->selection.count=UMI_TEST_PLATFORM_SELECTION_CAPACITY+1U;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_CAPACITY_EXCEEDED);goto done;}
    if(strcmp(mode,"sequence-overflow")==0){result.sequence=UINT64_MAX;CHECK(umi_test_platform_result_registry_upsert(platform.results,&result)==UMI_STATUS_OK);CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_CAPACITY_EXCEEDED);goto done;}
    if(strcmp(mode,"history-full")==0){
        for(unsigned n=1;n<UMI_TEST_PLATFORM_RESULT_CAPACITY;++n){(void)snprintf(result.id,sizeof(result.id),"old.%u",n);CHECK(umi_test_platform_result_registry_upsert(platform.results,&result)==UMI_STATUS_OK);}
        CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_CAPACITY_EXCEEDED);
        CHECK(umi_test_platform_result_registry_count(platform.results)==UMI_TEST_PLATFORM_RESULT_CAPACITY);goto done;
    }
    if(strcmp(mode,"output-full")==0||strcmp(mode,"existing-output")==0){
        UmiTestPlatformOutputSnapshot output={0};
        unsigned count=strcmp(mode,"output-full")==0?UMI_TEST_PLATFORM_OUTPUT_CAPACITY:1U;
        for(unsigned n=0;n<count;++n){
            if(count==1U)copy_text(output.id,sizeof(output.id),"output.job.41");
            else (void)snprintf(output.id,sizeof(output.id),"old.%u",n);
            CHECK(umi_test_platform_output_registry_upsert(platform.outputs,&output)==UMI_STATUS_OK);
        }
        CHECK(Start(&service,queue,&context,plan)==(count==1U?UMI_STATUS_INVALID_STATE:UMI_STATUS_CAPACITY_EXCEEDED));goto done;
    }
    if(strcmp(mode,"discovery")==0){service.discoveryPending=1;CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_BUSY);goto done;}
    if(strcmp(mode,"double-arm")==0){
        CHECK(UmiStudioTestRunArm(&service,queue,&context)==UMI_STATUS_OK);
        CHECK(UmiStudioTestRunArm(&service,queue,&context)==UMI_STATUS_BUSY);
        UmiStudioTestRunDisarm(&service);
    }
    if(strcmp(mode,"queued-stop")==0){
        atomic_int release;atomic_init(&release,0);UmiTaskConfig blockConfig={0};
        blockConfig.function=Blocking;blockConfig.user_data=&release;UmiTask *block=NULL;
        CHECK(umi_task_create(&blockConfig,&block)==UMI_STATUS_OK);CHECK(umi_task_queue_submit(queue,block)==UMI_STATUS_OK);
        for(unsigned n=0;n<5000U && umi_task_state(block)!=UMI_TASK_RUNNING;++n)umi_thread_sleep_ms(1U);
        CHECK(umi_task_state(block)==UMI_TASK_RUNNING);CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_OK);
        CHECK(UmiStudioTestRunCancel(&service)==UMI_STATUS_OK);CHECK(Finish(&service,&context,message)==UMI_STATUS_CANCELLED);
        CHECK(service.executionPublished==0U&&umi_test_platform_result_registry_count(platform.results)==1U);
        atomic_store(&release,1);CHECK(umi_task_wait(block,5000U)==UMI_STATUS_OK);umi_task_destroy(block);goto done;
    }
    CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_OK);
    CHECK(UmiStudioTestRunPending(&service)&&service.last_summary.total==0U);
    CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_BUSY);
    if(strcmp(mode,"dispose")==0){CHECK(UmiStudioExecutionDispose(&service));CHECK(!UmiStudioTestRunPending(&service));goto done;}
    if(strcmp(mode,"partial-stop")==0){
        AwaitCompleted(&service,1U);
        CHECK(UmiStudioTestRunPoll(&service,&context,message,sizeof(message))==UMI_STATUS_BUSY);
        CHECK(service.last_summary.passed==1U&&service.executionPublished==1U);
        for(unsigned n=0;n<5000U;++n){
            if(strcmp(Progress(&service).active_name,"notes.wait")==0)break;
            umi_thread_sleep_ms(1U);
        }
        CHECK(strcmp(Progress(&service).active_name,"notes.wait")==0);
        CHECK(UmiStudioTestRunCancel(&service)==UMI_STATUS_OK);
        CHECK(Finish(&service,&context,message)==UMI_STATUS_CANCELLED);
        CHECK(service.last_summary.passed==1U&&service.last_summary.cancelled==1U);
        CHECK(Progress(&service).completed==2U);goto done;
    }
    /* Mutate only owner-thread state after the worker is finished but before
     * publication. Even a completed success cannot bypass a changed boundary. */
    AwaitTerminal(&service);
    if(strcmp(mode,"publication-budget")==0){
        CHECK(UmiStudioTestRunPoll(&service,&context,message,sizeof(message))==UMI_STATUS_BUSY);
        CHECK(service.executionPublished==16U&&service.executionPending);
    }
    int stale=0;
    if(strcmp(mode,"generation")==0){++context.workspace_generation;stale=1;}
    if(strcmp(mode,"source")==0){strcpy(context.source_root,"Other project");stale=1;}
    if(strcmp(mode,"configuration")==0){strcpy(context.configuration,"Release");stale=1;}
    if(strcmp(mode,"trust")==0){context.trusted=0;stale=1;}
    if(strcmp(mode,"copied-context")==0){UmiStudioTestRunContext changed=context;strcpy(changed.source_root,"mutated copy");CHECK(strcmp(service.executionContext.source_root,changed.source_root)!=0);}
    if(strcmp(mode,"catalogue")==0){umi_test_platform_item_registry_clear(platform.items);stale=1;}
    if(strcmp(mode,"history")==0){umi_test_platform_result_registry_clear(platform.results);stale=1;}
    if(strcmp(mode,"output")==0){umi_test_platform_output_registry_clear(platform.outputs);stale=1;}
    if(strcmp(mode,"late-stop")==0)CHECK(UmiStudioTestRunCancel(&service)==UMI_STATUS_OK);
    if(strcmp(mode,"publish-refused")==0)refuseResult=1;
    UmiStatus finished=Finish(&service,&context,message);
    if(stale||refuseResult){
        CHECK(finished==UMI_STATUS_INVALID_STATE&&service.executionPublished==0U);
        CHECK(umi_test_platform_output_registry_count(platform.outputs)==0U);goto done;
    }
    if(strcmp(mode,"failure")==0||strcmp(mode,"stop-failure")==0||strcmp(mode,"retained-result")==0)
        CHECK(finished!=UMI_STATUS_OK);
    else CHECK(finished==UMI_STATUS_OK);
    CHECK(!platform.operation.running&&workspace.refreshes==1U);
    CHECK(umi_test_platform_result_registry_find(platform.results,"previous",&result)==UMI_STATUS_OK);
    CHECK(result.sequence==40U&&result.outcome==UMI_TEST_PLATFORM_OUTCOME_PASSED);
    CHECK(umi_test_platform_result_registry_count(platform.results)==Progress(&service).completed+1U);
    CHECK(umi_test_platform_output_registry_count(platform.outputs)==Progress(&service).completed);
    CHECK(service.last_summary.total==Progress(&service).completed);
    if(strcmp(mode,"repeat")==0)CHECK(service.last_summary.passed==3U);
    if(strcmp(mode,"disabled")==0||strcmp(mode,"skipped")==0)CHECK(service.last_summary.skipped==1U&&service.last_summary.passed==0U);
    if(strcmp(mode,"stop-failure")==0)CHECK(Progress(&service).planned==2U&&Progress(&service).completed==1U);
    if(strcmp(mode,"retained-result")==0){
        UmiTestResult full;UmiTestPlatformOutputSnapshot compact;uint32_t attempt=0U;
        CHECK(UmiStudioTestRunResultAt(&service,0U,&full,&attempt)==UMI_STATUS_OK&&attempt==1U);
        CHECK(strstr(full.output,"NOTES OUTPUT TAIL")!=NULL);
        CHECK(umi_test_platform_output_registry_at(platform.outputs,0U,&compact)==UMI_STATUS_OK);
        CHECK(strstr(compact.text,"Earlier output omitted")!=NULL&&strstr(compact.text,"NOTES OUTPUT TAIL")!=NULL);
        UmiCtestJob *retained=service.executionJob;plan->selection.count=0U;
        CHECK(Start(&service,queue,&context,plan)==UMI_STATUS_INVALID_ARGUMENT&&service.executionJob==retained);
    }
done:
    UmiStudioTestRunDisarm(&service);CHECK(UmiStudioExecutionDispose(&service));
    CHECK(umi_task_queue_shutdown(queue, 1)==UMI_STATUS_OK);umi_task_queue_destroy(queue);
    umi_test_platform_item_registry_destroy(platform.items);umi_test_platform_suite_registry_destroy(platform.suites);
    umi_test_platform_discovery_registry_destroy(platform.discoveries);umi_test_platform_result_registry_destroy(platform.results);
    umi_test_platform_output_registry_destroy(platform.outputs);free(plan);
    return 0;
}
