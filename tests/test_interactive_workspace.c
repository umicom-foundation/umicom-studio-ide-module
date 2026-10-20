/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_interactive_workspace.c
 * PURPOSE: Check project adoption independently from asynchronous file indexing.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace.h"
#include "umicom/studio/build.h"
#include "umicom/studio/source_control.h"
#include "umicom/studio/debugger.h"
#include "umicom/platform/clock.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *name = argv[1];
    const char *cases[] = {"prepare", "scan", "saved-profile", "missing-folder", "close-scan", "sync-compatibility", "repository-binding", "repository-close", "debug-open", "debug-close", "debug-profile", "debug-sync-open", "relative-binding", "missing-preserves-watch", "refresh-closed", "refresh-new-files", "refresh-close"};
    int known = 0;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(name, cases[i]) == 0) known = 1;
    if (!known) return 2;
    int failed = 0, created = 0;
    UmiStudioServices *services = NULL;
    UmiDataServer *storage = NULL;
    UmiStudioServicesOptions options = {0};
    UmiClock clock = umi_clock_system();
    char cwd[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY];
    char folder[128];
    CHECK(umi_fs_current_directory(cwd, sizeof cwd) == UMI_STATUS_OK);
    (void)snprintf(folder, sizeof folder, "interactive-%s-%" PRIu64, name, clock.wall_nanoseconds(&clock));
    CHECK(umi_fs_join(root, sizeof root, cwd, folder) == UMI_STATUS_OK && !umi_fs_exists(root));
    CHECK(umi_fs_make_directories(root) == UMI_STATUS_OK);
    created = 1;
    CHECK(umi_fs_join(source, sizeof source, root, "main.c") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(source, "int main(void) { return 0; }\n") == UMI_STATUS_OK);
    CHECK(umi_fs_join(source, sizeof source, root, "CMakeLists.txt") == UMI_STATUS_OK);
    CHECK(umi_fs_write_text(source, "cmake_minimum_required(VERSION 3.25)\nproject(Notes C)\n") == UMI_STATUS_OK);
    CHECK(umi_data_server_create_memory(&storage) == UMI_STATUS_OK);
    CHECK(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildProfilesBind(services, storage) == UMI_STATUS_OK);
    UmiFileIndex *index = umi_studio_services_file_index(services);
    UmiStudioWorkspaceSnapshot workspace;
    UmiFileIndexRefreshSnapshot refresh;
    UmiStatus indexStatus = UMI_STATUS_UNAVAILABLE;
    if (strcmp(name, "refresh-closed") == 0) {
        CHECK(UmiStudioWorkspaceRefreshForEditing(services) == UMI_STATUS_INVALID_STATE);
        CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.requestId == 0U);
    } else if (strcmp(name, "refresh-new-files") == 0 || strcmp(name, "refresh-close") == 0) {
        CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_file_index_stats(index).files == 2U);
        CHECK(umi_fs_join(source, sizeof source, root, "later.c") == UMI_STATUS_OK);
        CHECK(umi_fs_write_text(source, "/* discovered after initial open */\n") == UMI_STATUS_OK);
        CHECK(UmiStudioWorkspaceRefreshForEditing(services) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.requestId != 0U);
        if (strcmp(name, "refresh-close") == 0)
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        if (strcmp(name, "refresh-close") == 0)
            CHECK(!workspace.graph.open && workspace.files.files == 0U);
        else {
            CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.status == UMI_STATUS_OK);
            CHECK(workspace.graph.open && workspace.files.files == 3U);
        }
    } else if (strncmp(name, "debug-", 6U) == 0) {
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, 1) == UMI_STATUS_OK);
        UmiStudioBuildService *build = umi_studio_services_build(services);
        UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
        UmiBuildProfile profile = *umi_studio_build_service_profile(build);
        strcpy(profile.run_program, "reviewed-target");
        CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        UmiStudioWorkspaceSnapshot before = workspace;
        /* Queueing captures ownership but starts neither compiler nor debugger.
         * This models the interval before the background build is admitted. */
        CHECK(UmiStudioDebuggerQueueNative(debugger, build, &profile) == UMI_STATUS_OK);
        CHECK(UmiStudioDebuggerNativeBusy(debugger) && !UmiStudioBuildBusy(build));
        CHECK(umi_fs_join(source, sizeof source, root, "Second project") == UMI_STATUS_OK);
        CHECK(umi_fs_make_directories(source) == UMI_STATUS_OK);
        if (strcmp(name, "debug-close") == 0)
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_BUSY);
        else if (strcmp(name, "debug-profile") == 0) {
            UmiBuildProfile replacement = profile;
            replacement.parallel_jobs = 3U;
            CHECK(UmiStudioBuildProfileSave(services, &replacement) == UMI_STATUS_BUSY);
        } else if (strcmp(name, "debug-sync-open") == 0)
            CHECK(umi_studio_workspace_open(services, source, 0, 0) == UMI_STATUS_BUSY);
        else
            CHECK(UmiStudioServicesPrepareWorkspace(services, source, 0) == UMI_STATUS_BUSY);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.open && workspace.graph.trusted &&
            workspace.graph.revision == before.graph.revision && umi_path_equal(workspace.graph.root, root));
        CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(build)));
        UmiStudioSourceControlSnapshot repository;
        CHECK(umi_studio_source_control_service_snapshot(
            umi_studio_services_source_control(services), &repository) == UMI_STATUS_OK);
        CHECK(umi_path_equal(repository.root, root));
        CHECK(umi_studio_debugger_service_stop(debugger, 0) == UMI_STATUS_OK);
        CHECK(!UmiStudioDebuggerNativeBusy(debugger));
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, 0) == UMI_STATUS_OK);
        CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(build)));
        CHECK(UmiStudioServicesPrepareWorkspace(services, source, 0) == UMI_STATUS_OK);
    } else if (strcmp(name, "relative-binding") == 0) {
        CHECK(UmiStudioServicesPrepareWorkspace(services, folder, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(umi_path_equal(workspace.graph.root, root) && umi_path_equal(workspace.files.root, root));
        CHECK(umi_path_is_absolute(workspace.files.root));
        CHECK(UmiFileIndexRefreshStart(index, workspace.files.revision) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
        CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.status == UMI_STATUS_OK);
        CHECK(umi_path_equal(refresh.sourceRoot, root) && umi_file_index_stats(index).files == 2U);
    } else if (strcmp(name, "missing-preserves-watch") == 0) {
        CHECK(umi_studio_workspace_open(services, root, 0, 1) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        UmiStudioWorkspaceSnapshot before = workspace;
        CHECK(umi_fs_join(source, sizeof source, root, "missing-project") == UMI_STATUS_OK);
        CHECK(UmiStudioServicesPrepareWorkspace(services, source, 0) == UMI_STATUS_NOT_FOUND);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.revision == before.graph.revision &&
            umi_path_equal(workspace.graph.root, before.graph.root));
        CHECK(workspace.files.revision == before.files.revision && workspace.files.files == before.files.files);
        CHECK(workspace.watcher.running == before.watcher.running && workspace.watcher.running);
    } else if (strcmp(name, "prepare") == 0) {
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.open && !workspace.graph.trusted);
        CHECK(workspace.files.files == 0U && workspace.files.rebuilds == 0U);
        CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.requestId == 0U);
    } else if (strcmp(name, "sync-compatibility") == 0) {
        CHECK(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.files.files == 2U && workspace.files.rebuilds != 0U);
        CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && refresh.requestId == 0U);
    } else if (strcmp(name, "repository-binding") == 0 || strcmp(name, "repository-close") == 0) {
        /* Preparation never needs a Git executable. These assertions exercise
         * project identity and teardown without granting discovery permission. */
        UmiStudioSourceControlService *repository = umi_studio_services_source_control(services);
        UmiStudioSourceControlSnapshot state;
        CHECK(UmiStudioServicesPrepareWorkspace(services, root, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_source_control_service_snapshot(repository, &state) == UMI_STATUS_OK);
        CHECK(!state.available && umi_path_equal(state.root, root));
        CHECK(umi_fs_join(source, sizeof source, root, "Next project") == UMI_STATUS_OK);
        CHECK(umi_fs_make_directories(source) == UMI_STATUS_OK);
        CHECK(UmiStudioServicesPrepareWorkspace(services, source, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_source_control_service_snapshot(repository, &state) == UMI_STATUS_OK);
        CHECK(!state.available && umi_path_equal(state.root, source));
        CHECK(state.changes == 0U && state.staged == 0U && state.commits == 0U);
        CHECK(UmiStudioSourceControlSelectWorkspace(repository, "") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(UmiStudioSourceControlSelectWorkspace(repository, "relative-project") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(umi_studio_source_control_service_snapshot(repository, &state) == UMI_STATUS_OK);
        CHECK(umi_path_equal(state.root, source));
        if (strcmp(name, "repository-close") == 0) {
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
            CHECK(umi_studio_source_control_service_snapshot(repository, &state) == UMI_STATUS_OK);
            CHECK(!state.available && state.root[0] == '\0');
            CHECK(umi_studio_source_control_service_workspace(repository) == NULL);
            CHECK(umi_studio_source_control_service_refresh(repository, 10U) == UMI_STATUS_UNAVAILABLE);
        }
    } else {
        /* Relative input is resolved before the worker is admitted. */
        CHECK(UmiStudioWorkspaceOpenForEditing(services, folder, 0, 0, &indexStatus) == UMI_STATUS_OK);
        CHECK(indexStatus == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.open && !workspace.graph.trusted && umi_path_equal(workspace.graph.root, root));
        CHECK(umi_path_equal(umi_studio_build_service_profile(
            umi_studio_services_build(services))->source_directory, root));
        if (strcmp(name, "close-scan") == 0) {
            CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
            CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
            CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.open);
            CHECK(workspace.files.files == 0U); /* No late publication into a closed workspace. */
        } else {
            CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
            CHECK(UmiFileIndexRefreshRead(index, &refresh) == UMI_STATUS_OK && !refresh.active);
            CHECK(refresh.status == UMI_STATUS_OK && umi_path_equal(refresh.sourceRoot, root));
            CHECK(umi_file_index_stats(index).files == 2U);
            if (strcmp(name, "saved-profile") == 0) {
                UmiBuildProfile profile = *umi_studio_build_service_profile(umi_studio_services_build(services));
                profile.parallel_jobs = 3U;
                strcpy(profile.build_directory, "build/selected-output");
                CHECK(UmiStudioBuildProfileSave(services, &profile) == UMI_STATUS_OK);
                CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
                CHECK(UmiStudioWorkspaceOpenForEditing(services, root, 0, 0, &indexStatus) == UMI_STATUS_OK);
                CHECK(indexStatus == UMI_STATUS_OK);
                CHECK(umi_build_profile_equal(&profile, umi_studio_build_service_profile(umi_studio_services_build(services))));
                CHECK(UmiFileIndexRefreshWait(index, 10000U) == UMI_STATUS_OK);
                CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && !workspace.graph.trusted);
            } else if (strcmp(name, "missing-folder") == 0) {
                UmiBuildProfile previous = *umi_studio_build_service_profile(umi_studio_services_build(services));
                CHECK(umi_fs_join(source, sizeof source, root, "missing-project") == UMI_STATUS_OK);
                CHECK(UmiStudioWorkspaceOpenForEditing(services, source, 0, 0, &indexStatus) == UMI_STATUS_NOT_FOUND);
                CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK && workspace.graph.open);
                CHECK(umi_path_equal(workspace.graph.root, root));
                CHECK(umi_build_profile_equal(&previous, umi_studio_build_service_profile(umi_studio_services_build(services))));
            }
        }
    }
cleanup:
    if (services != NULL) {
        UmiStudioBuildProfilesDetach(services);
        umi_studio_services_destroy(services); /* Cancels and joins its owned index before freeing it. */
    }
    umi_data_server_destroy(storage);
    if (!failed && created) (void)umi_fs_remove_tree(root);
    return failed;
}
