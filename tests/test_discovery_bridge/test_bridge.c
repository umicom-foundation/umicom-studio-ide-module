/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_discovery_bridge/test_bridge.c
 * PURPOSE:
 *   Exercise the delivered bridge implementation with real Framework task/JSON
 *   registries and explicitly substituted outer service and process boundaries.
 *   This does not compile tests.c, the full service graph or GTK frontend.
 * AUTHOR AND ORGANISATION: Sammy Hegab | Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "test_support.h"
#include "umicom/studio/test_discovery.h"
#include "umicom/platform/process.h"

/* The bridge only uses these outer-service fields. Stand-ins are deliberately
 * local to this test; no replacement public headers enter a repository SDK. */
typedef struct UmiTestPlatformService {
    UmiTestPlatformItemRegistry *items;
    UmiTestPlatformSuiteRegistry *suites;
    UmiTestPlatformDiscoveryRegistry *discoveries;
    int operationRunning;
} UmiTestPlatformService;
typedef struct UmiTestPlatformServiceSnapshot { int operation_running; } UmiTestPlatformServiceSnapshot;
typedef struct UmiTestWorkspace { UmiStatus refreshStatus; unsigned refreshCount; } UmiTestWorkspace;
struct UmiStudioTestService {
    UmiTestPlatformService *platform;
    UmiTestWorkspace *workspace;
    char build_directory[1024];
    struct { char workspace_root[1024], active_project_id[128], active_suite_id[128]; uint64_t revision; } explorer;
    struct { size_t passed; } last_summary;
    UmiCtestDiscoveryJob *discoveryJob;
    UmiTaskQueue *discoveryQueue;
    UmiTestPlatformCtestImportOptions discoveryOptions;
    char discoverySourceRoot[1024];
    uint64_t discoveryGeneration;
    int discoveryStopRequested;
};
static void copy_text(char *out,size_t capacity,const char *value)
{ snprintf(out,capacity,"%s",value!=NULL ? value : ""); }
static UmiStatus umi_test_platform_service_snapshot(const UmiTestPlatformService *service,
    UmiTestPlatformServiceSnapshot *out)
{ out->operation_running=service->operationRunning;return UMI_STATUS_OK; }
static UmiTestPlatformItemRegistry *umi_test_platform_service_item(UmiTestPlatformService *service)
{ return service->items; }
static UmiTestPlatformSuiteRegistry *umi_test_platform_service_suite(UmiTestPlatformService *service)
{ return service->suites; }
static UmiTestPlatformDiscoveryRegistry *umi_test_platform_service_discovery(UmiTestPlatformService *service)
{ return service->discoveries; }
static UmiStatus umi_test_workspace_refresh(UmiTestWorkspace *workspace)
{ ++workspace->refreshCount;return workspace->refreshStatus; }
#include "../../src/app/test_discovery.inc"

static atomic_int entered,released;
UmiStatus UmiProcessExecuteStreamed(const UmiProcessRequest *request,
    UmiProcessOutputObserver observer,void *payload,UmiProcessResult *result)
{
    const char json[]="{\"kind\":\"ctestInfo\",\"version\":{\"major\":1,\"minor\":0},\"tests\":[{\"name\":\"notes.save\"}]}";
    atomic_store(&entered,1);
    memset(result,0,sizeof(*result));result->launched=1;result->exit_code=-1;
    while(!atomic_load(&released)) {
        if(umi_cancellation_token_is_requested(request->cancellation)) {
            result->cancelled=1;return UMI_STATUS_CANCELLED;
        }
        umi_thread_sleep_ms(1U);
    }
    observer(json,strlen(json),payload);result->exit_code=0;return UMI_STATUS_OK;
}
static int ContainsName(const UmiTestPlatformItemRegistry *items,const char *name)
{
    UmiTestPlatformItemSnapshot item;
    for(size_t index=0U;index<umi_test_platform_item_registry_count(items);++index) {
        CHECK(umi_test_platform_item_registry_at(items,index,&item)==UMI_STATUS_OK);
        if(strcmp(item.name,name)==0) return 1;
    }
    return 0;
}
int main(int argc,char **argv)
{
    CHECK(argc==3);const char *scenario=argv[1];
    UmiTaskQueue *queue=TestQueue(1U,4U);
    UmiTestPlatformService platform={0};UmiTestWorkspace workspace={UMI_STATUS_OK,0U};
    UmiStudioTestService service={0};service.platform=&platform;service.workspace=&workspace;
    CHECK(umi_test_platform_item_registry_create(&platform.items)==UMI_STATUS_OK);
    CHECK(umi_test_platform_suite_registry_create(&platform.suites)==UMI_STATUS_OK);
    CHECK(umi_test_platform_discovery_registry_create(&platform.discoveries)==UMI_STATUS_OK);
    atomic_init(&entered,0);atomic_init(&released,0);
    UmiTestPlatformCtestImportOptions options={0};
    copy_text(options.project_id,sizeof(options.project_id),"notes");
    copy_text(options.suite_id,sizeof(options.suite_id),"notes.ctest");
    copy_text(options.build_directory,sizeof(options.build_directory),argv[2]);
    copy_text(options.configuration,sizeof(options.configuration),"Debug");
    /* Seed a real non-empty catalogue: Stop must retain this record, while a
     * successful refresh must remove it rather than merely append a new test. */
    UmiTestPlatformCtestImportSummary initial;
    const char previousJson[] = "{\"tests\":[{\"name\":\"notes.previous\"}]}";
    CHECK(umi_test_platform_ctest_parse_json(previousJson,&options,platform.items,
        platform.suites,platform.discoveries,&initial)==UMI_STATUS_OK);
    CHECK(initial.discovered_count==1U && ContainsName(platform.items,"notes.previous"));
    copy_text(service.build_directory,sizeof(service.build_directory),"previous build");
    service.last_summary.passed=77U;
    CHECK(UmiStudioTestDiscoveryArm(&service,queue,42U)==UMI_STATUS_OK);
    if(strcmp(scenario,"double-arm")==0) {
        CHECK(UmiStudioTestDiscoveryArm(&service,queue,43U)==UMI_STATUS_BUSY);
        UmiStudioTestDiscoveryDisarm(&service);
        CHECK(service.discoveryQueue==NULL);
        CHECK(UmiStudioTestDiscoveryArm(&service,queue,42U)==UMI_STATUS_OK);
    }
    if(strcmp(scenario,"operation-busy")==0) {
        platform.operationRunning=1;
        CHECK(UmiStudioDiscoveryStart(&service,"notes source",&options)==UMI_STATUS_BUSY);
        CHECK(!UmiStudioTestDiscoveryPending(&service) && service.discoveryQueue==NULL);
    } else {
        CHECK(UmiStudioDiscoveryStart(&service,"notes source",&options)==UMI_STATUS_OK);
        UmiStudioTestDiscoveryDisarm(&service);
        CHECK(UmiStudioTestDiscoveryPending(&service));TestAwait(&entered);
        CHECK(umi_test_platform_item_registry_count(platform.items)==2U);
        CHECK(ContainsName(platform.items,"notes.previous") && !ContainsName(platform.items,"notes.save"));
        CHECK(strcmp(service.build_directory,"previous build")==0 && service.last_summary.passed==77U);
        if(strcmp(scenario,"dispose")==0) {
            CHECK(UmiStudioDiscoveryDispose(&service));CHECK(!UmiStudioTestDiscoveryPending(&service));
        } else {
            if(strcmp(scenario,"stop")==0) CHECK(UmiStudioDiscoveryCancelPending(&service)==UMI_STATUS_OK);
            if(strcmp(scenario,"view-capacity")==0) workspace.refreshStatus=UMI_STATUS_CAPACITY_EXCEEDED;
            atomic_store(&released,1);
            int published=0;char message[512];UmiStatus status;
            uint64_t start=TestNowMs();
            do {
                status=UmiStudioTestDiscoveryPoll(&service,
                    strcmp(scenario,"source")==0 ? "another source" : "notes source",
                    strcmp(scenario,"build")==0 ? "another build" : argv[2],
                    strcmp(scenario,"configuration")==0 ? "Release" : "Debug",
                    strcmp(scenario,"generation")==0 ? 43U : 42U,
                    strcmp(scenario,"trust")!=0,&published,message,sizeof(message));
                if(status==UMI_STATUS_BUSY) umi_thread_sleep_ms(1U);
                CHECK(TestNowMs()-start<10000U);
            } while(status==UMI_STATUS_BUSY);
            int success=strcmp(scenario,"success")==0 || strcmp(scenario,"double-arm")==0 ||
                strcmp(scenario,"view-capacity")==0;
            CHECK(status==(success ? UMI_STATUS_OK : UMI_STATUS_CANCELLED));
            CHECK(published==success);
            CHECK(!UmiStudioTestDiscoveryPending(&service));
            if(success) {
                CHECK(umi_test_platform_item_registry_count(platform.items)==2U);
                CHECK(ContainsName(platform.items,"notes.save") && !ContainsName(platform.items,"notes.previous"));
                CHECK(service.last_summary.passed==0U && workspace.refreshCount==1U);
                CHECK(strcmp(service.build_directory,argv[2])==0 && strstr(message,"Discovered 1")!=NULL);
                if(strcmp(scenario,"view-capacity")==0) CHECK(strstr(message,"narrower filter")!=NULL);
            } else {
                CHECK(umi_test_platform_item_registry_count(platform.items)==2U);
        CHECK(ContainsName(platform.items,"notes.previous") && !ContainsName(platform.items,"notes.save"));
                CHECK(service.last_summary.passed==77U && workspace.refreshCount==0U);
                CHECK(strcmp(service.build_directory,"previous build")==0);
            }
            CHECK(UmiStudioTestDiscoveryPoll(&service,"notes source",argv[2],"Debug",42U,1,
                &published,message,sizeof(message))==UMI_STATUS_NOT_FOUND);
        }
    }
    CHECK(umi_task_queue_wait_idle(queue,5000U)==UMI_STATUS_OK);umi_task_queue_destroy(queue);
    umi_test_platform_item_registry_destroy(platform.items);
    umi_test_platform_suite_registry_destroy(platform.suites);
    umi_test_platform_discovery_registry_destroy(platform.discoveries);
    printf("PASS studio.discovery_bridge.%s\n",scenario);return 0;
}
