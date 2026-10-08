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
    } UmiStudioTestArchiveState;
    /* All methods belong to the service's creating thread. Open validates an
 * absolute local filename and the catalogue before replacing an attachment.
 * NULL detaches without deleting the database. No location or credentials are
 * persisted in project files. Choose a private directory with suitable access.
 * Opening or reading an archive never executes tests or changes their status. */
    UmiStatus UmiStudioTestArchiveOpen(UmiStudioTestService *service, const char *path);
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
    UmiStatus UmiStudioTestArchiveRemove(UmiStudioTestService *service, uint64_t id);
#ifdef __cplusplus
}
#endif
#endif
