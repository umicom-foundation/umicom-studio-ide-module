/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive_compare.inc
 * PURPOSE: Check comparison ownership without executing user test commands.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
/* Inline keeps SQLite-disabled test builds free of unused local helpers. */
static inline void CompareServiceCase(const char *name, UmiStudioTestService *service, UmiTaskQueue *queue,
                                      const char *path)
{
    CHECK(ArchiveDisabledRun(service, queue) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
    UmiStudioTestArchiveState state = {0};
    CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK);
    bool gated = strcmp(name, "compare-cancel") == 0 || strcmp(name, "compare-busy") == 0;
    ArchiveComparisonGate gate;
    atomic_init(&gate.entered, 0);
    atomic_init(&gate.released, 0);
    UmiTask *blocker = NULL;
    if (gated)
    {
        UmiTaskConfig config = {0};
        config.function = ArchiveComparisonWaitGate;
        config.user_data = &gate;
        CHECK(umi_task_create(&config, &blocker) == UMI_STATUS_OK);
        CHECK(umi_task_queue_submit(queue, blocker) == UMI_STATUS_OK);
        for (unsigned i = 0; i < 5000U && !atomic_load(&gate.entered); ++i)
            umi_thread_sleep_ms(1U);
        CHECK(atomic_load(&gate.entered));
    }
    CHECK(UmiStudioTestArchiveCompare(service, queue, 1, strcmp(name, "compare-missing") == 0 ? 99 : 1) ==
          UMI_STATUS_OK);
    if (gated)
    {
        CHECK(UmiStudioTestArchivePending(service));
        CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_BUSY);
        CHECK(UmiStudioTestArchiveOpen(service, path) == UMI_STATUS_BUSY);
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_BUSY);
        CHECK(UmiStudioTestArchiveRemove(service, 1) == UMI_STATUS_BUSY);
        CHECK(UmiStudioTestArchiveCompare(service, queue, 1, 1) == UMI_STATUS_BUSY);
        if (strcmp(name, "compare-cancel") == 0)
            CHECK(UmiStudioTestArchiveComparisonCancel(service) == UMI_STATUS_OK);
        atomic_store(&gate.released, 1);
    }
    UmiStatus status = ArchiveAwaitComparison(service, &state);
    if (strcmp(name, "compare-cancel") == 0)
        CHECK(status == UMI_STATUS_CANCELLED && !state.comparison.ready);
    else if (strcmp(name, "compare-missing") == 0)
        CHECK(status == UMI_STATUS_NOT_FOUND && !state.comparison.ready);
    else
    {
        CHECK(strcmp(name, "compare-read") == 0 || strcmp(name, "compare-busy") == 0 ||
              strcmp(name, "compare-retry") == 0 || strcmp(name, "compare-remove") == 0);
        CHECK(status == UMI_STATUS_OK && state.comparison.ready);
        UmiTestArchiveComparisonRow row = {0};
        CHECK(UmiStudioTestArchiveComparisonRowAt(service, 0, &row) == UMI_STATUS_OK);
        CHECK(row.change == UMI_TEST_ARCHIVE_UNCHANGED && row.after.skipped == 1 && row.after.passed == 0);
        if (strcmp(name, "compare-retry") == 0)
        {
            CHECK(UmiStudioTestArchiveCompare(service, NULL, 1, 1) == UMI_STATUS_INVALID_ARGUMENT);
            CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && state.comparison.ready);
        }
        if (strcmp(name, "compare-remove") == 0)
        {
            CHECK(UmiStudioTestArchiveRemove(service, 1) == UMI_STATUS_OK);
            CHECK(UmiStudioTestArchiveComparisonRowAt(service, 0, &row) == UMI_STATUS_OK);
        }
        CHECK(UmiStudioTestArchiveComparisonCancel(service) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && state.comparison.ready);
        CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveComparisonRowAt(service, 0, &row) == UMI_STATUS_NOT_FOUND);
    }
    if (blocker != NULL)
    {
        for (unsigned i = 0; i < 5000U && umi_task_state(blocker) != UMI_TASK_SUCCEEDED; ++i)
            umi_thread_sleep_ms(1U);
        CHECK(umi_task_state(blocker) == UMI_TASK_SUCCEEDED);
        umi_task_destroy(blocker);
    }
}
