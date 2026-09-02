/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/include/umicom/studio/navigation_history.h
 *
 * PURPOSE:
 *   Provide editor back/forward location history.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#ifndef UMICOM_STUDIO_NAVIGATION_HISTORY_H
#define UMICOM_STUDIO_NAVIGATION_HISTORY_H

#include <stddef.h>

#include "umicom/base/status.h"
#include "umicom/studio/workspace_document.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_NAVIGATION_MAX 128U

/**
 * Represent the studio navigation location data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioNavigationLocation {
    char path[UMI_STUDIO_WORKSPACE_PATH_CAPACITY];
    size_t line;
    size_t column;
} UmiStudioNavigationLocation;

/**
 * Represent the studio navigation history data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioNavigationHistory {
    UmiStudioNavigationLocation items[UMI_STUDIO_NAVIGATION_MAX];
    size_t count;
    size_t cursor;
} UmiStudioNavigationHistory;

/**
 * Initialise studio navigation history from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_navigation_history_init(UmiStudioNavigationHistory *history);

/**
 * Provide the studio navigation history push operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_navigation_history_push(
    UmiStudioNavigationHistory *history,
    const char *path,
    size_t line,
    size_t column
);

/**
 * Provide the studio navigation history current operation used by this module and its
 * client applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_current(
    const UmiStudioNavigationHistory *history
);

/**
 * Provide the studio navigation history back operation used by this module and its client
 * applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_back(
    UmiStudioNavigationHistory *history
);

/**
 * Provide the studio navigation history forward operation used by this module and its
 * client applications.
 */
const UmiStudioNavigationLocation *umi_studio_navigation_history_forward(
    UmiStudioNavigationHistory *history
);

#ifdef __cplusplus
}
#endif

#endif
