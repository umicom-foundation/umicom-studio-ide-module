/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/test_discovery.inc
 *
 * PURPOSE:
 *   Adapt the existing Studio discovery command to Framework's supervised job.
 *   Only the owner thread touches Studio state; workers receive private catalogues.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/platform/process_search_path.h"

/* The compatibility arm keeps inherited PATH; the tool-aware arm owns a validated copy before dispatch. The previous implementation is retained for engineering review. */
#if 0
UmiStatus UmiStudioTestDiscoveryArm(UmiStudioTestService *service,
    UmiTaskQueue *queue, uint64_t workspaceGeneration)
{
    if (service == NULL || queue == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->discoveryJob != NULL || service->discoveryQueue != NULL)
        return UMI_STATUS_BUSY;
    service->discoveryQueue = queue;
    service->discoveryGeneration = workspaceGeneration;
    return UMI_STATUS_OK;
}
#endif
UmiStatus UmiStudioTestDiscoveryArm(UmiStudioTestService *service,
    UmiTaskQueue *queue, uint64_t workspaceGeneration)
{
    return UmiStudioTestDiscoveryArmWithToolDirectory(service,queue,workspaceGeneration,NULL);
}

UmiStatus UmiStudioTestDiscoveryArmWithToolDirectory(UmiStudioTestService *service,
    UmiTaskQueue *queue, uint64_t workspaceGeneration, const char *directory)
{
    if (service == NULL || queue == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (service->discoveryJob != NULL || service->discoveryQueue != NULL)
        return UMI_STATUS_BUSY;
    if (directory != NULL && directory[0] != '\0') {
        UmiStatus checked = UmiProcessSearchDirectoryValidate(directory);
        if (checked != UMI_STATUS_OK) return checked;
        if (strlen(directory) >= sizeof service->discoveryToolDirectory)
            return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    copy_text(service->discoveryToolDirectory,sizeof service->discoveryToolDirectory,directory);
    service->discoveryQueue = queue;
    service->discoveryGeneration = workspaceGeneration;
    return UMI_STATUS_OK;
}

void UmiStudioTestDiscoveryDisarm(UmiStudioTestService *service)
{
    if (service != NULL) service->discoveryQueue = NULL;
}

int UmiStudioTestDiscoveryPending(const UmiStudioTestService *service)
{
    return service != NULL && service->discoveryJob != NULL;
}

/* The shared discovery job receives a copied project tool choice, keeping command dispatch and worker lifetimes separate. The previous implementation is retained for engineering review. */
#if 0
static UmiStatus UmiStudioDiscoveryStart(UmiStudioTestService *service,
    const char *workspaceRoot, const UmiTestPlatformCtestImportOptions *options)
{
    UmiTaskQueue *queue = service->discoveryQueue;
    UmiTestPlatformServiceSnapshot snapshot;
    UmiStatus status;
    service->discoveryQueue = NULL;
    if (service->discoveryJob != NULL) return UMI_STATUS_BUSY;
    status = umi_test_platform_service_snapshot(service->platform, &snapshot);
    if (status != UMI_STATUS_OK) return status;
    if (snapshot.operation_running) return UMI_STATUS_BUSY;
    status = UmiCtestDiscoveryJobStart(queue, options, NULL,
        umi_test_platform_service_item(service->platform),
        umi_test_platform_service_suite(service->platform),
        umi_test_platform_service_discovery(service->platform),
        service->discoveryGeneration, &service->discoveryJob);
    if (status != UMI_STATUS_OK) return status;
    service->discoveryOptions = *options;
    copy_text(service->discoverySourceRoot, sizeof(service->discoverySourceRoot), workspaceRoot);
    service->discoveryStopRequested = 0;
    return UMI_STATUS_OK;
}
#endif
static UmiStatus UmiStudioDiscoveryStart(UmiStudioTestService *service,
    const char *workspaceRoot, const UmiTestPlatformCtestImportOptions *options)
{
    UmiTaskQueue *queue = service->discoveryQueue;
    UmiTestPlatformServiceSnapshot snapshot;
    UmiStatus status;
    service->discoveryQueue = NULL;
    if (service->discoveryJob != NULL) return UMI_STATUS_BUSY;
    status = umi_test_platform_service_snapshot(service->platform, &snapshot);
    if (status != UMI_STATUS_OK) return status;
    if (snapshot.operation_running) return UMI_STATUS_BUSY;
    UmiTestPlatformCtestDiscoveryOptions execution = {0};
    execution.tool_directory = service->discoveryToolDirectory;
    status = UmiCtestDiscoveryJobStart(queue, options, &execution,
        umi_test_platform_service_item(service->platform),
        umi_test_platform_service_suite(service->platform),
        umi_test_platform_service_discovery(service->platform),
        service->discoveryGeneration, &service->discoveryJob);
    if (status != UMI_STATUS_OK) return status;
    service->discoveryOptions = *options;
    copy_text(service->discoverySourceRoot, sizeof(service->discoverySourceRoot), workspaceRoot);
    service->discoveryStopRequested = 0;
    return UMI_STATUS_OK;
}

static UmiStatus UmiStudioDiscoveryCancelPending(UmiStudioTestService *service)
{
    if (service->discoveryJob == NULL) return UMI_STATUS_NOT_FOUND;
    service->discoveryStopRequested = 1;
    return UmiCtestDiscoveryJobCancel(service->discoveryJob);
}

/* Tool-aware publication also checks which CTest installation was selected; the inherited-PATH entry point remains available. The previous implementation is retained for engineering review. */
#if 0
UmiStatus UmiStudioTestDiscoveryPoll(UmiStudioTestService *service,
    const char *sourceRoot, const char *buildRoot, const char *configuration,
    uint64_t workspaceGeneration, int workspaceAllowed, int *outPublished,
    char *diagnostics, size_t capacity)
{
    UmiTestPlatformCtestImportSummary summary;
    int published = 0;
    UmiStatus status;
    const char *effectiveConfiguration = configuration != NULL && configuration[0] != '\0'
        ? configuration : "Debug";
    if (outPublished != NULL) *outPublished = 0;
    if (diagnostics != NULL && capacity != 0U) diagnostics[0] = '\0';
    if (service == NULL || (diagnostics == NULL && capacity != 0U))
        return UMI_STATUS_INVALID_ARGUMENT;
    if (service->discoveryJob == NULL) return UMI_STATUS_NOT_FOUND;
    if (!workspaceAllowed || sourceRoot == NULL || buildRoot == NULL ||
        strcmp(sourceRoot, service->discoverySourceRoot) != 0 ||
        strcmp(buildRoot, service->discoveryOptions.build_directory) != 0 ||
        strcmp(effectiveConfiguration, service->discoveryOptions.configuration) != 0 ||
        workspaceGeneration != service->discoveryGeneration) {
        (void)UmiStudioDiscoveryCancelPending(service);
    }
    status = UmiCtestDiscoveryJobPoll(service->discoveryJob, workspaceGeneration,
        &summary, &published, diagnostics, capacity);
    if (status == UMI_STATUS_BUSY) return status;
    /* A non-owner must not clear the owner-held pointer after a rejected Poll. */
    UmiStatus released = UmiCtestDiscoveryJobDestroy(service->discoveryJob);
    if (released != UMI_STATUS_OK) return released;
    service->discoveryJob = NULL;
    service->discoveryStopRequested = 0;
    if (published) {
        copy_text(service->build_directory, sizeof(service->build_directory),
            service->discoveryOptions.build_directory);
        copy_text(service->explorer.workspace_root, sizeof(service->explorer.workspace_root),
            service->discoverySourceRoot);
        copy_text(service->explorer.active_project_id, sizeof(service->explorer.active_project_id),
            service->discoveryOptions.project_id);
        copy_text(service->explorer.active_suite_id, sizeof(service->explorer.active_suite_id),
            service->discoveryOptions.suite_id);
        (void)memset(&service->last_summary, 0, sizeof(service->last_summary));
        service->explorer.revision += 1U;
        UmiStatus viewStatus = umi_test_workspace_refresh(service->workspace);
        if (diagnostics != NULL && capacity != 0U) {
            (void)snprintf(diagnostics, capacity,
                "Discovered %zu tests. %s", summary.discovered_count,
                viewStatus == UMI_STATUS_OK ? "Catalogue published." :
                viewStatus == UMI_STATUS_CAPACITY_EXCEEDED ?
                "Catalogue published; the existing selection/view capacity requires a narrower filter." :
                "Catalogue published, but view refresh failed. Inspect the view before running tests.");
        }
        if (outPublished != NULL) *outPublished = 1;
    }
    return status;
}
#endif
UmiStatus UmiStudioTestDiscoveryPoll(UmiStudioTestService *service,
    const char *sourceRoot, const char *buildRoot, const char *configuration,
    uint64_t workspaceGeneration, int workspaceAllowed, int *outPublished,
    char *diagnostics, size_t capacity)
{
    return UmiStudioTestDiscoveryPollWithToolDirectory(service,NULL,sourceRoot,
        buildRoot,configuration,workspaceGeneration,workspaceAllowed,outPublished,
        diagnostics,capacity);
}

UmiStatus UmiStudioTestDiscoveryPollWithToolDirectory(UmiStudioTestService *service,
    const char *directory, const char *sourceRoot, const char *buildRoot, const char *configuration,
    uint64_t workspaceGeneration, int workspaceAllowed, int *outPublished,
    char *diagnostics, size_t capacity)
{
    UmiTestPlatformCtestImportSummary summary;
    int published = 0;
    UmiStatus status;
    const char *effectiveConfiguration = configuration != NULL && configuration[0] != '\0'
        ? configuration : "Debug";
    if (outPublished != NULL) *outPublished = 0;
    if (diagnostics != NULL && capacity != 0U) diagnostics[0] = '\0';
    if (service == NULL || (diagnostics == NULL && capacity != 0U))
        return UMI_STATUS_INVALID_ARGUMENT;
    if (service->discoveryJob == NULL) return UMI_STATUS_NOT_FOUND;
    if (!workspaceAllowed ||
        strcmp(directory != NULL ? directory : "", service->discoveryToolDirectory) != 0 ||
        sourceRoot == NULL || buildRoot == NULL ||
        strcmp(sourceRoot, service->discoverySourceRoot) != 0 ||
        strcmp(buildRoot, service->discoveryOptions.build_directory) != 0 ||
        strcmp(effectiveConfiguration, service->discoveryOptions.configuration) != 0 ||
        workspaceGeneration != service->discoveryGeneration) {
        (void)UmiStudioDiscoveryCancelPending(service);
    }
    status = UmiCtestDiscoveryJobPoll(service->discoveryJob, workspaceGeneration,
        &summary, &published, diagnostics, capacity);
    if (status == UMI_STATUS_BUSY) return status;
    /* A non-owner must not clear the owner-held pointer after a rejected Poll. */
    UmiStatus released = UmiCtestDiscoveryJobDestroy(service->discoveryJob);
    if (released != UMI_STATUS_OK) return released;
    service->discoveryJob = NULL;
    service->discoveryStopRequested = 0;
    if (published) {
        copy_text(service->build_directory, sizeof(service->build_directory),
            service->discoveryOptions.build_directory);
        copy_text(service->explorer.workspace_root, sizeof(service->explorer.workspace_root),
            service->discoverySourceRoot);
        copy_text(service->explorer.active_project_id, sizeof(service->explorer.active_project_id),
            service->discoveryOptions.project_id);
        copy_text(service->explorer.active_suite_id, sizeof(service->explorer.active_suite_id),
            service->discoveryOptions.suite_id);
        (void)memset(&service->last_summary, 0, sizeof(service->last_summary));
        service->explorer.revision += 1U;
        UmiStatus viewStatus = umi_test_workspace_refresh(service->workspace);
        if (diagnostics != NULL && capacity != 0U) {
            (void)snprintf(diagnostics, capacity,
                "Discovered %zu tests. %s", summary.discovered_count,
                viewStatus == UMI_STATUS_OK ? "Catalogue published." :
                viewStatus == UMI_STATUS_CAPACITY_EXCEEDED ?
                "Catalogue published; the existing selection/view capacity requires a narrower filter." :
                "Catalogue published, but view refresh failed. Inspect the view before running tests.");
        }
        if (outPublished != NULL) *outPublished = 1;
    }
    return status;
}

/* Lifetime fallback for non-GUI owners: Stop and retain job payload until the
 * worker acknowledges completion. Interactive close requests are guarded before
 * reaching destruction. A stuck non-cooperating provider can still delay this
 * fallback; destroying its data while it is running would be unsafe. */
static int UmiStudioDiscoveryDispose(UmiStudioTestService *service)
{
    UmiStatus status;
    if (service->discoveryJob == NULL) return 1;
    if (UmiStudioDiscoveryCancelPending(service) != UMI_STATUS_OK) return 0;
    do {
        status = UmiCtestDiscoveryJobDestroy(service->discoveryJob);
        if (status == UMI_STATUS_BUSY) umi_thread_sleep_ms(1U);
    } while (status == UMI_STATUS_BUSY);
    if (status != UMI_STATUS_OK) return 0;
    service->discoveryJob = NULL;
    return 1;
}
