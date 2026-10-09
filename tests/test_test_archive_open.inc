/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive_open.inc
 * PURPOSE: Check private archive replacement while preserving a usable previous attachment.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static inline UmiStatus AwaitServiceOpen(UmiStudioTestService *service,
                                         UmiStudioTestArchiveState *state)
{
    for (unsigned i = 0; i < 5000U; ++i)
    {
        UmiStatus status = UmiStudioTestArchiveStateRead(service, state);
        if (status != UMI_STATUS_OK)
            return status;
        if (state->opening.state == UMI_TASK_SUCCEEDED || state->opening.state == UMI_TASK_FAILED ||
            state->opening.state == UMI_TASK_CANCELLED)
            return state->opening.status;
        umi_thread_sleep_ms(1U);
    }
    return UMI_STATUS_BUSY;
}
static inline void OpenServiceCase(const char *name, UmiStudioTestService *service,
                                   UmiTaskQueue *queue, const char *old_path)
{
    CHECK(ArchiveDisabledRun(service, queue) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
    UmiStudioTestArchiveState state = {0};
    CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK);
    char directory[UMI_PATH_CAPACITY], candidate[UMI_PATH_CAPACITY];
    FixtureDirectory(directory);
    FixturePath(candidate, directory, "replacement.sqlite");
    if (strcmp(name, "open-same") == 0)
        strcpy(candidate, old_path);
    if (strcmp(name, "open-corrupt") == 0)
    {
        UmiDataServer *database = NULL;
        CHECK(umi_data_server_create_sqlite(candidate, &database) == UMI_STATUS_OK);
        CHECK(umi_data_server_set(database, "ctest-archive/studio.tests/next", "invalid") ==
              UMI_STATUS_OK);
        umi_data_server_destroy(database);
    }
    ArchiveComparisonGate gate;
    atomic_init(&gate.entered, 0);
    atomic_init(&gate.released, 0);
    UmiTaskConfig held = {0};
    held.function = ArchiveComparisonWaitGate;
    held.user_data = &gate;
    UmiTask *blocker = NULL;
    CHECK(umi_task_create(&held, &blocker) == UMI_STATUS_OK);
    CHECK(umi_task_queue_submit(queue, blocker) == UMI_STATUS_OK);
    for (unsigned i = 0; i < 5000U && !atomic_load(&gate.entered); ++i)
        umi_thread_sleep_ms(1U);
    CHECK(atomic_load(&gate.entered));
    CHECK(UmiStudioTestArchiveOpenBegin(service, queue, candidate) == UMI_STATUS_OK);
    CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && state.pending &&
          state.attached && strcmp(state.path, old_path) == 0 &&
          strcmp(state.opening.path, candidate) == 0);
    CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveOpenBegin(service, queue, candidate) == UMI_STATUS_BUSY);
    CHECK(UmiStudioTestArchiveOpenPublish(service) == UMI_STATUS_BUSY);
    if (strcmp(name, "open-cancel") == 0)
        CHECK(UmiStudioTestArchiveOpenCancel(service) == UMI_STATUS_OK);
    atomic_store(&gate.released, 1);
    UmiStatus expected = strcmp(name, "open-cancel") == 0    ? UMI_STATUS_CANCELLED
                         : strcmp(name, "open-corrupt") == 0 ? UMI_STATUS_PARSE_ERROR
                                                             : UMI_STATUS_OK;
    CHECK(AwaitServiceOpen(service, &state) == expected);
    CHECK(strcmp(state.path, old_path) == 0);
    if (strcmp(name, "open-late-stop") == 0)
    {
        CHECK(UmiStudioTestArchiveOpenCancel(service) == UMI_STATUS_OK);
        expected = UMI_STATUS_CANCELLED;
    }
    if (strcmp(name, "open-retry") == 0)
    {
        CHECK(UmiStudioTestArchiveOpenBegin(service, NULL, candidate) ==
              UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK &&
              state.opening.ready);
    }
    if (strcmp(name, "open-superseded") == 0)
    {
        CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveOpenPublish(service) == UMI_STATUS_NOT_FOUND);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && !state.attached);
    }
    else
    {
        if (strcmp(name, "open-publish-busy") == 0)
        {
            /* A completed opener may wait while another archive action owns the
             * old attachment. Publication cannot close that borrowed database. */
            CHECK(umi_task_wait(blocker, 5000U) == UMI_STATUS_OK);
            umi_task_destroy(blocker);
            blocker = NULL;
            atomic_store(&gate.entered, 0);
            atomic_store(&gate.released, 0);
            CHECK(umi_task_create(&held, &blocker) == UMI_STATUS_OK);
            CHECK(umi_task_queue_submit(queue, blocker) == UMI_STATUS_OK);
            for (unsigned i = 0; i < 5000U && !atomic_load(&gate.entered); ++i)
                umi_thread_sleep_ms(1U);
            CHECK(atomic_load(&gate.entered));
            CHECK(UmiStudioTestArchiveBrowse(service, queue, UMI_TEST_ARCHIVE_READ_CATALOG, 0, 0) ==
                  UMI_STATUS_OK);
            CHECK(UmiStudioTestArchiveOpenPublish(service) == UMI_STATUS_BUSY);
            atomic_store(&gate.released, 1);
            CHECK(ArchiveAwaitRead(service, &state) == UMI_STATUS_OK);
        }
        CHECK(UmiStudioTestArchiveOpenPublish(service) == expected);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && state.attached);
        UmiTestArchiveCatalog *catalog = calloc(1U, sizeof(*catalog));
        CHECK(catalog != NULL && UmiStudioTestArchiveList(service, catalog) == UMI_STATUS_OK);
        if (expected == UMI_STATUS_OK)
        {
            CHECK(strcmp(state.path, candidate) == 0);
            CHECK(catalog->count == (strcmp(name, "open-same") == 0 ? 1U : 0U));
            CHECK(UmiStudioTestArchiveOpenPublish(service) == UMI_STATUS_ALREADY_EXISTS);
        }
        else
        {
            CHECK(strcmp(state.path, old_path) == 0 && catalog->count == 1 &&
                  catalog->entries[0].id == 1);
        }
        free(catalog);
    }
    CHECK(umi_task_wait(blocker, 5000U) == UMI_STATUS_OK);
    umi_task_destroy(blocker);
}
