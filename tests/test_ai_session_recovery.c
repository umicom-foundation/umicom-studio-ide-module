/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_ai_session_recovery.c
 *
 * PURPOSE:
 *   Exercise real Framework-backed Studio session adoption and bounded-file refusal without model activation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "umicom/studio/ai_platform.h"
#include "umicom/platform/filesystem.h"

int main(int argc, char **argv)
{
    static const char *const modes[] = {
        "insert", "replace", "conflict", "stale", "invalid", "null-platform",
        "null-record", "null-report", "file", "malformed", "file-limit",
        "relative", "zero-budget", "policy"
    };
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    UmiStudioAiPlatform *platform = NULL;
    UmiAiAuthorEngineService *engine;
    UmiAiConversationRecord *record = calloc(1U, sizeof *record);
    UmiAiConversationRecord *stored = malloc(sizeof *stored);
    UmiAiConversationImportReport report, before_report;
    UmiAiAuthorEngineServiceSnapshot before, after;
    UmiAiPolicy before_policy;
    UmiAiPrivacyPolicy before_privacy;
    UmiStatus status;
    uint64_t expected = 0U;
    int known = 0, file_case, success;
    assert(argc == 3 && record != NULL && stored != NULL);
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (strcmp(argv[1], modes[i]) == 0) known = 1;
    assert(known);
#define CASE(name) (strcmp(argv[1], name) == 0)
    file_case = CASE("file") || CASE("malformed") || CASE("file-limit");
    success = CASE("insert") || CASE("replace") || CASE("file") || CASE("policy");
    /* Recovery must not enable persistence or network access. */
    config.persist_sessions = 0; config.allow_remote = 0;
    assert(umi_studio_ai_platform_create_configured(&config, &platform) == UMI_STATUS_OK);
    engine = umi_studio_ai_platform_authorengine(platform);
    assert(engine != NULL);
    assert(umi_studio_ai_platform_snapshot(platform, &before) == UMI_STATUS_OK);
    memcpy(&before_policy, &umi_studio_ai_platform_runtime(platform)->policy, sizeof before_policy);
    memcpy(&before_privacy, umi_ai_authorengine_service_privacy_policy(engine), sizeof before_privacy);
    assert(umi_ai_conversation_record_init(record, "session.recovered", "provider.offline",
        "model.unavailable", "workspace.recovered", "Recovered title", 7U) == UMI_STATUS_OK);
    record->revision = 900U;
    if (CASE("replace") || CASE("conflict") || CASE("stale")) {
        strcpy(record->session.session_id, before.active_session_id);
        strcpy(record->conversation.conversation_id, before.active_session_id);
/* Use the existing public archive accessor and lookup; the earlier draft named an unavailable service convenience function and is retained for review. The former implementation is retained for owner review. */
#if 0
        assert(umi_ai_authorengine_service_read_session(engine, before.active_session_id, stored) == UMI_STATUS_OK);
#endif
        assert(umi_ai_conversation_archive_find(
            umi_ai_authorengine_service_conversations(engine), before.active_session_id, stored) == UMI_STATUS_OK);
        if (CASE("replace")) expected = stored->revision;
        if (CASE("stale")) expected = stored->revision + 1U;
    }
    if (CASE("invalid")) memset(record->title, 'x', sizeof record->title);
    if (CASE("policy")) {
        record->classification = UMI_AI_DATA_CONFIDENTIAL;
        record->remote_approved = 1; record->persistence_approved = 1;
    }
    memset(&report, 0xA5, sizeof report);
    memcpy(&before_report, &report, sizeof report);
    if (file_case) {
        UmiAiPrivacyPolicy fixture_policy = umi_ai_privacy_policy_default();
        if (CASE("malformed"))
            assert(umi_fs_write_text(argv[2], "format=umicom-ai-session-v1\n") == UMI_STATUS_OK);
        else
            assert(umi_ai_conversation_archive_save_record(record, &fixture_policy, argv[2]) == UMI_STATUS_OK);
        status = UmiStudioAiPlatformLoadSessionBounded(platform, argv[2],
            CASE("file-limit") ? 1U : UMI_AI_CONVERSATION_FILE_MAX_BYTES, expected, &report);
    } else if (CASE("relative") || CASE("zero-budget")) {
        status = UmiStudioAiPlatformLoadSessionBounded(platform,
            CASE("relative") ? "relative.session" : argv[2],
            CASE("zero-budget") ? 0U : UMI_AI_CONVERSATION_FILE_MAX_BYTES, 0U, &report);
    } else {
        status = UmiStudioAiPlatformImportSession(CASE("null-platform") ? NULL : platform,
            CASE("null-record") ? NULL : record, expected, CASE("null-report") ? NULL : &report);
    }
    assert(umi_studio_ai_platform_snapshot(platform, &after) == UMI_STATUS_OK);
    assert(memcmp(&before_policy, &umi_studio_ai_platform_runtime(platform)->policy, sizeof before_policy) == 0);
    assert(memcmp(&before_privacy, umi_ai_authorengine_service_privacy_policy(engine), sizeof before_privacy) == 0);
    assert(after.runtimes == before.runtimes && after.healthy_runtimes == before.healthy_runtimes &&
        after.providers == before.providers && after.context_sources == before.context_sources);
    if (success) {
        assert(status == UMI_STATUS_OK && report.source_record_revision == 900U);
        assert(report.service_revision == before.revision + 1U && after.revision == report.service_revision);
        assert(report.replaced == CASE("replace"));
        assert(after.sessions == before.sessions + (CASE("replace") ? 0U : 1U));
        assert(strcmp(after.active_session_id, report.session_id) == 0);
/* Use the existing public archive accessor and lookup; the earlier draft named an unavailable service convenience function and is retained for review. The former implementation is retained for owner review. */
#if 0
        assert(umi_ai_authorengine_service_read_session(engine, report.session_id, stored) == UMI_STATUS_OK);
#endif
        assert(umi_ai_conversation_archive_find(
            umi_ai_authorengine_service_conversations(engine), report.session_id, stored) == UMI_STATUS_OK);
        assert(stored->revision == report.stored_record_revision && stored->revision != 900U);
        assert(strcmp(stored->title, "Recovered title") == 0);
        if (CASE("policy")) {
            assert(stored->remote_approved && stored->persistence_approved);
            /* Record flags do not relax the owner's persistence policy. */
            assert(umi_studio_ai_platform_save_session(platform, report.session_id, argv[2]) == UMI_STATUS_PERMISSION_DENIED);
        }
    } else {
        UmiStatus wanted = CASE("conflict") || CASE("stale") ? UMI_STATUS_BUSY :
            CASE("malformed") ? UMI_STATUS_PARSE_ERROR :
            CASE("file-limit") ? UMI_STATUS_CAPACITY_EXCEEDED : UMI_STATUS_INVALID_ARGUMENT;
        assert(status == wanted);
        assert(memcmp(&before_report, &report, sizeof report) == 0);
        assert(memcmp(&before, &after, sizeof before) == 0);
    }
    if (CASE("null-platform")) {
        assert(UmiStudioAiPlatformLoadSessionBounded(NULL, argv[2],
            UMI_AI_CONVERSATION_FILE_MAX_BYTES, 0U, &report) == UMI_STATUS_INVALID_ARGUMENT);
        assert(memcmp(&before_report, &report, sizeof report) == 0);
    }
    if (file_case) assert(umi_fs_remove_tree(argv[2]) == UMI_STATUS_OK);
    umi_studio_ai_platform_destroy(platform);
    free(record); free(stored);
    return 0;
#undef CASE
}
