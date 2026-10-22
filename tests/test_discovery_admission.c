/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_discovery_admission.c
 * PURPOSE: Exercise canonical command admission without a graphical caller.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/commands.h"
#include "umicom/studio/build.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/test_discovery.h"
#include "umicom/build/project_profile.h"
#include "umicom/platform/clock.h"
#include "umicom/platform/filesystem.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *name = argv[1];
    const char *cases[] = {"closed", "untrusted", "armed-untrusted", "profile-mismatch", "catalogue-mismatch", "empty-owner"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(name, cases[i]) == 0) known = 1;
    if (!known) return 2;
    int failed = 0, created = 0;
    UmiStudioServices *services = NULL;
    UmiStudioTestService *tests = NULL;
    UmiDataServer *storage = NULL;
    UmiCommandRegistry *commands = NULL;
    UmiStudioServicesOptions options = {0};
    UmiClock clock = umi_clock_system();
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], buildRoot[UMI_PATH_CAPACITY];
    char file[UMI_PATH_CAPACITY], sentinel[UMI_PATH_CAPACITY], other[UMI_PATH_CAPACITY], folder[128];
    CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
    (void)snprintf(folder, sizeof folder, "discovery-%s-%" PRIu64, name, clock.wall_nanoseconds(&clock));
    CHECK(umi_fs_join(root, sizeof root, cwd, folder) == UMI_STATUS_OK && !umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    created = 1;
    CHECK(umi_fs_join(buildRoot, sizeof buildRoot, root, "build") == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(buildRoot) == UMI_STATUS_OK);
    CHECK(umi_fs_join(file, sizeof file, buildRoot, "CTestTestfile.cmake") == UMI_STATUS_OK);
    /* Discovery can execute CMake code even when there are no tests. None of
     * these refused requests may evaluate this sentinel-writing input. */
    CHECK(umi_fs_write_text(file, "file(WRITE \"${CMAKE_CURRENT_LIST_DIR}/discovery-ran.txt\" \"unexpected\")\n") == UMI_STATUS_OK);
    CHECK(umi_fs_join(sentinel, sizeof sentinel, buildRoot, "discovery-ran.txt") == UMI_STATUS_OK);
    CHECK(umi_fs_join(other, sizeof other, root, "another-source") == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(other) == UMI_STATUS_OK);
    CHECK(umi_data_server_create_memory(&storage) == UMI_STATUS_OK);
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, storage) == UMI_STATUS_OK);
    CHECK(umi_command_registry_create(&commands) == UMI_STATUS_OK);
    CHECK(umi_studio_commands_register(commands, services) == UMI_STATUS_OK);
    tests = umi_studio_services_tests(services);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    if (strcmp(name, "closed") != 0) {
        int trusted = strcmp(name, "untrusted") != 0 && strcmp(name, "armed-untrusted") != 0;
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, trusted) == UMI_STATUS_OK);
        UmiStudioWorkspaceSnapshot workspace;
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        if (strcmp(name, "profile-mismatch") == 0) {
            UmiBuildProfile replacement;
            CHECK(UmiBuildProfileForWorkspace(other, &replacement) == UMI_STATUS_OK);
            CHECK(umi_studio_build_service_set_profile(build, &replacement) == UMI_STATUS_OK);
        } else if (strcmp(name, "catalogue-mismatch") == 0)
            CHECK(umi_studio_test_service_set_workspace(tests, other, "workspace", workspace.graph.revision) == UMI_STATUS_OK);
        else if (strcmp(name, "empty-owner") == 0)
            CHECK(umi_studio_test_service_set_workspace(tests, root, "", workspace.graph.revision) == UMI_STATUS_OK);
        if (strcmp(name, "armed-untrusted") == 0)
            CHECK(UmiStudioTestDiscoveryArm(tests, umi_studio_services_task_queue(services), workspace.graph.revision) == UMI_STATUS_OK);
    }
    UmiBuildProfile beforeProfile = *umi_studio_build_service_profile(build);
    UmiStudioTestExplorerState before, after;
    UmiStudioTestSnapshot beforeTests, afterTests;
    CHECK(umi_studio_test_service_explorer_state(tests, &before) == UMI_STATUS_OK);
    CHECK(umi_studio_test_service_snapshot(tests, &beforeTests) == UMI_STATUS_OK);
    char message[512];
    UmiStatus expected = strstr(name, "untrusted") != NULL ? UMI_STATUS_PERMISSION_DENIED : UMI_STATUS_INVALID_STATE;
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_TESTS_DISCOVER, buildRoot,
        message, sizeof message) == expected);
    const char *reason = strcmp(name, "closed") == 0 ? "Open a project" :
        expected == UMI_STATUS_PERMISSION_DENIED ? "Trust the open project" : "no longer match";
    CHECK(strstr(message, reason) != NULL);
    CHECK(!UmiStudioTestDiscoveryPending(tests) && !umi_fs_exists(sentinel));
    CHECK(umi_build_profile_equal(&beforeProfile, umi_studio_build_service_profile(build)));
    CHECK(umi_studio_test_service_explorer_state(tests, &after) == UMI_STATUS_OK);
    CHECK(umi_studio_test_service_snapshot(tests, &afterTests) == UMI_STATUS_OK);
    CHECK(before.revision == after.revision && before.workspace_revision == after.workspace_revision);
    CHECK(strcmp(before.workspace_root, after.workspace_root) == 0 && strcmp(before.active_project_id, after.active_project_id) == 0);
    CHECK(beforeTests.test_count == afterTests.test_count && beforeTests.suite_count == afterTests.suite_count);
    UmiStudioTestDiscoveryDisarm(tests);
cleanup:
    UmiStudioTestDiscoveryDisarm(tests);
    umi_command_registry_destroy(commands);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(storage);
    if (created) (void)umi_fs_remove_tree(root);
    return failed;
}
