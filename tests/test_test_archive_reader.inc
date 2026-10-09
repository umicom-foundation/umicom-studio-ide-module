/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive_reader.inc
 * PURPOSE: Check Studio archive attachment guards and copied background observations.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static void ReaderServiceCase(const char *name, UmiStudioTestService *service, UmiTaskQueue *queue)
{
    CHECK(ArchiveDisabledRun(service, queue) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
    UmiStudioTestArchiveState state = {0};
    CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK);
    bool catalog_read = strcmp(name, "reader-catalog") == 0 || strcmp(name, "reader-retry") == 0;
    UmiTestArchiveReadKind kind =
        catalog_read ? UMI_TEST_ARCHIVE_READ_CATALOG : UMI_TEST_ARCHIVE_READ_ATTEMPT;
    ArchiveComparisonGate gate;
    atomic_init(&gate.entered, 0);
    atomic_init(&gate.released, 0);
    UmiTaskConfig config = {0};
    config.function = ArchiveComparisonWaitGate;
    config.user_data = &gate;
    UmiTask *blocker = NULL;
    CHECK(umi_task_create(&config, &blocker) == UMI_STATUS_OK);
    CHECK(umi_task_queue_submit(queue, blocker) == UMI_STATUS_OK);
    for (unsigned i = 0; i < 5000U && !atomic_load(&gate.entered); ++i)
        umi_thread_sleep_ms(1U);
    CHECK(atomic_load(&gate.entered));
    CHECK(UmiStudioTestArchiveBrowse(service, queue, kind,
                                     catalog_read                          ? 0
                                     : strcmp(name, "reader-missing") == 0 ? 99
                                                                           : 1,
                                     0) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchivePending(service));
    CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveRemove(service, 1) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveBrowse(service, queue, kind, 1, 0) == UMI_STATUS_BUSY);
    if (strcmp(name, "reader-cancel") == 0)
        CHECK(UmiStudioTestArchiveReadCancel(service) == UMI_STATUS_OK);
    atomic_store(&gate.released, 1);
    UmiStatus expected = strcmp(name, "reader-cancel") == 0    ? UMI_STATUS_CANCELLED
                         : strcmp(name, "reader-missing") == 0 ? UMI_STATUS_NOT_FOUND
                                                               : UMI_STATUS_OK;
    CHECK(ArchiveAwaitRead(service, &state) == expected);
    if (expected == UMI_STATUS_OK)
    {
        if (catalog_read)
        {
            UmiTestArchiveCatalog *catalog = calloc(1, sizeof(*catalog));
            CHECK(catalog != NULL);
            CHECK(UmiStudioTestArchiveCopyCatalog(service, catalog) == UMI_STATUS_OK);
            CHECK(catalog->count == 1 && catalog->entries[0].run.skipped == 1);
            if (strcmp(name, "reader-retry") == 0)
            {
                /* Queue rejection must retain the previous completed observation. */
                CHECK(UmiStudioTestArchiveBrowse(service, NULL, kind, 0, 0) ==
                      UMI_STATUS_INVALID_ARGUMENT);
                CHECK(UmiStudioTestArchiveCopyCatalog(service, catalog) == UMI_STATUS_OK &&
                      catalog->count == 1);
            }
            free(catalog);
        }
        else
        {
            UmiTestArchiveAttempt *attempt = calloc(1, sizeof(*attempt));
            CHECK(attempt != NULL);
            CHECK(UmiStudioTestArchiveCopyAttempt(service, attempt) == UMI_STATUS_OK);
            CHECK(attempt->entry.id == 1 && attempt->attempt == 1 &&
                  attempt->result.state == UMI_TEST_STATE_SKIPPED);
            CHECK(strcmp(attempt->result.test_id, attempt->request.test_id) == 0);
            free(attempt);
        }
    }
    else
        CHECK(!state.reader.ready);
    CHECK(umi_task_wait(blocker, 5000U) == UMI_STATUS_OK);
    umi_task_destroy(blocker);
}
