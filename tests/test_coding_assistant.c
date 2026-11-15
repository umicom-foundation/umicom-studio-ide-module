/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_coding_assistant.c
 *
 * PURPOSE:
 *   Verify Studio composes Framework coding task plans and approved patch
 *   apply/revert operations through its repository-root file adapter.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Assertions invoke the workflow, so keep them active in release qualification. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "umicom/studio/coding_assistant.h"

typedef struct RecoveryWorkspace {
    UmiAiCodingFileAdapter delegate;
    int fail_write, fail_remove;
} RecoveryWorkspace;

/* Report failure after the real adapter changed a file. This exercises the
 * thin Studio boundary without pretending that a failed callback did no work. */
static UmiStatus recovery_read(void *user, const char *path, char *out,
    size_t capacity, size_t *length)
{
    RecoveryWorkspace *workspace = user;
    return workspace->delegate.read(workspace->delegate.user_data, path, out, capacity, length);
}
static UmiStatus recovery_write(void *user, const char *path, const char *text, size_t length)
{
    RecoveryWorkspace *workspace = user;
    UmiStatus status = workspace->delegate.write(workspace->delegate.user_data, path, text, length);
    if (status == UMI_STATUS_OK && workspace->fail_write) {
        workspace->fail_write = 0;
        return UMI_STATUS_IO_ERROR;
    }
    return status;
}
static UmiStatus recovery_remove(void *user, const char *path)
{
    RecoveryWorkspace *workspace = user;
    UmiStatus status = workspace->delegate.remove(workspace->delegate.user_data, path);
    if (status == UMI_STATUS_OK && workspace->fail_remove) {
        workspace->fail_remove = 0;
        return UMI_STATUS_IO_ERROR;
    }
    return status;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioAiPlatform *platform = NULL;
    UmiAiCodingTaskPlan plan;
    UmiAiCodingPatch patch;
    UmiStudioCodingWorkspace workspace;
    UmiAiCodingFileAdapter adapter;
    RecoveryWorkspace recovery = {0};
    UmiAiCodingFileAdapter failing_adapter;
    UmiAiCodingPatchTransactionReport report = {0};
    FILE *file;
    char text[64] = {0};
    const char *path = "umicom-studio-coding-test.c";

    assert(umi_studio_ai_platform_create(&platform) == UMI_STATUS_OK);
    assert(umi_studio_ai_platform_coding_assistant(platform) != NULL);
    assert(umi_studio_coding_assistant_prepare(
        platform, "studio.request.48", UMI_AI_CODING_TASK_EXPLAIN,
        "Explain the AI platform ownership boundary.",
        "applications/studio/src/app/ai_platform.c", &plan)
        == UMI_STATUS_OK);
    assert(plan.ready && plan.repository_context.file_count > 0U);

    assert(umi_ai_coding_patch_init(
        &patch, "studio.patch.48", "studio.request.48", "Create test file",
        "Exercise controlled creation and reversion") == UMI_STATUS_OK);
    assert(umi_ai_coding_patch_add_file(
        &patch, path, UMI_AI_CODING_PATCH_CREATE, "", "int value = 48;\n")
        == UMI_STATUS_OK);
    assert(umi_studio_coding_assistant_record_patch(platform, &patch)
           == UMI_STATUS_OK);
    assert(umi_studio_coding_assistant_approve_patch(
        platform, patch.patch_id, "test.user") == UMI_STATUS_OK);
    assert(umi_studio_coding_workspace_adapter_init(
        &workspace, ".", &adapter) == UMI_STATUS_OK);
/* Exercise detailed Framework reports through Studio, including a write that changes bytes before failing. The legacy assertion remains for comparison. The former implementation is retained for owner review. */
#if 0
    assert(umi_studio_coding_assistant_apply_patch(
        platform, patch.patch_id, &adapter) == UMI_STATUS_OK);
#endif
    recovery.delegate = adapter;
    failing_adapter = (UmiAiCodingFileAdapter){sizeof(UmiAiCodingFileAdapter), UMI_AI_CODING_ABI_VERSION,
        recovery_read, recovery_write, recovery_remove, &recovery};
    assert(UmiStudioCodingAssistantApplyPatchWithReport(NULL, patch.patch_id, &adapter, &report)
        == UMI_STATUS_INVALID_ARGUMENT);
    assert(report.admitted == 0);
    recovery.fail_write = 1;
    assert(UmiStudioCodingAssistantApplyPatchWithReport(platform, patch.patch_id, &failing_adapter, &report)
        == UMI_STATUS_IO_ERROR);
    assert(report.admitted == 1 && report.primary_status == UMI_STATUS_IO_ERROR);
    assert(report.mutation_attempts == 1U && report.mutations_completed == 0U);
    assert(report.compensation_attempts == 1U && report.compensation_failures == 0U);
    assert(report.restoration_checks == 1U && report.restoration_failures == 0U);
    assert(report.revision_after == report.revision_before + 1U);
    assert(fopen(path, "rb") == NULL);
    memset(&report, 0, sizeof(report));
    assert(UmiStudioCodingAssistantApplyPatchWithReport(platform, patch.patch_id, &adapter, &report)
        == UMI_STATUS_OK);
    assert(report.admitted == 1 && report.primary_status == UMI_STATUS_OK);
    assert(report.mutation_attempts == 1U && report.mutations_completed == 1U);
    assert(report.compensation_attempts == 0U);
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fread(text, 1U, sizeof(text) - 1U, file) > 0U);
    (void)fclose(file);
    assert(strcmp(text, "int value = 48;\n") == 0);
/* Observe deletion failure and restoration before retrying a checked revert through the shared Framework service. The former implementation is retained for owner review. */
#if 0
    assert(umi_studio_coding_assistant_revert_patch(
        platform, patch.patch_id, &adapter) == UMI_STATUS_OK);
#endif
    recovery.fail_remove = 1;
    memset(&report, 0, sizeof(report));
    assert(UmiStudioCodingAssistantRevertPatchWithReport(platform, patch.patch_id, &failing_adapter, &report)
        == UMI_STATUS_IO_ERROR);
    assert(report.admitted == 1 && report.primary_status == UMI_STATUS_IO_ERROR);
    assert(report.mutation_attempts == 1U && report.mutations_completed == 0U);
    assert(report.compensation_attempts == 1U && report.compensation_failures == 0U);
    assert(report.restoration_checks == 1U && report.restoration_failures == 0U);
    assert(report.revision_after == report.revision_before + 1U);
    file = fopen(path, "rb");
    assert(file != NULL);
    memset(text, 0, sizeof(text));
    assert(fread(text, 1U, sizeof(text) - 1U, file) > 0U);
    (void)fclose(file);
    assert(strcmp(text, "int value = 48;\n") == 0);
    memset(&report, 0, sizeof(report));
    assert(UmiStudioCodingAssistantRevertPatchWithReport(platform, patch.patch_id, &adapter, &report)
        == UMI_STATUS_OK);
    assert(report.admitted == 1 && report.mutation_attempts == 1U && report.mutations_completed == 1U);
    assert(report.compensation_attempts == 0U);
    assert(fopen(path, "rb") == NULL);
    umi_studio_ai_platform_destroy(platform);
    return 0;
}
