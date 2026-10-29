/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_service_lifecycle.c
 *
 * PURPOSE:
 *   Verify service and bootstrap owners survive refused or incomplete shutdown.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Assertions execute setup and checks in every build configuration. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "umicom/studio/bootstrap.h"

typedef struct WorkerAttempt {
    UmiStudioServices *services;
    UmiStudioBootstrap *bootstrap;
    unsigned attempts;
} WorkerAttempt;

static UmiStudioServices *CreateServices(void)
{
    UmiStudioServices *services = NULL;
    UmiStudioServicesOptions options = {0};
    assert(umi_studio_services_create_with_options(
        NULL, NULL, &options, &services) == UMI_STATUS_OK);
    return services;
}

static UmiStudioBootstrap *CreateBootstrap(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServicesOptions options = {0};
    assert(umi_studio_bootstrap_create_with_options(&options, &bootstrap)
           == UMI_STATUS_OK);
    return bootstrap;
}

static UmiStatus AttemptWorkerRelease(UmiTaskContext *context, void *data)
{
    WorkerAttempt *work = data;
    UmiStudioServices *services = work->services;
    (void)context;
    assert(UmiTaskQueueIsCurrentWorker(umi_studio_services_task_queue(services)));
    assert(UmiStudioServicesStopBackground(services) == UMI_STATUS_INVALID_STATE);
    assert(UmiStudioServicesRelease(&work->services) == UMI_STATUS_INVALID_STATE);
    assert(work->services == services);
    umi_studio_services_destroy(services);
    assert(umi_studio_services_documents(services) != NULL);
    if (work->bootstrap != NULL) {
        UmiStudioBootstrap *bootstrap = work->bootstrap;
        assert(UmiStudioBootstrapRelease(&work->bootstrap)
               == UMI_STATUS_INVALID_STATE);
        assert(work->bootstrap == bootstrap);
        umi_studio_bootstrap_destroy(bootstrap);
        assert(umi_studio_bootstrap_services(bootstrap) == services);
        assert(umi_studio_bootstrap_ui(bootstrap) != NULL);
    }
    ++work->attempts;
    return UMI_STATUS_OK;
}

static void CheckWorkerRefusal(int with_bootstrap)
{
    WorkerAttempt work = {0};
    UmiTask *task = NULL;
    if (with_bootstrap) {
        work.bootstrap = CreateBootstrap();
        work.services = umi_studio_bootstrap_services(work.bootstrap);
    } else {
        work.services = CreateServices();
    }
    UmiTaskQueue *queue = umi_studio_services_task_queue(work.services);
    UmiTaskConfig item = {"Check shutdown ownership", AttemptWorkerRelease,
                          &work, NULL, NULL};
    assert(umi_task_create(&item, &task) == UMI_STATUS_OK);
    assert(umi_task_queue_submit(queue, task) == UMI_STATUS_OK);
    assert(umi_task_wait(task, 10000U) == UMI_STATUS_OK);
    assert(umi_task_result(task) == UMI_STATUS_OK);
    assert(work.attempts == 1U);
    /* A refused call must not close admission or stop unrelated owners. */
    UmiTaskQueueShutdownSnapshot snapshot;
    assert(UmiTaskQueueCaptureShutdown(queue, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.phase == UMI_TASK_QUEUE_ACCEPTING);
    if (with_bootstrap) {
        assert(UmiStudioBootstrapRelease(&work.bootstrap) == UMI_STATUS_OK);
        assert(work.bootstrap == NULL);
        work.services = NULL; /* The bootstrap owned this borrowed reference. */
    } else {
        assert(UmiStudioServicesRelease(&work.services) == UMI_STATUS_OK);
        assert(work.services == NULL);
    }
    /* The caller's task reference remains valid after the queue owner closes. */
    assert(umi_task_result(task) == UMI_STATUS_OK);
    umi_task_destroy(task);
}


static void CheckSessionCycles(void)
{
    UmiStudioBootstrap *bootstrap = CreateBootstrap();
    UmiDesktopSessionRecoverySnapshot snapshot;
    for (unsigned cycle = 0U; cycle < 2U; ++cycle) {
        assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
        assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
        assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
        assert(umi_desktop_session_recovery_snapshot(
            umi_studio_bootstrap_session_recovery(bootstrap), &snapshot) == UMI_STATUS_OK);
        assert(snapshot.clean_shutdown);
        assert(snapshot.state == UMI_DESKTOP_SESSION_RECOVERY_CLEAN);
    }
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
}


typedef struct ServiceDestructorAttempt {
    UmiStudioBootstrap *bootstrap;
    unsigned calls;
} ServiceDestructorAttempt;

static void AttemptServiceDestructorRelease(void *service, void *data)
{
    ServiceDestructorAttempt *attempt = data;
    assert(service == attempt);
    UmiStudioBootstrap *borrowed = attempt->bootstrap;
    UmiStudioServices *services = umi_studio_bootstrap_services(borrowed);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(borrowed);
    assert(UmiStudioBootstrapRelease(&borrowed) == UMI_STATUS_INVALID_STATE);
    assert(borrowed == attempt->bootstrap);
    umi_studio_bootstrap_destroy(borrowed);
    assert(umi_studio_bootstrap_ui(borrowed) == ui);
    assert(umi_studio_bootstrap_services(borrowed) == services);
    UmiTaskQueueShutdownSnapshot snapshot;
    assert(UmiTaskQueueCaptureShutdown(umi_studio_services_task_queue(services),
        &snapshot) == UMI_STATUS_OK);
    assert(snapshot.phase == UMI_TASK_QUEUE_ACCEPTING);
    ++attempt->calls;
}

static void CheckServiceDestructorRefusal(void)
{
    ServiceDestructorAttempt attempt = {CreateBootstrap(), 0U};
    UmiServiceRegistry *registry = umi_studio_bootstrap_service_registry(attempt.bootstrap);
    UmiServiceDescriptor descriptor = {0};
    descriptor.structure_size = (uint32_t)sizeof(descriptor);
    descriptor.service_id = "example.studio.destructor";
    descriptor.provider_module_id = "example.test";
    descriptor.service = &attempt;
    descriptor.flags = UMI_SERVICE_OWNED;
    descriptor.destroy = AttemptServiceDestructorRelease;
    descriptor.destroy_user_data = &attempt;
    assert(umi_service_registry_register(registry, &descriptor) == UMI_STATUS_OK);
    assert(umi_service_registry_unregister(registry, descriptor.service_id,
        descriptor.provider_module_id) == UMI_STATUS_OK);
    assert(attempt.calls == 1U);
    assert(UmiStudioBootstrapRelease(&attempt.bootstrap) == UMI_STATUS_OK);
    assert(attempt.bootstrap == NULL);
}

#ifdef UMICOM_STUDIO_LIFECYCLE_FAULTS
typedef enum FaultSite {
    FAULT_NONE,
    FAULT_WATCHER_STOP,
    FAULT_SUPERVISOR_STOP,
    FAULT_QUEUE_STOP,
    FAULT_SUPERVISOR_RELEASE,
    FAULT_SESSION_CLOSE,
    FAULT_MASTER_STOP,
    FAULT_MASTER_RELEASE
} FaultSite;
static FaultSite next_fault;
static unsigned injected_faults;

static int Inject(FaultSite site)
{
    if (next_fault != site) return 0;
    next_fault = FAULT_NONE;
    ++injected_faults;
    return 1;
}

UmiStatus __real_umi_watcher_stop(UmiWatcher *);
UmiStatus __wrap_umi_watcher_stop(UmiWatcher *watcher)
{
    if (Inject(FAULT_WATCHER_STOP)) return UMI_STATUS_IO_ERROR;
    return __real_umi_watcher_stop(watcher);
}
UmiStatus __real_umi_process_supervisor_shutdown(UmiProcessSupervisor *);
UmiStatus __wrap_umi_process_supervisor_shutdown(UmiProcessSupervisor *supervisor)
{
    if (Inject(FAULT_SUPERVISOR_STOP)) return UMI_STATUS_IO_ERROR;
    return __real_umi_process_supervisor_shutdown(supervisor);
}
UmiStatus __real_umi_task_queue_shutdown(UmiTaskQueue *, int);
UmiStatus __wrap_umi_task_queue_shutdown(UmiTaskQueue *queue, int cancel_pending)
{
    if (Inject(FAULT_QUEUE_STOP)) return UMI_STATUS_IO_ERROR;
    return __real_umi_task_queue_shutdown(queue, cancel_pending);
}
UmiStatus __real_UmiProcessSupervisorRelease(UmiProcessSupervisor **);
UmiStatus __wrap_UmiProcessSupervisorRelease(UmiProcessSupervisor **supervisor)
{
    if (Inject(FAULT_SUPERVISOR_RELEASE)) return UMI_STATUS_IO_ERROR;
    return __real_UmiProcessSupervisorRelease(supervisor);
}


UmiStatus __real_umi_studio_federated_workspace_session_end(UmiDesktopSessionRecovery *);
UmiStatus __wrap_umi_studio_federated_workspace_session_end(UmiDesktopSessionRecovery *recovery)
{
    if (Inject(FAULT_SESSION_CLOSE)) return UMI_STATUS_CAPACITY_EXCEEDED;
    return __real_umi_studio_federated_workspace_session_end(recovery);
}

static void CheckSessionRetry(int release_owner)
{
    UmiStudioBootstrap *bootstrap = CreateBootstrap();
    UmiStudioBootstrap *original = bootstrap;
    UmiDesktopSessionRecoverySnapshot snapshot;
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    next_fault = FAULT_SESSION_CLOSE;
    UmiStatus status = release_owner
        ? UmiStudioBootstrapRelease(&bootstrap)
        : umi_studio_bootstrap_stop(bootstrap);
    assert(status == UMI_STATUS_CAPACITY_EXCEEDED);
    assert(injected_faults == 1U && bootstrap == original);
    assert(umi_desktop_session_recovery_snapshot(
        umi_studio_bootstrap_session_recovery(bootstrap), &snapshot) == UMI_STATUS_OK);
    assert(!snapshot.clean_shutdown);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_INVALID_STATE);
    /* The module stop already completed. The second call must retry the
     * failed checkpoint instead of using started == 0 as proof of success. */
    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    assert(umi_desktop_session_recovery_snapshot(
        umi_studio_bootstrap_session_recovery(bootstrap), &snapshot) == UMI_STATUS_OK);
    assert(snapshot.clean_shutdown);
    assert(snapshot.state == UMI_DESKTOP_SESSION_RECOVERY_CLEAN);
    assert(injected_faults == 1U);
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
    assert(bootstrap == NULL);
}

UmiStatus __real_umi_master_controller_stop(UmiMasterController *);
UmiStatus __wrap_umi_master_controller_stop(UmiMasterController *master)
{
    if (Inject(FAULT_MASTER_STOP)) return UMI_STATUS_IO_ERROR;
    return __real_umi_master_controller_stop(master);
}
UmiStatus __real_UmiMasterControllerRelease(UmiMasterController **);
UmiStatus __wrap_UmiMasterControllerRelease(UmiMasterController **master)
{
    if (Inject(FAULT_MASTER_RELEASE)) return UMI_STATUS_IO_ERROR;
    return __real_UmiMasterControllerRelease(master);
}

static void CheckRetry(const char *name)
{
    UmiStudioBootstrap *bootstrap = CreateBootstrap();
    UmiStudioBootstrap *original = bootstrap;
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    if (strcmp(name, "watcher-stop") == 0) next_fault = FAULT_WATCHER_STOP;
    else if (strcmp(name, "supervisor-stop") == 0) next_fault = FAULT_SUPERVISOR_STOP;
    else if (strcmp(name, "queue-stop") == 0) next_fault = FAULT_QUEUE_STOP;
    else if (strcmp(name, "supervisor-release") == 0)
        next_fault = FAULT_SUPERVISOR_RELEASE;
    else if (strcmp(name, "master-stop") == 0) {
        assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
        next_fault = FAULT_MASTER_STOP;
    } else if (strcmp(name, "master-release") == 0)
        next_fault = FAULT_MASTER_RELEASE;
    else assert(!"unknown fault case");
    FaultSite selected = next_fault;
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_IO_ERROR);
    assert(injected_faults == 1U && next_fault == FAULT_NONE);
    assert(bootstrap == original);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_INVALID_STATE);
    assert(umi_studio_bootstrap_services(bootstrap) == services);
    if (selected == FAULT_MASTER_RELEASE) {
        assert(umi_studio_bootstrap_ui(bootstrap) == NULL);
        assert(umi_studio_services_documents(services) != NULL);
    } else if (selected == FAULT_SUPERVISOR_RELEASE) {
        /* Model release already progressed; retry must accept NULL children. */
        assert(umi_studio_bootstrap_ui(bootstrap) == NULL);
        assert(umi_studio_services_workspace(services) == NULL);
    } else {
        assert(umi_studio_bootstrap_ui(bootstrap) != NULL);
        assert(umi_studio_services_documents(services) != NULL);
    }
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
    assert(bootstrap == NULL);
    assert(injected_faults == 1U);
}
#endif

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (strcmp(argv[1], "empty") == 0) {
        UmiStudioServices *services = NULL;
        UmiStudioBootstrap *bootstrap = NULL;
        assert(UmiStudioServicesStopBackground(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        assert(UmiStudioServicesRelease(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        assert(UmiStudioServicesRelease(&services) == UMI_STATUS_OK);
        assert(UmiStudioBootstrapRelease(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
        umi_studio_services_destroy(NULL);
        umi_studio_bootstrap_destroy(NULL);
    } else if (strcmp(argv[1], "services") == 0) {
        UmiStudioServices *services = CreateServices();
        assert(UmiStudioServicesStopBackground(services) == UMI_STATUS_OK);
        assert(umi_studio_services_documents(services) != NULL);
        assert(UmiStudioServicesStopBackground(services) == UMI_STATUS_OK);
        assert(UmiStudioServicesRelease(&services) == UMI_STATUS_OK);
        assert(services == NULL);
    } else if (strcmp(argv[1], "bootstrap") == 0) {
        UmiStudioBootstrap *bootstrap = CreateBootstrap();
        assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
        assert(bootstrap == NULL);
    } else if (strcmp(argv[1], "bootstrap-session") == 0) {
        CheckSessionCycles();
    } else if (strcmp(argv[1], "worker-services") == 0) {
        CheckWorkerRefusal(0);
    } else if (strcmp(argv[1], "service-destructor") == 0) {
        CheckServiceDestructorRefusal();
    } else if (strcmp(argv[1], "worker-bootstrap") == 0) {
        CheckWorkerRefusal(1);
#ifdef UMICOM_STUDIO_LIFECYCLE_FAULTS
    } else if (strcmp(argv[1], "session-close") == 0) {
        CheckSessionRetry(0);
    } else if (strcmp(argv[1], "session-release") == 0) {
        CheckSessionRetry(1);
    } else {
        CheckRetry(argv[1]);
#else
    } else {
        assert(!"unknown lifecycle case");
#endif
    }
    return 0;
}
