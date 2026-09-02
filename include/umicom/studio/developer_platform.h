/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_platform.h
 *
 * PURPOSE:
 *   Compose build, testing, terminal, language, debugging and source-control services for Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_DEVELOPER_PLATFORM_H
#define UMICOM_STUDIO_DEVELOPER_PLATFORM_H

#include <stddef.h>

#include "umicom/studio/build.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/language.h"
#include "umicom/studio/source_control.h"
#include "umicom/studio/terminal.h"
#include "umicom/studio/tests.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio developer platform data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperPlatform UmiStudioDeveloperPlatform;

/**
 * Represent the studio developer snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperSnapshot {
    UmiStudioBuildSnapshot build;
    UmiStudioTestSnapshot tests;
    UmiStudioTerminalSnapshot terminal;
    UmiStudioLanguageSnapshot language;
    UmiStudioDebuggerSnapshot debugger;
    UmiStudioSourceControlSnapshot source_control;
} UmiStudioDeveloperSnapshot;

/**
 * Initialise studio developer platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_platform_create(
    const char *workspace_root,
    UmiClock *clock,
    UmiStudioDeveloperPlatform **out_platform
);
/**
 * Release or reset state held by studio developer platform so the same storage can be
 * reused safely.
 */
void umi_studio_developer_platform_destroy(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_platform_snapshot(
    const UmiStudioDeveloperPlatform *platform,
    UmiStudioDeveloperSnapshot *out_snapshot
);
/**
 * Provide the studio developer platform report operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_platform_report(
    const UmiStudioDeveloperPlatform *platform,
    char *out_text,
    size_t capacity
);
/**
 * Provide the studio developer platform build operation used by this module and its client
 * applications.
 */
UmiStudioBuildService *umi_studio_developer_platform_build(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform tests operation used by this module and its client
 * applications.
 */
UmiStudioTestService *umi_studio_developer_platform_tests(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform terminal operation used by this module and its
 * client applications.
 */
UmiStudioTerminalService *umi_studio_developer_platform_terminal(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform language operation used by this module and its
 * client applications.
 */
UmiStudioLanguageService *umi_studio_developer_platform_language(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform debugger operation used by this module and its
 * client applications.
 */
UmiStudioDebuggerService *umi_studio_developer_platform_debugger(
    UmiStudioDeveloperPlatform *platform
);
/**
 * Provide the studio developer platform source control operation used by this module and
 * its client applications.
 */
UmiStudioSourceControlService *umi_studio_developer_platform_source_control(
    UmiStudioDeveloperPlatform *platform
);

#ifdef __cplusplus
}
#endif

#endif
