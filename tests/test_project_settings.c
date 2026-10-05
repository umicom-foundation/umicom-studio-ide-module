/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_project_settings.c
 * PURPOSE: Reopen project settings through real Studio and Framework services.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "umicom/studio/services.h"
#include "umicom/studio/build.h"
#include "umicom/studio/workspace.h"
#include "umicom/build/profile_store.h"
#include "umicom/platform/clock.h"
#include "umicom/platform/filesystem.h"
#define CHECK(test) do { if (!(test)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #test); return EXIT_FAILURE; } } while (0)

int main(int argc, char **argv)
{
    UmiStudioServices *services = NULL;
    UmiStudioServicesOptions options = {0};
    UmiDataServer *server = NULL;
    UmiBuildProfile profile, previous, loaded;
    UmiStudioWorkspaceSnapshot workspace;
    UmiClock clock = umi_clock_system();
    uint64_t revision = 0U;
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], folder[96], database[UMI_PATH_CAPACITY];
    int durable;
    CHECK(argc == 2);
    durable = strcmp(argv[1], "durable") == 0;
#ifndef UMICOM_HAS_SQLITE
    if (durable) return 77;
#endif
    CHECK(umi_fs_current_directory(cwd, sizeof(cwd)) == UMI_STATUS_OK);
    (void)snprintf(folder, sizeof(folder), "Umicom Notes-%" PRIu64, clock.wall_nanoseconds(&clock));
    CHECK(umi_fs_join(root, sizeof(root), cwd, folder) == UMI_STATUS_OK);
    CHECK(!umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    CHECK(umi_fs_join(database, sizeof(database), root, "settings.sqlite3") == UMI_STATUS_OK);
    CHECK((durable ? umi_data_server_create_sqlite(database, &server)
                   : umi_data_server_create_memory(&server)) == UMI_STATUS_OK);
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, server) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_open(services, root, 1, 0) == UMI_STATUS_OK);
    profile = *umi_studio_build_service_profile(umi_studio_services_build(services));
    CHECK(profile.run_program[0] == '\0');
    (void)snprintf(profile.run_program, sizeof(profile.run_program), "%s", "build/bin/umicom-notes");
    (void)snprintf(profile.run_argument, sizeof(profile.run_argument), "%s", "--notes \"meeting notes.txt\"");
    profile.parallel_jobs = 3U;
    /* Separate names must survive the same project-close and durable reopen
     * path as ordinary settings without granting workspace execution trust. */
    profile.preset[0] = '\0';
    strcpy(profile.configure_preset, "notes-configure");
    strcpy(profile.build_preset, "notes-build");
    strcpy(profile.test_preset, "notes-check");
    strcpy(profile.run_working_directory, "sample data");
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_INVALID_STATE);
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(umi_studio_services_build(services))));
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
    CHECK(!workspace.graph.trusted); /* Saved settings do not authorise execution. */
    /* A second writer saves revision 2 while this Studio still holds revision 1. */
    previous = profile;
    profile.parallel_jobs = 4U;
    CHECK(UmiBuildProfileStoreSave(server, &profile, 1U, &revision) == UMI_STATUS_OK && revision == 2U);
    CHECK(UmiStudioBuildProfileSave(services, &previous) == UMI_STATUS_INVALID_STATE);
    CHECK(umi_build_profile_equal(&previous, umi_studio_build_service_profile(umi_studio_services_build(services))));
    CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(umi_studio_services_build(services))));
    /* The dialog's immutable source root cannot redirect settings to a different project. */
    loaded = profile;
    (void)snprintf(loaded.source_directory, sizeof(loaded.source_directory), "%s", cwd);
    CHECK(UmiStudioBuildProfileSave(services, &loaded) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.trusted);
    UmiStudioBuildProfilesDetach(services);
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_INVALID_STATE);
    umi_studio_services_destroy(services); services = NULL;
    if (durable) {
        umi_data_server_destroy(server); server = NULL;
        CHECK(umi_data_server_create_sqlite(database, &server) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, server) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
    CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(umi_studio_services_build(services))));
    profile.run_program[0] = '\0';
    CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
    CHECK(UmiBuildProfileStoreLoad(server, root, &loaded, &revision) == UMI_STATUS_OK && revision == 3U);
    CHECK(loaded.run_program[0] == '\0');
    CHECK(strcmp(loaded.configure_preset, "notes-configure") == 0);
    CHECK(strcmp(loaded.build_preset, "notes-build") == 0);
    CHECK(strcmp(loaded.test_preset, "notes-check") == 0);
    CHECK(strcmp(loaded.run_working_directory, "sample data") == 0);
    /* Reject conflicting old and new fields before replacing the active
     * profile or publishing another revision to the shared settings store. */
    previous = loaded;
    strcpy(loaded.preset, "ambiguous-shared-name");
    CHECK(UmiStudioBuildProfileSave(services, &loaded) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_build_profile_equal(&previous,
        umi_studio_build_service_profile(umi_studio_services_build(services))));
    CHECK(UmiBuildProfileStoreLoad(server, root, &loaded, &revision) == UMI_STATUS_OK && revision == 3U);
    CHECK(umi_build_profile_equal(&previous, &loaded));
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.trusted);
    UmiStudioBuildProfilesDetach(services);
    umi_studio_services_destroy(services);
    umi_data_server_destroy(server);
    CHECK(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    return EXIT_SUCCESS;
}
