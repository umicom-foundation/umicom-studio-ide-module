/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/test_archive_reader.inc
 * PURPOSE: Bind copied archive observations to Studio service ownership.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UmiStatus UmiStudioTestArchiveBrowse(UmiStudioTestService *service, UmiTaskQueue *queue,
                                     UmiTestArchiveReadKind kind, uint64_t id, size_t index)
{
    UmiStatus status = StudioArchiveReady(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiTestArchiveReader *reader = NULL;
    status = UmiTestArchiveReaderCreate(service->archive, kind, id, index, &reader);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveReaderSubmit(reader, queue);
    if (status != UMI_STATUS_OK)
    {
        (void)UmiTestArchiveReaderDestroy(reader);
        return status;
    }
    /* Keep the previous copied observation when allocation or queue submission
     * fails. After acceptance, only this reader belongs to the current request. */
    (void)UmiTestArchiveReaderDestroy(service->archiveReader);
    service->archiveReader = reader;
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveReadCancel(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveReader != NULL ? UmiTestArchiveReaderCancel(service->archiveReader)
                                          : UMI_STATUS_NOT_FOUND;
}
UmiStatus UmiStudioTestArchiveCopyCatalog(UmiStudioTestService *service, UmiTestArchiveCatalog *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveReader != NULL ? UmiTestArchiveReaderCatalog(service->archiveReader, out)
                                          : UMI_STATUS_NOT_FOUND;
}
UmiStatus UmiStudioTestArchiveCopyAttempt(UmiStudioTestService *service, UmiTestArchiveAttempt *out)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveReader != NULL ? UmiTestArchiveReaderAttempt(service->archiveReader, out)
                                          : UMI_STATUS_NOT_FOUND;
}
