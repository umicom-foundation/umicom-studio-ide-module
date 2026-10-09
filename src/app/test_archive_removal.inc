/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/test_archive_removal.inc
 * PURPOSE: Bind queued history removal to Studio attachment and duplicate-save ownership.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UmiStatus UmiStudioTestArchiveRemoveBegin(UmiStudioTestService *service, UmiTaskQueue *queue,
                                          uint64_t id)
{
    UmiStatus status = StudioArchiveReady(service);
    if (status != UMI_STATUS_OK)
        return status;
    /* Account for the previous committed removal before replacing its snapshot. */
    status = StudioArchiveReleaseWriter(service);
    if (status != UMI_STATUS_OK)
        return status;
    UmiTestArchiveRemoval *removal = NULL;
    status = UmiTestArchiveRemovalCreate(service->archive, id, &removal);
    if (status == UMI_STATUS_OK)
        status = UmiTestArchiveRemovalSubmit(removal, queue);
    if (status != UMI_STATUS_OK)
    {
        (void)UmiTestArchiveRemovalDestroy(removal);
        return status;
    }
    (void)UmiTestArchiveRemovalDestroy(service->archiveRemoval);
    service->archiveRemoval = removal;
    return UMI_STATUS_OK;
}
UmiStatus UmiStudioTestArchiveRemoveCancel(UmiStudioTestService *service)
{
    UmiStatus status = StudioArchiveOwner(service);
    if (status != UMI_STATUS_OK)
        return status;
    return service->archiveRemoval != NULL ? UmiTestArchiveRemovalCancel(service->archiveRemoval)
                                           : UMI_STATUS_NOT_FOUND;
}
