/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/developer_platform.c
 *
 * PURPOSE:
 *   Compose the Framework-backed developer services used by Studio tools and frontends.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/developer_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioDeveloperPlatform {
    UmiStudioBuildService *build;
    UmiStudioTestService *tests;
    UmiStudioTerminalService *terminal;
    UmiStudioLanguageService *language;
    UmiStudioDebuggerService *debugger;
    UmiStudioSourceControlService *source_control;
};

/* Provide the root to uri operation used by this module and its client applications. */
static UmiStatus root_to_uri(const char *root, char *out_uri, size_t capacity)
{
    size_t input_index;
    size_t output_index;
    const char *prefix;
    size_t prefix_length;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL || root[0] == '\0' || out_uri == NULL || capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    prefix = root[1] == ':' ? "file:///" : "file://";
    prefix_length = strlen(prefix);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (prefix_length + strlen(root) + 1U > capacity) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)memcpy(out_uri, prefix, prefix_length);
    output_index = prefix_length;
    /* Visit each bounded item once so every record receives the same rule. */
    for (input_index = 0U; root[input_index] != '\0'; ++input_index) {
        out_uri[output_index++] = root[input_index] == '\\'
            ? '/' : root[input_index];
    }
    out_uri[output_index] = '\0';
    return UMI_STATUS_OK;
}

/*
 * Initialise studio developer platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_platform_create(
    const char *workspace_root,
    UmiClock *clock,
    UmiStudioDeveloperPlatform **out_platform)
{
    UmiStudioDeveloperPlatform *platform;
    UmiStatus status;
    char root_uri[UMI_PROTOCOL_URI_CAPACITY];
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workspace_root == NULL || clock == NULL || out_platform == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_platform = NULL;
    platform = (UmiStudioDeveloperPlatform *)calloc(1U, sizeof(*platform));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = root_to_uri(workspace_root, root_uri, sizeof(root_uri));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_build_service_create(workspace_root, clock,
                                                 &platform->build);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_test_service_create(&platform->tests);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_terminal_service_create(workspace_root, clock,
                                                    &platform->terminal);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_language_service_create(root_uri,
                                                    &platform->language);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_debugger_service_create(&platform->debugger);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_source_control_service_create(
            workspace_root, &platform->source_control);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_developer_platform_destroy(platform);
        return status;
    }
    *out_platform = platform;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio developer platform so the same storage can be
 * reused safely.
 */
void umi_studio_developer_platform_destroy(
    UmiStudioDeveloperPlatform *platform)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return;
    umi_studio_source_control_service_destroy(platform->source_control);
    umi_studio_debugger_service_destroy(platform->debugger);
    umi_studio_language_service_destroy(platform->language);
    umi_studio_terminal_service_destroy(platform->terminal);
    umi_studio_test_service_destroy(platform->tests);
    umi_studio_build_service_destroy(platform->build);
    free(platform);
}

/*
 * Provide the studio developer platform snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_platform_snapshot(
    const UmiStudioDeveloperPlatform *platform,
    UmiStudioDeveloperSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    status = umi_studio_build_service_snapshot(platform->build,
                                               &out_snapshot->build);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_test_service_snapshot(platform->tests, &out_snapshot->tests);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_terminal_service_snapshot(platform->terminal, &out_snapshot->terminal);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_language_service_snapshot(platform->language, &out_snapshot->language);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_debugger_service_snapshot(platform->debugger, &out_snapshot->debugger);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_source_control_service_snapshot(platform->source_control, &out_snapshot->source_control);
    return status;
}

/*
 * Provide the studio developer platform report operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_platform_report(
    const UmiStudioDeveloperPlatform *platform,
    char *out_text,
    size_t capacity)
{
    UmiStudioDeveloperSnapshot snapshot;
    int written;
    UmiStatus status = umi_studio_developer_platform_snapshot(platform,
                                                              &snapshot);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (status != UMI_STATUS_OK || out_text == NULL || capacity == 0U) {
        return status != UMI_STATUS_OK ? status : UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(out_text, capacity,
        "Build profile: %s\nBuild history: %zu\n"
        "Test suites: %zu\nTests: %zu\n"
        "Terminal sessions: %zu\n"
        "Language documents: %zu\nLanguage messages: %zu\n"
        "Debugger breakpoints: %zu\nDebugger messages: %zu\n"
        "Source control: %s\nBranch: %s\nChanges: %zu\n",
        snapshot.build.profile_id,
        snapshot.build.history_count,
        snapshot.tests.suite_count,
        snapshot.tests.test_count,
        snapshot.terminal.session_count,
        snapshot.language.open_documents,
        snapshot.language.queued_messages,
        snapshot.debugger.breakpoint_count,
        snapshot.debugger.queued_messages,
        snapshot.source_control.available ? "available" : "unavailable",
        snapshot.source_control.branch,
        snapshot.source_control.changes);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK : UMI_STATUS_CAPACITY_EXCEEDED;
}

/*
 * Provide the studio developer platform build operation used by this module and its client
 * applications.
 */
UmiStudioBuildService *umi_studio_developer_platform_build(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->build : NULL;
}

/*
 * Provide the studio developer platform tests operation used by this module and its client
 * applications.
 */
UmiStudioTestService *umi_studio_developer_platform_tests(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->tests : NULL;
}

/*
 * Provide the studio developer platform terminal operation used by this module and its
 * client applications.
 */
UmiStudioTerminalService *umi_studio_developer_platform_terminal(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->terminal : NULL;
}

/*
 * Provide the studio developer platform language operation used by this module and its
 * client applications.
 */
UmiStudioLanguageService *umi_studio_developer_platform_language(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->language : NULL;
}

/*
 * Provide the studio developer platform debugger operation used by this module and its
 * client applications.
 */
UmiStudioDebuggerService *umi_studio_developer_platform_debugger(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->debugger : NULL;
}

/*
 * Provide the studio developer platform source control operation used by this module and
 * its client applications.
 */
UmiStudioSourceControlService *umi_studio_developer_platform_source_control(
    UmiStudioDeveloperPlatform *platform)
{
    return platform != NULL ? platform->source_control : NULL;
}
