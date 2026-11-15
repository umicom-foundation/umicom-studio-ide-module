/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/services_lifecycle.inc
 *
 * PURPOSE:
 *   Preserve the shared service graph when a background owner cannot stop.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Cancel the Framework review before stopping its shared queue; retain the service graph on any refused shutdown. The previous implementation remains for review. */
#if 0
UmiStatus UmiStudioServicesStopBackground(UmiStudioServices *services)
{
    UmiStatus status;
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Refuse self-destruction before stopping other service owners. The worker
     * identity belongs to Framework, so Studio does not inspect private threads. */
    if (UmiTaskQueueIsCurrentWorker(services->task_queue))
        return UMI_STATUS_INVALID_STATE;
    if (services->watcher != NULL) {
        status = umi_watcher_stop(services->watcher);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->process_supervisor != NULL) {
        status = umi_process_supervisor_shutdown(services->process_supervisor);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->task_queue != NULL) {
        /* Keep the existing policy: cancel pending work and join running work.
         * This checked boundary does not silently force or cancel callbacks. */
        status = umi_task_queue_shutdown(services->task_queue, 1);
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}
#endif
/* Direct background-stop requests also honor active AI patch ownership before cancelling or joining unrelated work. The former implementation is retained for owner review. */
#if 0
UmiStatus UmiStudioServicesStopBackground(UmiStudioServices *services)
{
    UmiStatus status;
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Refuse self-destruction before stopping other service owners. The worker
     * identity belongs to Framework, so Studio does not inspect private threads. */
    if (UmiTaskQueueIsCurrentWorker(services->task_queue))
        return UMI_STATUS_INVALID_STATE;
    /* Suppress pending report publication and request cooperative cancellation
     * before joining the queue. Never release review storage while it can run. */
    if (services->codeguard_review != NULL) {
        status = UmiStudioCodeGuardCancel(services);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->watcher != NULL) {
        status = umi_watcher_stop(services->watcher);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->process_supervisor != NULL) {
        status = umi_process_supervisor_shutdown(services->process_supervisor);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->task_queue != NULL) {
        /* Keep the existing policy: cancel pending work and join running work.
         * This checked boundary does not silently force or cancel callbacks. */
        status = umi_task_queue_shutdown(services->task_queue, 1);
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}

#endif
UmiStatus UmiStudioServicesStopBackground(UmiStudioServices *services)
{
    UmiStatus status;
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Refuse self-destruction before stopping other service owners. The worker
     * identity belongs to Framework, so Studio does not inspect private threads. */
    if (UmiTaskQueueIsCurrentWorker(services->task_queue))
        return UMI_STATUS_INVALID_STATE;
    status = UmiStudioAiPlatformCheckIdle(services->ai_platform);
    if (status != UMI_STATUS_OK) return status;
    /* Suppress pending report publication and request cooperative cancellation
     * before joining the queue. Never release review storage while it can run. */
    if (services->codeguard_review != NULL) {
        status = UmiStudioCodeGuardCancel(services);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->watcher != NULL) {
        status = umi_watcher_stop(services->watcher);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->process_supervisor != NULL) {
        status = umi_process_supervisor_shutdown(services->process_supervisor);
        if (status != UMI_STATUS_OK) return status;
    }
    if (services->task_queue != NULL) {
        /* Keep the existing policy: cancel pending work and join running work.
         * This checked boundary does not silently force or cancel callbacks. */
        status = umi_task_queue_shutdown(services->task_queue, 1);
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}

/* Guard aggregate release reentry while child/provider destructors run. On refusal the still-live owner is unlocked for retry; success clears only the caller handle after the owner was freed. The former implementation is retained for owner review. */
#if 0
UmiStatus UmiStudioServicesRelease(UmiStudioServices **inOutServices)
{
    if (inOutServices == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioServices *services = *inOutServices;
    if (services == NULL) return UMI_STATUS_OK;
    UmiStatus status = destroy_partial_checked(services);
    if (status == UMI_STATUS_OK) *inOutServices = NULL;
    return status;
}

#endif
UmiStatus UmiStudioServicesRelease(UmiStudioServices **inOutServices)
{
    if (inOutServices == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioServices *services = *inOutServices;
    if (services == NULL) return UMI_STATUS_OK;
    if (services->release_in_progress) return UMI_STATUS_BUSY;
    services->release_in_progress = 1;
    UmiStatus status = destroy_partial_checked(services);
    if (status == UMI_STATUS_OK) *inOutServices = NULL;
    else services->release_in_progress = 0;
    return status;
}

/* Construction rollback and the public void destroy retain their existing
 * entry points. Failure retains memory rather than freeing live callback data.
 * Lifecycle owners which need the result use UmiStudioServicesRelease. */
static void destroy_partial(UmiStudioServices *services)
{
    (void)UmiStudioServicesRelease(&services);
}
