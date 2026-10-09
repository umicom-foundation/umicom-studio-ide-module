/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_native_attach.c
 * PURPOSE: Check that Studio attachment neither starts a build nor terminates an attached target.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/filesystem.h"
#include "umicom/platform/path.h"
#include "umicom/studio/build.h"
#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <string.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            fprintf(stderr, "%d: %s\n", __LINE__, #v);                                             \
            failed = 1;                                                                            \
            goto done;                                                                             \
        }                                                                                          \
    } while (0)
int main(int argc, char **argv)
{
    if (argc != 4)
        return 2;
    const char *mode = argv[1];
    bool denied = strcmp(mode, "denied") == 0;
    bool invalid = strcmp(mode, "invalid") == 0;
    bool relative_adapter = strcmp(mode, "relative-adapter") == 0;
    bool destroy = strcmp(mode, "destroy") == 0;
    bool busy = strcmp(mode, "busy") == 0;
    if (!denied && !invalid && !relative_adapter && !destroy && !busy &&
        strcmp(mode, "normal") != 0)
        return 2;
    int failed = 0;
    UmiStudioBuildService *build = NULL;
    UmiStudioDebuggerService *debugger = NULL;
    UmiClock clock = umi_clock_system();
    char settings[UMI_PATH_CAPACITY], transcript[UMI_PATH_CAPACITY];
    char *log = NULL;
    size_t bytes = 0U;
    CHECK(umi_path_join(argv[3], "attach-fixture-mode.txt", settings, sizeof settings) ==
          UMI_STATUS_OK);
    CHECK(umi_path_join(argv[3], "attach-fixture-requests.jsonl", transcript, sizeof transcript) ==
          UMI_STATUS_OK);
    CHECK(umi_fs_write_text(settings, "normal") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(transcript, "") == UMI_STATUS_OK);
    CHECK(umi_studio_build_service_create(argv[3], &clock, &build) == UMI_STATUS_OK);
    CHECK(umi_studio_debugger_service_create(&debugger) == UMI_STATUS_OK);
    CHECK(UmiStudioDebuggerConfigureNative(
              debugger, "gdb", relative_adapter ? "fixture-adapter" : argv[2]) == UMI_STATUS_OK);
    UmiStudioBuildSnapshot before, after;
    CHECK(umi_studio_build_service_snapshot(build, &before) == UMI_STATUS_OK);
    UmiStatus expected = denied             ? UMI_STATUS_PERMISSION_DENIED
                         : invalid          ? UMI_STATUS_INVALID_ARGUMENT
                         : relative_adapter ? UMI_STATUS_INVALID_ARGUMENT
                                            : UMI_STATUS_OK;
    CHECK(UmiStudioDebuggerAttachNative(debugger, build, invalid ? 0U : 123456U, "", !denied) ==
          expected);
    CHECK(umi_studio_build_service_snapshot(build, &after) == UMI_STATUS_OK);
    /* Attachment is not a build phase. Even a successful request leaves the
     * next build identity and recorded history available for the user's next build. */
    CHECK(before.history_count == after.history_count &&
          before.next_operation_id == after.next_operation_id);
    if (expected == UMI_STATUS_OK)
    {
        CHECK(UmiStudioDebuggerNativeBusy(debugger));
        if (busy)
            CHECK(UmiStudioDebuggerAttachNative(debugger, build, 123457U, "", 1) ==
                  UMI_STATUS_BUSY);
        if (destroy)
        {
            umi_studio_debugger_service_destroy(debugger);
            debugger = NULL;
        }
        else
        {
            CHECK(umi_studio_debugger_service_stop(debugger, 0) == UMI_STATUS_OK);
            CHECK(!UmiStudioDebuggerNativeBusy(debugger));
        }
    }
    else
        CHECK(!UmiStudioDebuggerNativeBusy(debugger));
    CHECK(umi_fs_read_text(transcript, &log, &bytes) == UMI_STATUS_OK);
    if (expected != UMI_STATUS_OK)
        CHECK(bytes == 0U);
    else
    {
        CHECK(strstr(log, "\"command\":\"attach\"") != NULL);
        CHECK(strstr(log, "\"command\":\"launch\"") == NULL);
        CHECK(strstr(log, "\"terminateDebuggee\":false") != NULL);
        CHECK(strstr(log, "\"terminateDebuggee\":true") == NULL);
    }
done:
    umi_studio_debugger_service_destroy(debugger);
    umi_studio_build_service_destroy(build);
    umi_clock_dispose(&clock);
    umi_fs_free_text(log);
    return failed;
}
