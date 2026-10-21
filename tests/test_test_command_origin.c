/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_command_origin.c
 * PURPOSE: Refuse retained catalogues from a different selected project or build.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/commands.h"
#include "umicom/studio/build.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/test_execution.h"
#include "umicom/test_platform/test_platform.h"
#include "umicom/platform/clock.h"
#include "umicom/platform/filesystem.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *mode = argv[1];
    const char *cases[] = {"other-build", "other-config", "other-project", "mixed-projects", "missing-origin", "wrong-explorer"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(mode, cases[i]) == 0) known = 1;
    if (!known) return 2;
    int failed = 0, created = 0;
    UmiStudioServices *services = NULL;
    UmiDataServer *storage = NULL;
    UmiCommandRegistry *commands = NULL;
    UmiStudioServicesOptions options = {0};
    UmiClock clock = umi_clock_system();
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], other[UMI_PATH_CAPACITY], name[128];
    CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
    (void)snprintf(name, sizeof name, "test-origin-%" PRIu64, clock.wall_nanoseconds(&clock));
    CHECK(umi_fs_join(root, sizeof root, cwd, name) == UMI_STATUS_OK && !umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    created = 1;
    CHECK(umi_fs_join(other, sizeof other, root, "another-build") == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(other) == UMI_STATUS_OK);
    CHECK(umi_data_server_create_memory(&storage) == UMI_STATUS_OK);
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, storage) == UMI_STATUS_OK);
    CHECK(umi_command_registry_create(&commands) == UMI_STATUS_OK);
    CHECK(umi_studio_commands_register(commands, services) == UMI_STATUS_OK);
    CHECK(UmiStudioServicesPrepareWorkspace(services, root, 1) == UMI_STATUS_OK);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    const UmiBuildProfile *profile = umi_studio_build_service_profile(build);
    UmiStudioTestService *tests = umi_studio_services_tests(services);
    UmiTestPlatformService *platform = umi_studio_test_service_platform(tests);
    UmiTestPlatformCtestImportOptions input = {0};
    strcpy(input.project_id, "workspace");
    strcpy(input.suite_id, "selected.ctest");
    strcpy(input.configuration, profile->configuration);
    CHECK(UmiBuildProfileBuildDirectory(profile, root, input.build_directory, sizeof input.build_directory) == UMI_STATUS_OK);
    if (strcmp(mode, "other-build") == 0) strcpy(input.build_directory, other);
    if (strcmp(mode, "other-config") == 0) strcpy(input.configuration, "Release");
    if (strcmp(mode, "other-project") == 0) strcpy(input.project_id, "another-project");
    const char *json = "{\"tests\":[{\"name\":\"smoke\",\"command\":[\"unused\"]}]}";
    UmiTestPlatformCtestImportSummary summary;
    CHECK(umi_test_platform_service_import_ctest_json(platform, json, &input, &summary) == UMI_STATUS_OK);
    if (strcmp(mode, "mixed-projects") == 0) {
        strcpy(input.project_id, "another-project");
        strcpy(input.suite_id, "foreign.ctest");
        strcpy(input.build_directory, other);
        CHECK(umi_test_platform_service_import_ctest_json(platform, json, &input, &summary) == UMI_STATUS_OK);
    }
    if (strcmp(mode, "missing-origin") == 0)
        CHECK(umi_test_platform_discovery_registry_remove(umi_test_platform_service_discovery(platform),
            "discovery.selected.ctest") == UMI_STATUS_OK);
    if (strcmp(mode, "wrong-explorer") == 0)
        CHECK(umi_studio_test_service_set_workspace(tests, other, "workspace", 1U) == UMI_STATUS_OK);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    UmiTestPlatformOutputRegistry *outputs = umi_test_platform_service_output(platform);
    uint64_t resultRevision = umi_test_platform_result_registry_revision(results);
    uint64_t outputRevision = umi_test_platform_output_registry_revision(outputs);
    size_t resultCount = umi_test_platform_result_registry_count(results);
    size_t outputCount = umi_test_platform_output_registry_count(outputs);
    char message[512];
    UmiStatus expected = strcmp(mode, "missing-origin") == 0 ? UMI_STATUS_NOT_FOUND : UMI_STATUS_INVALID_STATE;
    CHECK(umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_TESTS_RUN_ALL, NULL, message, sizeof message) == expected);
    CHECK(strstr(message, strcmp(mode, "wrong-explorer") == 0 ? "no longer matches" : "no tests were started") != NULL);
    CHECK(!UmiStudioTestRunPending(tests));
    CHECK(resultRevision == umi_test_platform_result_registry_revision(results));
    CHECK(outputRevision == umi_test_platform_output_registry_revision(outputs));
    CHECK(resultCount == umi_test_platform_result_registry_count(results));
    CHECK(outputCount == umi_test_platform_output_registry_count(outputs));
cleanup:
    umi_command_registry_destroy(commands);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(storage);
    if (created) (void)umi_fs_remove_tree(root);
    return failed;
}
