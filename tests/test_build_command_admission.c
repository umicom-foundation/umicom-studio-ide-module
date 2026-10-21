/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_command_admission.c
 * PURPOSE: Verify project authority before canonical build and debug operations.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/commands.h"
#include "umicom/studio/build.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/workspace.h"
#include "umicom/platform/clock.h"
#include "umicom/platform/filesystem.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *state = argv[2];
    if (strcmp(state, "closed") != 0 && strcmp(state, "untrusted") != 0 &&
        strcmp(state, "other-project") != 0 && strcmp(state, "relative-source") != 0) return 2;
    static const struct { const char *name; const char *id; } operations[] = {
        {"configure", UMI_STUDIO_COMMAND_BUILD_CONFIGURE},
        {"compile", UMI_STUDIO_COMMAND_BUILD_COMPILE},
        {"test", UMI_STUDIO_COMMAND_BUILD_TEST},
        {"clean", UMI_STUDIO_COMMAND_BUILD_CLEAN},
        {"run", UMI_STUDIO_COMMAND_BUILD_RUN},
        {"run-current", UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT},
        {"install", UMI_STUDIO_COMMAND_BUILD_INSTALL},
        {"package", UMI_STUDIO_COMMAND_BUILD_PACKAGE},
        {"rebuild", UMI_STUDIO_COMMAND_BUILD_REBUILD},
        {"deploy", UMI_STUDIO_COMMAND_BUILD_DEPLOY},
        {"retry", UMI_STUDIO_COMMAND_BUILD_RETRY},
        {"run-next", UMI_STUDIO_COMMAND_BUILD_RUN_NEXT},
        {"run-all", UMI_STUDIO_COMMAND_BUILD_RUN_ALL},
        {"debug", UMI_STUDIO_COMMAND_DEBUG_START},
        {"tests-all", UMI_STUDIO_COMMAND_TESTS_RUN_ALL},
        {"tests-selected", UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED},
        {"tests-debug", UMI_STUDIO_COMMAND_TESTS_DEBUG_SELECTED},
        {"tests-coverage", UMI_STUDIO_COMMAND_TESTS_RUN_COVERAGE},
        {"tests-failed", UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED},
        {"tests-repeat", UMI_STUDIO_COMMAND_TESTS_REPEAT_SELECTED}
    };
    const char *command = NULL;
    for (size_t i = 0U; i < sizeof operations / sizeof operations[0]; ++i)
        if (strcmp(argv[1], operations[i].name) == 0) command = operations[i].id;
    if (command == NULL) return 2;
    int failed = 0, created = 0;
    UmiStudioServices *services = NULL;
    UmiDataServer *storage = NULL;
    UmiCommandRegistry *commands = NULL;
    UmiStudioServicesOptions options = {0};
    UmiClock clock = umi_clock_system();
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], other[UMI_PATH_CAPACITY], name[128];
    CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
    (void)snprintf(name, sizeof name, "build-admission-%" PRIu64, clock.wall_nanoseconds(&clock));
    CHECK(umi_fs_join(root, sizeof root, cwd, name) == UMI_STATUS_OK && !umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    created = 1;
    CHECK(umi_fs_join(other, sizeof other, root, "another-project") == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(other) == UMI_STATUS_OK);
    CHECK(umi_data_server_create_memory(&storage) == UMI_STATUS_OK);
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, storage) == UMI_STATUS_OK);
    CHECK(umi_command_registry_create(&commands) == UMI_STATUS_OK);
    CHECK(umi_studio_commands_register(commands, services) == UMI_STATUS_OK);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
    if (strcmp(state, "closed") != 0)
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, strcmp(state, "untrusted") != 0) == UMI_STATUS_OK);
    UmiBuildProfile selected = *umi_studio_build_service_profile(build);
    if (strcmp(state, "other-project") == 0) strcpy(selected.source_directory, other);
    else if (strcmp(state, "relative-source") == 0) strcpy(selected.source_directory, ".");
    /* An explicit program avoids a missing-target guard hiding the ownership
     * defect. Admission must refuse this before any executable is inspected. */
    strcpy(selected.run_program, "./unselected-program");
    CHECK(umi_studio_build_service_set_profile(build, &selected) == UMI_STATUS_OK);
    UmiStudioBuildSnapshot before, after;
    CHECK(umi_studio_build_service_snapshot(build, &before) == UMI_STATUS_OK);
    UmiBuildProjectSessionSnapshot progressBefore, progressAfter;
    CHECK(UmiStudioBuildProgress(build, &progressBefore) == UMI_STATUS_OK);
    char message[512];
    UmiStatus expected = strcmp(state, "untrusted") == 0 ? UMI_STATUS_PERMISSION_DENIED : UMI_STATUS_INVALID_STATE;
    /* A native host can inspect the same admission before saving a draft. This
     * call must return the canonical refusal without changing build state. */
    CHECK(UmiStudioBuildCheckAdmission(services, message, sizeof message) == expected);
    CHECK(UmiStudioBuildCheckAdmission(services, NULL, 0U) == expected);
    CHECK(UmiStudioBuildCheckAdmission(NULL, message, sizeof message) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(message[0] == '\0');
    /* Both entry styles reach the same canonical gate. Package/rebuild/deploy
     * always queue; the ordinary phases also have a synchronous compatibility API. */
    for (unsigned background = 0U; background < 2U; ++background) {
        CHECK(umi_command_registry_execute(commands, command,
            background ? "background" : NULL, message, sizeof message) == expected);
        const char *reason = strcmp(state, "closed") == 0 ? "Open a project" :
            strcmp(state, "untrusted") == 0 ? "enable its build tools" : "no longer matches";
        CHECK(strstr(message, reason) != NULL);
        CHECK(!UmiStudioBuildBusy(build) && !UmiStudioDebuggerNativeBusy(debugger));
        CHECK(umi_build_profile_equal(&selected, umi_studio_build_service_profile(build)));
        CHECK(umi_studio_build_service_snapshot(build, &after) == UMI_STATUS_OK);
        CHECK(before.history_count == after.history_count && before.next_operation_id == after.next_operation_id);
        CHECK(before.graph.revision == after.graph.revision && before.graph.node_count == after.graph.node_count);
        CHECK(UmiStudioBuildProgress(build, &progressAfter) == UMI_STATUS_OK);
        CHECK(progressBefore.operation_id == progressAfter.operation_id && !progressAfter.active);
    }
cleanup:
    umi_command_registry_destroy(commands);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(storage);
    if (created) (void)umi_fs_remove_tree(root);
    return failed;
}
