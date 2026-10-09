/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_configuration_lifecycle.c
 * PURPOSE: Verify configuration lifecycle operations preserve active settings and reject stale Studio authority.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/filesystem.h"
#include "umicom/studio/build.h"
#include "umicom/studio/build_configurations.h"
#include "umicom/studio/workspace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #value);                            \
            exit(1);                                                                               \
        }                                                                                          \
    } while (0)

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    UmiStudioServices *services = NULL;
    UmiStudioServicesOptions options = {0};
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) ==
          UMI_STATUS_OK);
    UmiDataServer *first = NULL, *second = NULL;
    CHECK(umi_data_server_create_memory(&first) == UMI_STATUS_OK);
    CHECK(umi_data_server_create_memory(&second) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, first) == UMI_STATUS_OK);
    char root[UMI_PATH_CAPACITY];
    CHECK(umi_fs_current_directory(root, sizeof root) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    UmiStudioBuildConfigurationContext context;
    CHECK(UmiStudioBuildConfigurationContextRead(services, &context) == UMI_STATUS_OK);
    UmiBuildProfile *saved = malloc(sizeof *saved), *active = malloc(sizeof *active),
                    *loaded = malloc(sizeof *loaded);
    CHECK(saved != NULL && active != NULL && loaded != NULL);
    *active = *umi_studio_build_service_profile(umi_studio_services_build(services));
    *saved = *active;
    strcpy(saved->configuration, "Release");
    strcpy(saved->run_environment, "APP_MODE=review");
    uint64_t revision = 0U;
    CHECK(UmiStudioBuildConfigurationSave(services, &context, "Sample", saved, 0U, &revision) ==
          UMI_STATUS_OK);
    bool stale = false;
    if (strcmp(argv[1], "workspace") == 0)
    {
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
        stale = true;
    }
    else if (strcmp(argv[1], "trust") == 0)
    {
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        stale = true;
    }
    else if (strcmp(argv[1], "rebind") == 0)
    {
        CHECK(UmiStudioBuildProfilesBind(services, second) == UMI_STATUS_OK);
        stale = true;
    }
    else if (strcmp(argv[1], "detach") == 0)
    {
        UmiStudioBuildProfilesDetach(services);
        stale = true;
    }
    if (stale)
    {
        uint64_t unchanged = UINT64_MAX;
        CHECK(UmiStudioBuildConfigurationRename(services, &context, "Sample", "Renamed", revision,
                                                &unchanged) == UMI_STATUS_INVALID_STATE);
        CHECK(unchanged == UINT64_MAX);
        CHECK(UmiStudioBuildConfigurationRemove(services, &context, "Sample", revision,
                                                &unchanged) == UMI_STATUS_INVALID_STATE);
        CHECK(unchanged == UINT64_MAX);
        /* The captured authority becomes stale before touching the old database.
         * Inspect that database directly to prove the named profile survived. */
        CHECK(UmiBuildConfigurationLoad(first, root, "Sample", revision, loaded) == UMI_STATUS_OK);
        CHECK(umi_build_profile_equal(saved, loaded));
    }
    else
    {
        if (strcmp(argv[1], "revision") == 0)
        {
            uint64_t unchanged = UINT64_MAX;
            CHECK(UmiStudioBuildConfigurationRename(services, &context, "Sample", "Renamed", 0U,
                                                    &unchanged) == UMI_STATUS_INVALID_STATE);
            CHECK(UmiStudioBuildConfigurationRemove(services, &context, "Sample", 0U, &unchanged) ==
                  UMI_STATUS_INVALID_STATE);
            CHECK(unchanged == UINT64_MAX);
        }
        else if (strcmp(argv[1], "rename") == 0)
        {
            CHECK(UmiStudioBuildConfigurationRename(services, &context, "Sample", "Renamed",
                                                    revision, &revision) == UMI_STATUS_OK);
            CHECK(UmiStudioBuildConfigurationLoad(services, &context, "Renamed", revision,
                                                  loaded) == UMI_STATUS_OK);
            CHECK(umi_build_profile_equal(saved, loaded));
            CHECK(UmiStudioBuildConfigurationLoad(services, &context, "Sample", revision, loaded) ==
                  UMI_STATUS_NOT_FOUND);
        }
        else
        {
            CHECK(strcmp(argv[1], "remove") == 0);
            CHECK(UmiStudioBuildConfigurationRemove(services, &context, "Sample", revision,
                                                    &revision) == UMI_STATUS_OK);
            UmiBuildConfigurationCatalogue catalogue;
            CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
                  UMI_STATUS_OK);
            CHECK(catalogue.count == 0U && catalogue.revision == revision);
            CHECK(UmiStudioBuildConfigurationSave(services, &context, "Reused", saved, revision,
                                                  &revision) == UMI_STATUS_OK);
        }
        /* Library management does not apply settings or grant execution trust. */
        CHECK(umi_build_profile_equal(
            active, umi_studio_build_service_profile(umi_studio_services_build(services))));
        UmiStudioWorkspaceSnapshot workspace;
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.trusted);
    }
    UmiStudioBuildProfilesDetach(services);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(first);
    umi_data_server_destroy(second);
    free(loaded);
    free(active);
    free(saved);
    return 0;
}
