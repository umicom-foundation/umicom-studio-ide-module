/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/test_archive.h
 * PURPOSE: Offer explicit private test-history actions using Framework storage.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_TEST_ARCHIVE_H
#define UMICOM_STUDIO_TEST_ARCHIVE_H
#include "umicom/testing/archive_write.h"
#include "umicom/testing/archive_review.h"
#include "umicom/testing/archive_reader.h"
#include "umicom/testing/archive_open.h"
#include "umicom/testing/archive_removal.h"
#include "umicom/platform/path.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct UmiStudioTestService UmiStudioTestService;
    typedef struct UmiStudioTestArchiveState
    {
        char path[UMI_PATH_CAPACITY];
        bool attached;
        bool pending;
        UmiTestArchiveWriteSnapshot write;
        UmiTestArchiveReviewSnapshot comparison;
        UmiTestArchiveReaderSnapshot reader;
        UmiTestArchiveOpenSnapshot opening;
        UmiTestArchiveRemovalSnapshot removal;
    } UmiStudioTestArchiveState;
    /* All methods belong to the service's creating thread. Open validates an
 * absolute local filename and the catalogue before replacing an attachment.
 * NULL detaches without deleting the database. No location or credentials are
 * persisted in project files. Choose a private directory with suitable access.
 * Opening or reading an archive never executes tests or changes their status. */
    UmiStatus UmiStudioTestArchiveOpen(UmiStudioTestService *service, const char *path);
    /** Open and validate a replacement archive on the shared worker queue.
     * The existing attachment remains usable if opening fails. The selected
     * path is copied now; editing the path field cannot redirect pending work.
     * No attachment changes until OpenPublish is called by the owner thread. */
    UmiStatus UmiStudioTestArchiveOpenBegin(UmiStudioTestService *service,
        UmiTaskQueue *queue, const char *path);
    /** Request Stop, including after worker completion but before publication.
     * The old attachment remains selected; files created while opening remain
     * on disk. A completed, already published attachment is not revoked. */
    UmiStatus UmiStudioTestArchiveOpenCancel(UmiStudioTestService *service);
    /** Publish an opened replacement on the owner thread after workers finish.
     * BUSY leaves both attachments intact. Success retires the previous archive,
     * which may close a SQLite handle; it does not open or read the new file. */
    UmiStatus UmiStudioTestArchiveOpenPublish(UmiStudioTestService *service);
    /* Save the most recently accepted queued run after all results are published.
 * The original accepted context is captured, not the workspace open now.
 * Duplicate saves of that run in the same attachment return ALREADY_EXISTS.
 * Output retention is explicit; false keeps outcomes and selection only. */
    UmiStatus UmiStudioTestArchiveSave(UmiStudioTestService *service, UmiTaskQueue *queue,
                                       bool retain_output);
    int UmiStudioTestArchivePending(const UmiStudioTestService *service);
    UmiStatus UmiStudioTestArchiveCancel(UmiStudioTestService *service);
    /* State reads copy memory only; they remain safe for a periodic UI refresh.
 * The catalogue/result/remove methods perform bounded database operations on
 * explicit user actions. Their errors leave output objects unchanged. */
    UmiStatus UmiStudioTestArchiveStateRead(UmiStudioTestService *service,
                                            UmiStudioTestArchiveState *out_state);
    UmiStatus UmiStudioTestArchiveList(UmiStudioTestService *service, UmiTestArchiveCatalog *out_catalog);
    UmiStatus UmiStudioTestArchiveRead(UmiStudioTestService *service, uint64_t id,
                                       UmiTestArchiveEntry *out_entry);
    UmiStatus UmiStudioTestArchiveResultAt(UmiStudioTestService *service, uint64_t id, size_t index,
                                           UmiTestResult *out_result, uint32_t *out_attempt);
    UmiStatus UmiStudioTestArchiveRequestAt(UmiStudioTestService *service, uint64_t id, size_t index,
                                            UmiCtestJobRequest *out_request);
    /* Comparison runs on the supplied queue. Its results are independent of
     * current discovery; it neither grants workspace trust nor starts tests.
     * A pending archive save/comparison blocks replacement and other DB actions. */
    UmiStatus UmiStudioTestArchiveCompare(UmiStudioTestService *service, UmiTaskQueue *queue,
                                          uint64_t baseline_id, uint64_t candidate_id);
    UmiStatus UmiStudioTestArchiveComparisonCancel(UmiStudioTestService *service);
    UmiStatus UmiStudioTestArchiveComparisonRowAt(UmiStudioTestService *service, size_t index,
                                                  UmiTestArchiveComparisonRow *out_row);
    /** Start a catalogue or attempt read on the shared queue. The copied request
     * remains tied to this attachment until completion. Replacing a pending
     * reader or detaching its archive returns BUSY. */
    UmiStatus UmiStudioTestArchiveBrowse(UmiStudioTestService *service, UmiTaskQueue *queue,
        UmiTestArchiveReadKind kind, uint64_t id, size_t index);
    /** Request Stop for the current read without waiting for storage. */
    UmiStatus UmiStudioTestArchiveReadCancel(UmiStudioTestService *service);
    /** Copy the completed catalogue from memory; errors preserve the output.
     * This does not perform storage I/O and is safe for UI completion handling. */
    UmiStatus UmiStudioTestArchiveCopyCatalog(UmiStudioTestService *service,
        UmiTestArchiveCatalog *out_catalog);
    /** Copy one complete historical attempt from memory. Missing results remain
     * NOT_FOUND; no historical row is inserted into current test results. */
    UmiStatus UmiStudioTestArchiveCopyAttempt(UmiStudioTestService *service,
        UmiTestArchiveAttempt *out_attempt);
    UmiStatus UmiStudioTestArchiveRemove(UmiStudioTestService *service, uint64_t id);
    /** Queue removal of the copied run ID from the currently attached archive.
     * Another archive operation or detach returns BUSY until this worker ends.
     * Failed submission preserves the previous outcome and removes nothing. */
    UmiStatus UmiStudioTestArchiveRemoveBegin(UmiStudioTestService *service,
        UmiTaskQueue *queue, uint64_t id);
    /** Request Stop without blocking the UI. Read removal.committed in StateRead:
     * a late request cannot restore a removal which storage already committed. */
    UmiStatus UmiStudioTestArchiveRemoveCancel(UmiStudioTestService *service);

#ifdef __cplusplus
}
#endif
#endif
