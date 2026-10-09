/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/test_archive_open.inc
 * PURPOSE: Publish a worker-validated archive only after the owner accepts its complete handles.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UmiStatus UmiStudioTestArchiveOpenBegin(UmiStudioTestService *service, UmiTaskQueue *queue,
                                        const char *path)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    UmiTestArchiveOpen *opening = NULL;
    status = UmiTestArchiveOpenCreate(path, "studio.tests", &opening);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveOpenSubmit(opening, queue);
    if (status != UMI_STATUS_OK)
    {
        (void)UmiTestArchiveOpenDestroy(opening);
        return status;
    }
    /* A rejected replacement leaves the prior request and attachment unchanged.
     * Only a successfully queued request supersedes an earlier completed one. */
    (void)UmiTestArchiveOpenDestroy(service->archiveOpening);
    service->archiveOpening = opening;
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveOpenCancel(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveOpening != NULL ? UmiTestArchiveOpenCancel(service->archiveOpening)
                                           : UMI_STATUS_NOT_FOUND;
}
/* A published replacement also retires the old terminal removal worker. The previous publication implementation is retained for review. The previous implementation is retained for engineering review. */
#if 0
UmiStatus UmiStudioTestArchiveOpenPublish(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (service->archiveOpening == NULL)
        return UMI_STATUS_NOT_FOUND;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    UmiTestArchiveOpenSnapshot state = {0};
    status = UmiTestArchiveOpenRead(service->archiveOpening, &state);
    if (status != UMI_STATUS_OK)
        return status;
    if (state.taken)
        return UMI_STATUS_ALREADY_EXISTS;
    if (!state.ready)
        return state.status;
    status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiDataServer *database = NULL;
    UmiTestArchive *archive = NULL;
    status = UmiTestArchiveOpenTake(service->archiveOpening, &database, &archive);
    if (status != UMI_STATUS_OK)
        return status;
    /* All workers are terminal. Retire their copied observations before closing
     * the database they borrowed, then expose the complete new ownership pair.
     * Historical rows never enter the live test registry through attachment. */
    (void)UmiTestArchiveReaderDestroy(service->archiveReader);
    service->archiveReader = NULL;
    (void)UmiTestArchiveReviewDestroy(service->archiveReview);
    service->archiveReview = NULL;
    UmiTestArchiveDestroy(service->archive);
    umi_data_server_destroy(service->archiveDatabase);
    service->archive = archive;
    service->archiveDatabase = database;
    memcpy(service->archivePath, state.path, sizeof(service->archivePath));
    service->archiveSavedTask = 0;
    memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    return UMI_STATUS_OK;
}
#endif
UmiStatus UmiStudioTestArchiveOpenPublish(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (service->archiveOpening == NULL)
        return UMI_STATUS_NOT_FOUND;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    UmiTestArchiveOpenSnapshot state = {0};
    status = UmiTestArchiveOpenRead(service->archiveOpening, &state);
    if (status != UMI_STATUS_OK)
        return status;
    if (state.taken)
        return UMI_STATUS_ALREADY_EXISTS;
    if (!state.ready)
        return state.status;
    status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiDataServer *database = NULL;
    UmiTestArchive *archive = NULL;
    status = UmiTestArchiveOpenTake(service->archiveOpening, &database, &archive);
    if (status != UMI_STATUS_OK)
        return status;
    /* All workers are terminal. Retire their copied observations before closing
     * the database they borrowed, then expose the complete new ownership pair.
     * Historical rows never enter the live test registry through attachment. */
    (void)UmiTestArchiveReaderDestroy(service->archiveReader);
    service->archiveReader = NULL;
    (void)UmiTestArchiveReviewDestroy(service->archiveReview);
    service->archiveReview = NULL;
    (void)UmiTestArchiveRemovalDestroy(service->archiveRemoval);
    service->archiveRemoval = NULL;
    UmiTestArchiveDestroy(service->archive);
    umi_data_server_destroy(service->archiveDatabase);
    service->archive = archive;
    service->archiveDatabase = database;
    memcpy(service->archivePath, state.path, sizeof(service->archivePath));
    service->archiveSavedTask = 0;
    memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    return UMI_STATUS_OK;
}
