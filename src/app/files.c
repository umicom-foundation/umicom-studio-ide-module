/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/files.c
 *
 * PURPOSE:
 *   Implement workspace-confined file operations, trust checks and index
 *   maintenance on top of the portable Framework filesystem contracts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/files.h"

#include <string.h>

#include "umicom/platform/atomic_file.h"
#include "umicom/platform/directory.h"

/*
 * Provide the workspace snapshot operation used by this module and its client
 * applications.
 */
static UmiStatus workspace_snapshot(UmiStudioServices *services,
                                    UmiWorkspaceGraphSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_workspace_graph_snapshot(
        umi_studio_services_workspace(services), out_snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return out_snapshot->open ? UMI_STATUS_OK : UMI_STATUS_INVALID_STATE;
}

/* Provide the require trusted operation used by this module and its client applications. */
static UmiStatus require_trusted(UmiStudioServices *services,
                                 UmiWorkspaceGraphSnapshot *out_snapshot)
{
    UmiStatus status = workspace_snapshot(services, out_snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return out_snapshot->trusted
        ? UMI_STATUS_OK
        : UMI_STATUS_PERMISSION_DENIED;
}

/*
 * Provide the studio files resolve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_resolve(UmiStudioServices *services,
                                   const char *workspace_path,
                                   char *out_path,
                                   size_t capacity)
{
    UmiWorkspaceGraphSnapshot snapshot;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workspace_path == NULL || workspace_path[0] == '\0' ||
        out_path == NULL || capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = workspace_snapshot(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_path_absolute(workspace_path,
                               snapshot.root,
                               out_path,
                               capacity);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_path_is_within(snapshot.root, out_path)
        ? UMI_STATUS_OK
        : UMI_STATUS_PERMISSION_DENIED;
}

/*
 * Provide the studio files read text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_read_text(UmiStudioServices *services,
                                     const char *workspace_path,
                                     char **out_text,
                                     size_t *out_size)
{
    char path[UMI_PATH_CAPACITY];
    UmiStatus status = umi_studio_files_resolve(services,
                                                workspace_path,
                                                path,
                                                sizeof(path));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_fs_read_text(path, out_text, out_size);
}

/*
 * Provide the studio files write text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_write_text(UmiStudioServices *services,
                                      const char *workspace_path,
                                      const char *text)
{
    UmiWorkspaceGraphSnapshot snapshot;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = require_trusted(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_files_resolve(services,
                                      workspace_path,
                                      path,
                                      sizeof(path));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK || umi_path_equal(snapshot.root, path)) {
        return status != UMI_STATUS_OK ? status : UMI_STATUS_PERMISSION_DENIED;
    }
    status = umi_atomic_file_write_text(path, text);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_update(
            umi_studio_services_file_index(services), path);
    }
    return status;
}

/*
 * Provide the studio files append text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_append_text(UmiStudioServices *services,
                                       const char *workspace_path,
                                       const char *text)
{
    UmiWorkspaceGraphSnapshot snapshot;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = require_trusted(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_files_resolve(services,
                                      workspace_path,
                                      path,
                                      sizeof(path));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK || umi_path_equal(snapshot.root, path)) {
        return status != UMI_STATUS_OK ? status : UMI_STATUS_PERMISSION_DENIED;
    }
    status = umi_fs_append_text(path, text);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_update(
            umi_studio_services_file_index(services), path);
    }
    return status;
}

/*
 * Provide the studio files create directory operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_create_directory(UmiStudioServices *services,
                                            const char *workspace_path)
{
    UmiWorkspaceGraphSnapshot snapshot;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status = require_trusted(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_files_resolve(services,
                                      workspace_path,
                                      path,
                                      sizeof(path));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK || umi_path_equal(snapshot.root, path)) {
        return status != UMI_STATUS_OK ? status : UMI_STATUS_ALREADY_EXISTS;
    }
    return umi_fs_make_directories(path);
}

/*
 * Provide the studio files rename operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_rename(UmiStudioServices *services,
                                  const char *source_workspace_path,
                                  const char *destination_workspace_path)
{
    UmiWorkspaceGraphSnapshot snapshot;
    UmiFileInfo source_info;
    char source[UMI_PATH_CAPACITY];
    char destination[UMI_PATH_CAPACITY];
    UmiStatus status = require_trusted(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_files_resolve(services,
                                      source_workspace_path,
                                      source,
                                      sizeof(source));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_files_resolve(services,
                                          destination_workspace_path,
                                          destination,
                                          sizeof(destination));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (umi_path_equal(snapshot.root, source) ||
        umi_path_equal(snapshot.root, destination)) {
        return UMI_STATUS_PERMISSION_DENIED;
    }
    status = umi_directory_stat(source, &source_info);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_fs_rename(source, destination);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Apply this branch only when its contract condition is satisfied. */
    if (source_info.kind == UMI_FILE_KIND_REGULAR) {
        (void)umi_file_index_remove(
            umi_studio_services_file_index(services), source);
        return umi_file_index_update(
            umi_studio_services_file_index(services), destination);
    }
    return umi_file_index_rebuild(
        umi_studio_services_file_index(services));
}

/*
 * Remove studio files while keeping the remaining records in a valid and discoverable
 * state.
 */
UmiStatus umi_studio_files_remove(UmiStudioServices *services,
                                  const char *workspace_path)
{
    UmiWorkspaceGraphSnapshot snapshot;
    UmiFileInfo info;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status = require_trusted(services, &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_files_resolve(services,
                                      workspace_path,
                                      path,
                                      sizeof(path));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (umi_path_equal(snapshot.root, path)) {
        return UMI_STATUS_PERMISSION_DENIED;
    }
    status = umi_directory_stat(path, &info);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_fs_remove_tree(path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Apply this branch only when its contract condition is satisfied. */
    if (info.kind == UMI_FILE_KIND_REGULAR) {
        status = umi_file_index_remove(
            umi_studio_services_file_index(services), path);
        return status == UMI_STATUS_NOT_FOUND ? UMI_STATUS_OK : status;
    }
    return umi_file_index_rebuild(
        umi_studio_services_file_index(services));
}
