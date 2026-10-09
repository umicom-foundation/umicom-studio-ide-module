/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_archive.c
 * PURPOSE: Check private archive attachment, accepted-run origin, restart and owner-thread boundaries.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "fixture.h"
#include "test_archive_job.h"
typedef struct WrongThread
{
    UmiStudioTestService *service;
    UmiStatus status;
} WrongThread;
static inline int ReadElsewhere(void *data)
{
    WrongThread *request = data;
    UmiStudioTestArchiveState state = {0};
    request->status = UmiStudioTestArchiveStateRead(request->service, &state);
    return 0;
}
#include "test_test_archive_compare.inc"
#include "test_test_archive_reader.inc"
#include "test_test_archive_open.inc"
#include "test_test_archive_removal.inc"

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
#ifndef UMICOM_HAS_SQLITE
    (void)argv;
    return 77;
#else
    const char *name = argv[1];
    char directory[UMI_PATH_CAPACITY], path[UMI_PATH_CAPACITY];
    FixtureDirectory(directory);
    FixturePath(path, directory, "studio-tests.sqlite");
    UmiStudioTestService *service = NULL;
    CHECK(umi_studio_test_service_create(&service) == UMI_STATUS_OK);
    UmiTaskQueue *queue = NULL;
    UmiTaskQueueConfig config = umi_task_queue_config_default();
    config.worker_count = 1;
    CHECK(umi_task_queue_create(&config, &queue) == UMI_STATUS_OK);
    UmiStudioTestArchiveState state = {0};
    CHECK(UmiStudioTestArchiveOpen(service, path) == UMI_STATUS_OK);
    if (strncmp(name, "compare-", 8) == 0)
        CompareServiceCase(name, service, queue, path);
    else if (strncmp(name, "open-", 5) == 0)
        OpenServiceCase(name, service, queue, path);
    else if (strncmp(name, "removal-", 8) == 0)
        RemovalServiceCase(name, service, queue, path);
    else if (strncmp(name, "reader-", 7) == 0)
        ReaderServiceCase(name, service, queue);
    else if (strcmp(name, "identity") == 0)
    {
        UmiJobIdentity accepted = {0};
        memset(accepted.subject, 'a', 64);
        memset(accepted.configuration, 'b', 64);
        CHECK(ArchiveDisabledRunWithIdentity(service, queue, &accepted) == UMI_STATUS_OK);
        UmiStudioTestRunContext later = {0};
        later.trusted = 1;
        strcpy(later.source_root, "/different/source");
        strcpy(later.build_root, "/different/build");
        later.identity = accepted; later.identity.configuration[0] = 'c';
        CHECK(UmiStudioTestRunArm(service, queue, &later) == UMI_STATUS_OK);
        UmiStudioTestRunDisarm(service);
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_OK);
        CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK);
        CHECK(memcmp(&state.write.saved.origin.identity, &accepted, sizeof(accepted)) == 0);
        CHECK(state.write.saved.selection_digest[0] != '\0');
        CHECK(state.write.saved.run.passed == 0 && state.write.saved.run.skipped == 1);
        umi_studio_test_service_destroy(service); service = NULL;
        CHECK(umi_studio_test_service_create(&service) == UMI_STATUS_OK);
        CHECK(UmiStudioTestArchiveOpen(service, path) == UMI_STATUS_OK);
        UmiTestArchiveEntry saved = {0};
        CHECK(UmiStudioTestArchiveRead(service, 1, &saved) == UMI_STATUS_OK);
        CHECK(UmiJobIdentityCompare(&saved.origin.identity, &accepted) ==
              UMI_JOB_IDENTITY_INPUTS_UNRECORDED);
        CHECK(strcmp(saved.selection_digest, state.write.saved.selection_digest) == 0);
    }
    else if (strcmp(name, "wrong-thread") == 0)
    {
        WrongThread request = {service, UMI_STATUS_OK};
        UmiThread *thread = NULL;
        int code = 0;
        CHECK(umi_thread_start(ReadElsewhere, &request, &thread) == UMI_STATUS_OK);
        CHECK(umi_thread_join(thread, &code) == UMI_STATUS_OK && request.status == UMI_STATUS_INVALID_STATE);
        umi_thread_destroy(thread);
    }
    else if (strcmp(name, "invalid") == 0)
    {
        CHECK(UmiStudioTestArchiveOpen(service, "relative.sqlite") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK &&
              strcmp(state.path, path) == 0);
    }
    else if (strcmp(name, "no-run") == 0)
    {
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_NOT_FOUND);
    }
    else
    {
        CHECK(ArchiveDisabledRun(service, queue) == UMI_STATUS_OK);
        /* An armed command which fails must not relabel the accepted run. */
        UmiStudioTestRunContext later = {0};
        later.trusted = 1;
        later.workspace_generation = 90;
        strcpy(later.source_root, "/other/source");
        strcpy(later.build_root, "/other/build");
        CHECK(UmiStudioTestRunArm(service, queue, &later) == UMI_STATUS_OK);
        UmiStudioTestRunDisarm(service);
        bool output = strcmp(name, "retain-output") == 0;
        CHECK(UmiStudioTestArchiveSave(service, queue, output) == UMI_STATUS_OK);
        CHECK(ArchiveAwaitSave(service, &state) == UMI_STATUS_OK && state.write.saved.id == 1);
        CHECK(strcmp(state.write.saved.origin.source_root, "/archive-fixture/source") == 0);
        CHECK(UmiStudioTestArchiveSave(service, queue, false) == UMI_STATUS_ALREADY_EXISTS);
        if (strcmp(name, "detach") == 0)
        {
            CHECK(UmiStudioTestArchiveOpen(service, NULL) == UMI_STATUS_OK);
            CHECK(UmiStudioTestArchiveStateRead(service, &state) == UMI_STATUS_OK && !state.attached);
        }
        else
        {
            CHECK(strcmp(name, "reopen") == 0 || strcmp(name, "retain-output") == 0 ||
                  strcmp(name, "remove") == 0);
            umi_studio_test_service_destroy(service);
            service = NULL;
            CHECK(umi_studio_test_service_create(&service) == UMI_STATUS_OK);
            CHECK(UmiStudioTestArchiveOpen(service, path) == UMI_STATUS_OK);
            UmiTestArchiveEntry entry = {0};
            CHECK(UmiStudioTestArchiveRead(service, 1, &entry) == UMI_STATUS_OK);
            CHECK(entry.run.skipped == 1 && entry.run.passed == 0 && entry.origin.workspace_generation == 17);
            UmiTestResult *result = calloc(1, sizeof(*result));
            CHECK(result != NULL);
            uint32_t attempt = 0;
            CHECK(UmiStudioTestArchiveResultAt(service, 1, 0, result, &attempt) == UMI_STATUS_OK &&
                  attempt == 1);
            CHECK((result->output[0] != '\0') == output);
            free(result);
            UmiStudioTestSnapshot current = {0};
            CHECK(umi_studio_test_service_snapshot(service, &current) == UMI_STATUS_OK &&
                  current.retained_result_count == 0);
            if (strcmp(name, "remove") == 0)
            {
                CHECK(UmiStudioTestArchiveRemove(service, 1) == UMI_STATUS_OK);
                CHECK(UmiStudioTestArchiveRead(service, 1, &entry) == UMI_STATUS_NOT_FOUND);
            }
        }
    }
    umi_studio_test_service_destroy(service);
    umi_task_queue_destroy(queue);
    return 0;
#endif
}
