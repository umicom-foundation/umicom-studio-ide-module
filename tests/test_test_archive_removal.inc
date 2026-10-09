/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive_removal.inc
 * PURPOSE: Check exact saved-run removal, attachment exclusion and save-again behavior.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static void RemovalServiceCase(const char *name, UmiStudioTestService *service, UmiTaskQueue *queue,
                               const char *path)
{
    CHECK(ArchiveDisabledRun(service, queue) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
    UmiStudioTestArchiveState state = {0};
    CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK);
    UmiStudioTestSnapshot before = {0}, after = {0};
    CHECK(umi_studio_test_service_snapshot(service, &before) == UMI_STATUS_OK);
    ArchiveComparisonGate gate;
    atomic_init(&gate.entered, 0);
    atomic_init(&gate.released, 0);
    UmiTaskConfig config = {0};
    config.function = ArchiveComparisonWaitGate;
    config.user_data = &gate;
    UmiTask *blocker = NULL;
    CHECK(umi_task_create(&config, &blocker) == UMI_STATUS_OK);
    CHECK(umi_task_queue_submit(queue, blocker) == UMI_STATUS_OK);
    for (unsigned i = 0; i < 5000 && !atomic_load(&gate.entered); ++i)
        umi_thread_sleep_ms(1);
    CHECK(atomic_load(&gate.entered));
    CHECK(UmiStudioTestArchiveRemoveBegin(
              service, queue, strcmp(name, "removal-missing") == 0 ? 99 : 1) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchivePending(service));
    CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveOpenBegin(service, queue, path) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveRemove(service, 1) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveRemoveBegin(service, queue, 1) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveBrowse(service, queue, UMI_TEST_ARCHIVE_READ_CATALOG, 0, 0) ==
          UMI_STATUS_BUSY);
    if (strcmp(name, "removal-cancel") == 0)
        CHECK(UmiStudioTestArchiveRemoveCancel(service) == UMI_STATUS_OK);
    atomic_store(&gate.released, 1);
    UmiStatus expected = strcmp(name, "removal-cancel") == 0    ? UMI_STATUS_CANCELLED
                         : strcmp(name, "removal-missing") == 0 ? UMI_STATUS_NOT_FOUND
                                                                : UMI_STATUS_OK;
    CHECK(ArchiveAwaitRemoval(service, &state) == expected);
    CHECK(state.removal.committed == (expected == UMI_STATUS_OK) && !state.pending);
    UmiTestArchiveEntry entry = {0};
    CHECK(UmiStudioTestArchiveRead(service, 1, &entry) ==
          (state.removal.committed ? UMI_STATUS_NOT_FOUND : UMI_STATUS_OK));
    if (strcmp(name, "removal-late-stop") == 0)
    {
        CHECK(UmiStudioTestArchiveRemoveCancel(service) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK &&
              state.removal.committed);
    }
    if (strcmp(name, "removal-retry") == 0)
    {
        CHECK(UmiStudioTestArchiveRemoveBegin(service, NULL, 2) == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK &&
              state.removal.id == 1 && state.removal.committed);
    }
    if (strcmp(name, "removal-resave") == 0)
    {
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
        CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK && state.write.saved.id == 2);
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_ALREADY_EXISTS);
    }
    if (strcmp(name, "removal-detach") == 0)
    {
        CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && !state.attached &&
              state.removal.id == 0 && !state.removal.committed);
    }
    CHECK(strcmp(name, "removal-success") == 0 || strcmp(name, "removal-cancel") == 0 ||
          strcmp(name, "removal-missing") == 0 || strcmp(name, "removal-late-stop") == 0 ||
          strcmp(name, "removal-retry") == 0 || strcmp(name, "removal-resave") == 0 ||
          strcmp(name, "removal-detach") == 0);
    CHECK(umi_studio_test_service_snapshot(service, &after) == UMI_STATUS_OK &&
          before.retained_result_count == after.retained_result_count);
    CHECK(umi_task_wait(blocker, 5000) == UMI_STATUS_OK);
    umi_task_destroy(blocker);
}
