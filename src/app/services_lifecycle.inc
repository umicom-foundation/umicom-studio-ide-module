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

UmiStatus UmiStudioServicesRelease(UmiStudioServices **inOutServices)
{
    if (inOutServices == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioServices *services = *inOutServices;
    if (services == NULL) return UMI_STATUS_OK;
    UmiStatus status = destroy_partial_checked(services);
    if (status == UMI_STATUS_OK) *inOutServices = NULL;
    return status;
}

/* Construction rollback and the public void destroy retain their existing
 * entry points. Failure retains memory rather than freeing live callback data.
 * Lifecycle owners which need the result use UmiStudioServicesRelease. */
static void destroy_partial(UmiStudioServices *services)
{
    (void)UmiStudioServicesRelease(&services);
}
