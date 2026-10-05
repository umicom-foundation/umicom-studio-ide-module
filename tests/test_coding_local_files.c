/*-----------------------------------------------------------------------------
 * Umicom Framework
 * File: tests/test_coding_local_files.c
 * PURPOSE: Verify Studio-approved Unicode patch application and rollback through shared native file services.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "umicom/studio/coding_assistant.h"
#include "umicom/platform/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(x))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                          \
            exit(1);                                                                                         \
        }                                                                                                    \
    } while (0)

/* Reserve a new test-owned directory without reusing a prior run's source.
 * Native creation is independent of the coding adapter under test. */
static void Directory(char *root, const char *base)
{
#ifdef _WIN32
    unsigned long process = (unsigned long)GetCurrentProcessId();
#else
    unsigned long process = (unsigned long)getpid();
#endif
    for (unsigned attempt = 0U; attempt < 1000U; ++attempt)
    {
        int length =
            snprintf(root, UMI_PATH_CAPACITY, "%s/coding-caf\xc3\xa9-%lu-%u", base, process, attempt);
        CHECK(length > 0 && (size_t)length < UMI_PATH_CAPACITY);
#ifdef _WIN32
        wchar_t native[UMI_PATH_CAPACITY];
        CHECK(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, root, -1, native, (int)UMI_PATH_CAPACITY) >
              0);
        if (CreateDirectoryW(native, NULL))
            return;
        CHECK(GetLastError() == ERROR_ALREADY_EXISTS);
#else
        if (mkdir(root, 0700) == 0)
            return;
        CHECK(errno == EEXIST);
#endif
    }
    CHECK(0);
}
static void ChangeDirectory(const char *root)
{
#ifdef _WIN32
    wchar_t native[UMI_PATH_CAPACITY];
    CHECK(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, root, -1, native, (int)UMI_PATH_CAPACITY) > 0);
    CHECK(SetCurrentDirectoryW(native));
#else
    CHECK(chdir(root) == 0);
#endif
}
static void Bytes(const char *path, const char *expected)
{
    unsigned char *bytes = NULL;
    size_t length = 0U;
    CHECK(umi_fs_read_bytes(path, &bytes, &length) == UMI_STATUS_OK);
    CHECK(length == strlen(expected) && memcmp(bytes, expected, length) == 0);
    umi_fs_free_bytes(bytes);
}
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *mode = argv[1];
    if (strcmp(mode, "create") != 0 && strcmp(mode, "modify") != 0 && strcmp(mode, "conflict") != 0 &&
        strcmp(mode, "delete") != 0 && strcmp(mode, "binary") != 0 && strcmp(mode, "capture") != 0)
        return 2;
    char current[UMI_PATH_CAPACITY], root[UMI_PATH_CAPACITY], path[UMI_PATH_CAPACITY];
    CHECK(umi_fs_current_directory(current, sizeof(current)) == UMI_STATUS_OK);
    Directory(root, current);
    const char *leaf = "source-\xe6\x96\x87.c";
    CHECK(umi_path_join(root, leaf, path, sizeof(path)) == UMI_STATUS_OK);
    UmiStudioCodingWorkspace workspace;
    UmiAiCodingFileAdapter adapter;
    if (strcmp(mode, "capture") == 0)
        ChangeDirectory(root);
    CHECK(umi_studio_coding_workspace_adapter_init(&workspace, strcmp(mode, "capture") == 0 ? "." : root,
                                                   &adapter) == UMI_STATUS_OK);
    if (strcmp(mode, "capture") == 0)
        ChangeDirectory(current);
    CHECK(umi_path_is_absolute(workspace.root));
    if (strcmp(mode, "binary") == 0)
    {
        static const char data[] = {'a', '\0', 'b'};
        CHECK(umi_fs_write_bytes(path, data, sizeof(data)) == UMI_STATUS_OK);
        char output[16] = "previous";
        size_t length = 9U;
        CHECK(adapter.read(adapter.user_data, leaf, output, sizeof(output), &length) ==
              UMI_STATUS_PARSE_ERROR);
        CHECK(output[0] == '\0' && length == 0U);
        return 0;
    }
    int creating = strcmp(mode, "create") == 0 || strcmp(mode, "capture") == 0;
    int deleting = strcmp(mode, "delete") == 0;
    if (!creating)
        CHECK(umi_fs_write_text(path, "before\n") == UMI_STATUS_OK);
    /* Patch values are deliberately heap-owned: their bounded per-file text
     * arrays can exceed a native Windows thread's small stack. */
    UmiAiCodingPatch *patch = calloc(1U, sizeof(*patch));
    CHECK(patch != NULL);
    UmiAiCodingPatchPolicy policy = umi_ai_coding_patch_policy_default();
    policy.allow_delete = deleting;
    CHECK(umi_ai_coding_patch_init(patch, "local.patch", "local.request", "Edit source",
                                   "Exercise native reviewed file access") == UMI_STATUS_OK);
    CHECK(umi_ai_coding_patch_add_file(patch, leaf,
                                       creating   ? UMI_AI_CODING_PATCH_CREATE
                                       : deleting ? UMI_AI_CODING_PATCH_DELETE
                                                  : UMI_AI_CODING_PATCH_MODIFY,
                                       creating ? "" : "before\n",
                                       deleting ? "" : "after\n") == UMI_STATUS_OK);
    CHECK(umi_ai_coding_patch_apply(patch, &policy, &adapter) != UMI_STATUS_OK);
    if (creating)
        CHECK(!umi_fs_exists(path));
    else
        Bytes(path, "before\n");
    CHECK(umi_ai_coding_patch_approve(patch, "local.reviewer") == UMI_STATUS_OK);
    if (strcmp(mode, "conflict") == 0)
    {
        CHECK(umi_fs_write_text(path, "external\n") == UMI_STATUS_OK);
        CHECK(umi_ai_coding_patch_apply(patch, &policy, &adapter) != UMI_STATUS_OK);
        Bytes(path, "external\n");
    }
    else
    {
        CHECK(umi_ai_coding_patch_apply(patch, &policy, &adapter) == UMI_STATUS_OK);
        if (deleting)
            CHECK(!umi_fs_exists(path));
        else
            Bytes(path, "after\n");
        CHECK(umi_ai_coding_patch_revert(patch, &adapter) == UMI_STATUS_OK);
        if (creating)
            CHECK(!umi_fs_exists(path));
        else
            Bytes(path, "before\n");
    }
    free(patch);
    return 0;
}
