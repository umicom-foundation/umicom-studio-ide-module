/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_configurations.c
 * PURPOSE: Check configuration review against workspace changes, storage rebinding and active settings.
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
    UmiBuildProfile *profile = malloc(sizeof *profile), *original = malloc(sizeof *original),
                    *loaded = malloc(sizeof *loaded);
    CHECK(profile != NULL && original != NULL && loaded != NULL);
    *original = *umi_studio_build_service_profile(umi_studio_services_build(services));
    *profile = *original;
    strcpy(profile->configuration, "Release");
    strcpy(profile->run_environment, "APP_MODE=review");
    UmiBuildConfigurationCatalogue catalogue;
    CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) == UMI_STATUS_OK);
    CHECK(catalogue.count == 0U && catalogue.revision == 0U);
    uint64_t revision = 0U;
    CHECK(UmiStudioBuildConfigurationSave(services, &context, "Release review", profile, 0U,
                                          &revision) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildConfigurationLoad(services, &context, "Release review", revision, loaded) ==
          UMI_STATUS_OK);
    CHECK(umi_build_profile_equal(profile, loaded));
    CHECK(umi_build_profile_equal(
        original, umi_studio_build_service_profile(umi_studio_services_build(services))));
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
          !workspace.graph.trusted);
    if (strcmp(argv[1], "rebind") == 0)
    {
        CHECK(UmiStudioBuildProfilesBind(services, second) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
              UMI_STATUS_INVALID_STATE);
        CHECK(UmiStudioBuildConfigurationSave(services, &context, "Old form", profile, 0U,
                                              &revision) == UMI_STATUS_INVALID_STATE);
        CHECK(UmiStudioBuildConfigurationContextRead(services, &context) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
                  UMI_STATUS_OK &&
              catalogue.count == 0U);
        UmiStudioBuildProfilesDetach(services);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
              UMI_STATUS_INVALID_STATE);
    }
    else if (strcmp(argv[1], "workspace") == 0)
    {
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
              UMI_STATUS_INVALID_STATE);
        CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
              UMI_STATUS_INVALID_STATE);
        CHECK(UmiStudioBuildConfigurationContextRead(services, &context) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationsCapture(services, &context, &catalogue) ==
                  UMI_STATUS_OK &&
              catalogue.count == 1U);
    }
    else if (strcmp(argv[1], "trust") == 0)
    {
        CHECK(umi_studio_workspace_set_trusted(services, 1) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationLoad(services, &context, "Release review", revision,
                                              loaded) == UMI_STATUS_INVALID_STATE);
        CHECK(UmiStudioBuildConfigurationContextRead(services, &context) == UMI_STATUS_OK);
        CHECK(UmiStudioBuildConfigurationLoad(services, &context, "Release review", revision,
                                              loaded) == UMI_STATUS_OK);
    }
    else
    {
        CHECK(strcmp(argv[1], "review") == 0);
        CHECK(UmiStudioBuildProfileSave(services, loaded) == UMI_STATUS_OK);
        CHECK(umi_build_profile_equal(
            loaded, umi_studio_build_service_profile(umi_studio_services_build(services))));
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
              !workspace.graph.trusted);
        strcpy(profile->source_directory, "./another-project");
        CHECK(UmiStudioBuildConfigurationSave(services, &context, "Wrong root", profile, revision,
                                              &revision) == UMI_STATUS_INVALID_ARGUMENT);
    }
    UmiStudioBuildProfilesDetach(services);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(first);
    umi_data_server_destroy(second);
    free(loaded);
    free(original);
    free(profile);
    return 0;
}
