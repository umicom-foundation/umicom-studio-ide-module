/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/test_archive.inc
 * PURPOSE: Bind private archives to accepted test jobs without granting execution authority.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static UmiStatus StudioArchiveOwner(const UmiStudioTestService *service)
{
    if (service == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    return service->archiveOwner == umi_thread_current_id() ? UMI_STATUS_OK : UMI_STATUS_INVALID_STATE;
}
/* Archive comparisons now share the database lifetime guard with saves. The save-only guard is retained for review.
 * The previous implementation remains here for engineering review. */
#if 0
int UmiStudioTestArchivePending(const UmiStudioTestService *service)
{
    if (service == NULL || service->archiveWriter == NULL)
        return 0;
    UmiTestArchiveWriteSnapshot state = {0};
    if (UmiTestArchiveWriteRead(service->archiveWriter, &state) != UMI_STATUS_OK)
        return 1;
    return state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING;
}
#endif
/* Removal now borrows the attached database alongside reads and writes. Retain the earlier lifetime guard for review. The previous implementation is retained for engineering review. */
#if 0
int UmiStudioTestArchivePending(const UmiStudioTestService *service)
{
    if (service == NULL)
        return 0;
    /* Both operations borrow the same database. Keep it attached until every
     * callback has stopped; controls may poll this without touching SQLite. */
    /* Opening owns a separate candidate database until owner-thread publication.
     * Keep archive actions serial while its worker still touches that storage. */
    if (service->archiveOpening != NULL)
    {
        UmiTestArchiveOpenSnapshot state = {0};
        if (UmiTestArchiveOpenRead(service->archiveOpening, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveWriter != NULL)
    {
        UmiTestArchiveWriteSnapshot state = {0};
        if (UmiTestArchiveWriteRead(service->archiveWriter, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveReview != NULL)
    {
        UmiTestArchiveReviewSnapshot state = {0};
        if (UmiTestArchiveReviewRead(service->archiveReview, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveReader != NULL)
    {
        UmiTestArchiveReaderSnapshot state = {0};
        if (UmiTestArchiveReaderRead(service->archiveReader, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    return 0;
}
#endif
int UmiStudioTestArchivePending(const UmiStudioTestService *service)
{
    if (service == NULL)
        return 0;
    /* Both operations borrow the same database. Keep it attached until every
     * callback has stopped; controls may poll this without touching SQLite. */
    /* Opening owns a separate candidate database until owner-thread publication.
     * Keep archive actions serial while its worker still touches that storage. */
    if (service->archiveOpening != NULL)
    {
        UmiTestArchiveOpenSnapshot state = {0};
        if (UmiTestArchiveOpenRead(service->archiveOpening, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveWriter != NULL)
    {
        UmiTestArchiveWriteSnapshot state = {0};
        if (UmiTestArchiveWriteRead(service->archiveWriter, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveReview != NULL)
    {
        UmiTestArchiveReviewSnapshot state = {0};
        if (UmiTestArchiveReviewRead(service->archiveReview, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveReader != NULL)
    {
        UmiTestArchiveReaderSnapshot state = {0};
        if (UmiTestArchiveReaderRead(service->archiveReader, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    if (service->archiveRemoval != NULL)
    {
        UmiTestArchiveRemovalSnapshot state = {0};
        if (UmiTestArchiveRemovalRead(service->archiveRemoval, &state) != UMI_STATUS_OK)
            return 1;
        if (state.state == UMI_TASK_QUEUED || state.state == UMI_TASK_RUNNING)
            return 1;
    }
    return 0;
}
/* Release only a completed worker, preserving its final outcome for the UI.
 * Remember committed IDs even when a cancellation arrived after commit. */

/* A successfully removed latest save can be saved again. Observe the committed
 * outcome, never the task's cancellation label, before retiring duplicate-save
 * bookkeeping. The source job and its live results remain unchanged. */
static void StudioArchiveAccountRemoval(UmiStudioTestService *service)
{
    UmiTestArchiveRemovalSnapshot state = {0};
    if (service->archiveRemoval != NULL &&
        UmiTestArchiveRemovalRead(service->archiveRemoval, &state) == UMI_STATUS_OK &&
        state.committed && service->archiveLastWrite.saved.id == state.id)
    {
        service->archiveSavedTask = 0;
        memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    }
}
/* Committed removal retires duplicate-save bookkeeping before another save. The former writer-only accounting is retained for review. The previous implementation is retained for engineering review. */
#if 0
static UmiStatus StudioArchiveReleaseWriter(UmiStudioTestService *service)
{
    if (service->archiveWriter == NULL)
        return UMI_STATUS_OK;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    UmiTestArchiveWriteSnapshot state = {0};
    UmiStatus status = UmiTestArchiveWriteRead(service->archiveWriter, &state);
    if (status != UMI_STATUS_OK)
        return status;
    status = UmiTestArchiveWriteDestroy(service->archiveWriter);
    if (status != UMI_STATUS_OK)
        return status;
    service->archiveWriter = NULL;
    service->archiveLastWrite = state;
    if (state.saved.id != 0)
        service->archiveSavedTask = state.saved.run.task_id;
    return UMI_STATUS_OK;
}
#endif
static UmiStatus StudioArchiveReleaseWriter(UmiStudioTestService *service)
{
    if (service->archiveWriter == NULL)
    {
        StudioArchiveAccountRemoval(service);
        return UMI_STATUS_OK;
    }
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    UmiTestArchiveWriteSnapshot state = {0};
    UmiStatus status = UmiTestArchiveWriteRead(service->archiveWriter, &state);
    if (status != UMI_STATUS_OK)
        return status;
    status = UmiTestArchiveWriteDestroy(service->archiveWriter);
    if (status != UMI_STATUS_OK)
        return status;
    service->archiveWriter = NULL;
    service->archiveLastWrite = state;
    if (state.saved.id != 0)
        service->archiveSavedTask = state.saved.run.task_id;
    StudioArchiveAccountRemoval(service);
    return UMI_STATUS_OK;
}
/* Shutdown must wait for every borrower, including queued removal, before releasing storage. Retain the earlier shutdown implementation for review. The previous implementation is retained for engineering review. */
#if 0
static int StudioArchiveDispose(UmiStudioTestService *service)
{
    /* Shutdown drains every owned worker before freeing its payload. Opening
     * can own an unpublished database independently of the current attachment. */
    if (service->archiveOpening != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveOpenCancel(service->archiveOpening);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveOpenDestroy(service->archiveOpening);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveOpening = NULL;
    }
    /* Partial service construction has no archive owner or payload to drain. */
    /* The reader borrows the archive just like a writer. Drain it before either
     * the archive or its Data Server can be released during service shutdown. */
    if (service->archiveReader != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveReaderCancel(service->archiveReader);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveReaderDestroy(service->archiveReader);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveReader = NULL;
    }
    if (service->archiveReview != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveReviewCancel(service->archiveReview);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveReviewDestroy(service->archiveReview);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveReview = NULL;
    }
    if (service->archiveWriter == NULL && service->archiveDatabase == NULL)
        return 1;
    if (StudioArchiveOwner(service) != UMI_STATUS_OK)
        return 0;
    if (service->archiveWriter != NULL)
    {
        (void)UmiTestArchiveWriteCancel(service->archiveWriter);
        UmiStatus status;
        do
        {
            status = StudioArchiveReleaseWriter(service);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
    }
    UmiTestArchiveDestroy(service->archive);
    umi_data_server_destroy(service->archiveDatabase);
    service->archive = NULL;
    service->archiveDatabase = NULL;
    return 1;
}
#endif
static int StudioArchiveDispose(UmiStudioTestService *service)
{
    /* Drain removal before releasing the archive it borrows. Teardown follows
     * the service's existing shutdown contract; normal UI Stop never waits. */
    if (service->archiveRemoval != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveRemovalCancel(service->archiveRemoval);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveRemovalDestroy(service->archiveRemoval);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveRemoval = NULL;
    }
    /* Shutdown drains every owned worker before freeing its payload. Opening
     * can own an unpublished database independently of the current attachment. */
    if (service->archiveOpening != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveOpenCancel(service->archiveOpening);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveOpenDestroy(service->archiveOpening);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveOpening = NULL;
    }
    /* Partial service construction has no archive owner or payload to drain. */
    /* The reader borrows the archive just like a writer. Drain it before either
     * the archive or its Data Server can be released during service shutdown. */
    if (service->archiveReader != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveReaderCancel(service->archiveReader);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveReaderDestroy(service->archiveReader);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveReader = NULL;
    }
    if (service->archiveReview != NULL)
    {
        if (StudioArchiveOwner(service) != UMI_STATUS_OK)
            return 0;
        (void)UmiTestArchiveReviewCancel(service->archiveReview);
        UmiStatus status;
        do
        {
            status = UmiTestArchiveReviewDestroy(service->archiveReview);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
        service->archiveReview = NULL;
    }
    if (service->archiveWriter == NULL && service->archiveDatabase == NULL)
        return 1;
    if (StudioArchiveOwner(service) != UMI_STATUS_OK)
        return 0;
    if (service->archiveWriter != NULL)
    {
        (void)UmiTestArchiveWriteCancel(service->archiveWriter);
        UmiStatus status;
        do
        {
            status = StudioArchiveReleaseWriter(service);
            if (status == UMI_STATUS_BUSY)
                umi_thread_sleep_ms(1U);
        } while (status == UMI_STATUS_BUSY);
        if (status != UMI_STATUS_OK)
            return 0;
    }
    UmiTestArchiveDestroy(service->archive);
    umi_data_server_destroy(service->archiveDatabase);
    service->archive = NULL;
    service->archiveDatabase = NULL;
    return 1;
}
/* Successful attachment replacement retires the old completed removal outcome with its database. The former attachment code remains for review. The previous implementation is retained for engineering review. */
#if 0
UmiStatus UmiStudioTestArchiveOpen(UmiStudioTestService *service, const char *path)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    char selected[UMI_PATH_CAPACITY] = {0};
    UmiDataServer *database = NULL;
    UmiTestArchive *archive = NULL;
    if (path != NULL)
    {
        status = UmiOutputFileValidatePath(path);
        if (status != UMI_STATUS_OK)
            return status;
        if (strlen(path) >= sizeof(selected))
            return UMI_STATUS_CAPACITY_EXCEEDED;
        memcpy(selected, path, strlen(path) + 1U);
        status = umi_data_server_create_sqlite(selected, &database);
        if (status == UMI_STATUS_OK)
            status = UmiTestArchiveCreate(database, "studio.tests", &archive);
        UmiTestArchiveCatalog *catalog = NULL;
        if (status == UMI_STATUS_OK)
        {
            catalog = calloc(1U, sizeof(*catalog));
            status = catalog != NULL ? UmiTestArchiveList(archive, catalog) : UMI_STATUS_OUT_OF_MEMORY;
        }
        free(catalog);
    }
    if (status == UMI_STATUS_OK)
        status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
    {
        UmiTestArchiveDestroy(archive);
        umi_data_server_destroy(database);
        return status;
    }
    /* The synchronous API remains available to headless callers. An accepted
     * synchronous replacement retires any earlier completed opening request. */
    (void)UmiTestArchiveOpenDestroy(service->archiveOpening);
    service->archiveOpening = NULL;
    /* A replacement is published only after its metadata was read successfully.
     * Failure leaves the previously attached database and controls usable. */
    (void)UmiTestArchiveReaderDestroy(service->archiveReader);
    service->archiveReader = NULL;
    (void)UmiTestArchiveReviewDestroy(service->archiveReview);
    service->archiveReview = NULL;
    UmiTestArchiveDestroy(service->archive);
    umi_data_server_destroy(service->archiveDatabase);
    service->archive = archive;
    service->archiveDatabase = database;
    memcpy(service->archivePath, selected, sizeof(selected));
    service->archiveSavedTask = 0;
    memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    return UMI_STATUS_OK;
}
#endif
UmiStatus UmiStudioTestArchiveOpen(UmiStudioTestService *service, const char *path)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    char selected[UMI_PATH_CAPACITY] = {0};
    UmiDataServer *database = NULL;
    UmiTestArchive *archive = NULL;
    if (path != NULL)
    {
        status = UmiOutputFileValidatePath(path);
        if (status != UMI_STATUS_OK)
            return status;
        if (strlen(path) >= sizeof(selected))
            return UMI_STATUS_CAPACITY_EXCEEDED;
        memcpy(selected, path, strlen(path) + 1U);
        status = umi_data_server_create_sqlite(selected, &database);
        if (status == UMI_STATUS_OK)
            status = UmiTestArchiveCreate(database, "studio.tests", &archive);
        UmiTestArchiveCatalog *catalog = NULL;
        if (status == UMI_STATUS_OK)
        {
            catalog = calloc(1U, sizeof(*catalog));
            status = catalog != NULL ? UmiTestArchiveList(archive, catalog) : UMI_STATUS_OUT_OF_MEMORY;
        }
        free(catalog);
    }
    if (status == UMI_STATUS_OK)
        status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
    {
        UmiTestArchiveDestroy(archive);
        umi_data_server_destroy(database);
        return status;
    }
    /* The synchronous API remains available to headless callers. An accepted
     * synchronous replacement retires any earlier completed opening request. */
    (void)UmiTestArchiveOpenDestroy(service->archiveOpening);
    service->archiveOpening = NULL;
    /* A replacement is published only after its metadata was read successfully.
     * Failure leaves the previously attached database and controls usable. */
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
    memcpy(service->archivePath, selected, sizeof(selected));
    service->archiveSavedTask = 0;
    memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveSave(UmiStudioTestService *service, UmiTaskQueue *queue, bool retain_output)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (queue == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    if (service->archive == NULL)
        return UMI_STATUS_INVALID_STATE;
    if (UmiStudioTestArchivePending(service) || service->executionPending || service->executionQueue != NULL)
        return UMI_STATUS_BUSY;
    if (service->executionJob == NULL)
        return UMI_STATUS_NOT_FOUND;
    status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiCtestJobSnapshot run = {0};
    status = UmiCtestJobGetSnapshot(service->executionJob, &run);
    if (status != UMI_STATUS_OK)
        return status;
    if (service->archiveSavedTask == run.task_id)
        return UMI_STATUS_ALREADY_EXISTS;
    UmiTestArchiveOrigin origin = {0};
    memcpy(origin.source_root, service->executionRecordedContext.source_root, sizeof(origin.source_root));
    origin.workspace_generation = service->executionRecordedContext.workspace_generation;
    origin.retain_output = retain_output;
    /* Use the accepted run's copy, even after another workspace or build profile
     * is selected. Clicking Save must not relabel old results with new settings. */
    origin.identity = service->executionRecordedContext.identity;
    UmiTestArchiveWrite *writer = NULL;
    status = UmiTestArchiveWriteCreate(service->archive, service->executionJob, &origin, &writer);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveWriteSubmit(writer, queue);
    if (status != UMI_STATUS_OK)
    {
        (void)UmiTestArchiveWriteDestroy(writer);
        return status;
    }
    service->archiveWriter = writer;
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveCancel(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveWriter != NULL ? UmiTestArchiveWriteCancel(service->archiveWriter)
                                          : UMI_STATUS_NOT_FOUND;
}
/* State reads now expose comparison progress as well as saves. The previous save-only snapshot is retained for review.
 * The previous implementation remains here for engineering review. */
#if 0
UmiStatus UmiStudioTestArchiveStateRead(UmiStudioTestService *service, UmiStudioTestArchiveState *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (out == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioTestArchiveState state = {0};
    memcpy(state.path, service->archivePath, sizeof(state.path));
    state.attached = service->archive != NULL;
    state.write = service->archiveLastWrite;
    if (service->archiveWriter != NULL)
    {
        status = UmiTestArchiveWriteRead(service->archiveWriter, &state.write);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.write.state == UMI_TASK_QUEUED || state.write.state == UMI_TASK_RUNNING;
    }
    *out = state;
    return UMI_STATUS_OK;
}
#endif
/* Progress polling includes removal outcomes using memory-only reads. The earlier snapshot remains for engineering review. The previous implementation is retained for engineering review. */
#if 0
UmiStatus UmiStudioTestArchiveStateRead(UmiStudioTestService *service, UmiStudioTestArchiveState *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (out == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioTestArchiveState state = {0};
    memcpy(state.path, service->archivePath, sizeof(state.path));
    state.attached = service->archive != NULL;
    state.write = service->archiveLastWrite;
    if (service->archiveWriter != NULL)
    {
        status = UmiTestArchiveWriteRead(service->archiveWriter, &state.write);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.write.state == UMI_TASK_QUEUED || state.write.state == UMI_TASK_RUNNING;
    }
    if (service->archiveReview != NULL)
    {
        status = UmiTestArchiveReviewRead(service->archiveReview, &state.comparison);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.comparison.state == UMI_TASK_QUEUED ||
                        state.comparison.state == UMI_TASK_RUNNING;
    }
    if (service->archiveReader != NULL)
    {
        status = UmiTestArchiveReaderRead(service->archiveReader, &state.reader);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.reader.state == UMI_TASK_QUEUED ||
                        state.reader.state == UMI_TASK_RUNNING;
    }
    if (service->archiveOpening != NULL)
    {
        status = UmiTestArchiveOpenRead(service->archiveOpening, &state.opening);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.opening.state == UMI_TASK_QUEUED ||
                        state.opening.state == UMI_TASK_RUNNING;
    }
    *out = state;
    return UMI_STATUS_OK;
}
#endif
UmiStatus UmiStudioTestArchiveStateRead(UmiStudioTestService *service, UmiStudioTestArchiveState *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (out == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioTestArchiveState state = {0};
    memcpy(state.path, service->archivePath, sizeof(state.path));
    state.attached = service->archive != NULL;
    state.write = service->archiveLastWrite;
    if (service->archiveWriter != NULL)
    {
        status = UmiTestArchiveWriteRead(service->archiveWriter, &state.write);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.write.state == UMI_TASK_QUEUED || state.write.state == UMI_TASK_RUNNING;
    }
    if (service->archiveReview != NULL)
    {
        status = UmiTestArchiveReviewRead(service->archiveReview, &state.comparison);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.comparison.state == UMI_TASK_QUEUED ||
                        state.comparison.state == UMI_TASK_RUNNING;
    }
    if (service->archiveReader != NULL)
    {
        status = UmiTestArchiveReaderRead(service->archiveReader, &state.reader);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.reader.state == UMI_TASK_QUEUED ||
                        state.reader.state == UMI_TASK_RUNNING;
    }
    if (service->archiveOpening != NULL)
    {
        status = UmiTestArchiveOpenRead(service->archiveOpening, &state.opening);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.opening.state == UMI_TASK_QUEUED ||
                        state.opening.state == UMI_TASK_RUNNING;
    }
    if (service->archiveRemoval != NULL)
    {
        status = UmiTestArchiveRemovalRead(service->archiveRemoval, &state.removal);
        if (status != UMI_STATUS_OK)
            return status;
        state.pending = state.pending || state.removal.state == UMI_TASK_QUEUED ||
                        state.removal.state == UMI_TASK_RUNNING;
    }
    *out = state;
    return UMI_STATUS_OK;
}
static UmiStatus StudioArchiveReady(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    if (UmiStudioTestArchivePending(service))
        return UMI_STATUS_BUSY;
    return service->archive != NULL ? UMI_STATUS_OK : UMI_STATUS_INVALID_STATE;
}
UmiStatus UmiStudioTestArchiveList(UmiStudioTestService *service, UmiTestArchiveCatalog *output)
{
    UmiStatus status = StudioArchiveReady(service);
    return status == UMI_STATUS_OK ? UmiTestArchiveList(service->archive, output) : status;
}
UmiStatus UmiStudioTestArchiveRead(UmiStudioTestService *service, uint64_t id, UmiTestArchiveEntry *output)
{
    UmiStatus status = StudioArchiveReady(service);
    return status == UMI_STATUS_OK ? UmiTestArchiveRead(service->archive, id, output) : status;
}
UmiStatus UmiStudioTestArchiveRequestAt(UmiStudioTestService *service, uint64_t id, size_t index,
                                        UmiCtestJobRequest *output)
{
    UmiStatus status = StudioArchiveReady(service);
    return status == UMI_STATUS_OK ? UmiTestArchiveRequestAt(service->archive, id, index, output) : status;
}
UmiStatus UmiStudioTestArchiveResultAt(UmiStudioTestService *service, uint64_t id, size_t index,
                                       UmiTestResult *output, uint32_t *attempt)
{
    UmiStatus status = StudioArchiveReady(service);
    return status == UMI_STATUS_OK ? UmiTestArchiveResultAt(service->archive, id, index, output, attempt)
                                   : status;
}
UmiStatus UmiStudioTestArchiveRemove(UmiStudioTestService *service, uint64_t id)
{
    UmiStatus status = StudioArchiveReady(service);
    if (status == UMI_STATUS_OK)
        status = StudioArchiveReleaseWriter(service);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveRemove(service->archive, id);
    if (status == UMI_STATUS_OK && service->archiveLastWrite.saved.id == id)
    {
        service->archiveSavedTask = 0;
        memset(&service->archiveLastWrite, 0, sizeof(service->archiveLastWrite));
    }
    return status;
}

UmiStatus UmiStudioTestArchiveCompare(UmiStudioTestService *service, UmiTaskQueue *queue,
                                      uint64_t baseline_id, uint64_t candidate_id)
{
    UmiStatus status = StudioArchiveReady(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiTestArchiveReview *review = NULL;
    status = UmiTestArchiveReviewCreate(service->archive, baseline_id, candidate_id, &review);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveReviewSubmit(review, queue);
    if (status != UMI_STATUS_OK)
    {
        (void)UmiTestArchiveReviewDestroy(review);
        return status;
    }
    /* The old comparison stays readable if creation or submission fails.
     * Successful submission owns a fresh immutable result and replaces it. */
    (void)UmiTestArchiveReviewDestroy(service->archiveReview);
    service->archiveReview = review;
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveComparisonCancel(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveReview != NULL ? UmiTestArchiveReviewCancel(service->archiveReview)
                                          : UMI_STATUS_NOT_FOUND;
}
UmiStatus UmiStudioTestArchiveComparisonRowAt(UmiStudioTestService *service, size_t index,
                                              UmiTestArchiveComparisonRow *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveReview != NULL ? UmiTestArchiveReviewRowAt(service->archiveReview, index, out)
                                          : UMI_STATUS_NOT_FOUND;
}

/* Browsing shares the same archive ownership guard as saving and comparison. */
#include "test_archive_reader.inc"

/* Asynchronous attachment uses the same owner thread and archive lifetime rules. */
#include "test_archive_open.inc"

/* Framework owns the transaction; Studio binds it to the attached history. */
#include "test_archive_removal.inc"
