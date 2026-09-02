/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/include/umicom/studio/runtime_filter.h
 *
 * PURPOSE:
 *   Define deterministic search/category/state filtering for the Application Hub.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#ifndef UMICOM_STUDIO_RUNTIME_FILTER_H
#define UMICOM_STUDIO_RUNTIME_FILTER_H

#include <stdbool.h>

#include "umicom/studio/runtime_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio runtime filter data shared with callers of this public contract.
 */
typedef struct UmiStudioRuntimeFilter {
    char query[UMI_INTEGRATION_TEXT_CAPACITY];
    UmiStudioRuntimeCategory category;
    bool favourites_only;
    bool running_only;
    bool installed_only;
} UmiStudioRuntimeFilter;

/**
 * Initialise studio runtime filter from caller-provided values so later operations receive
 * a known state.
 */
void umi_studio_runtime_filter_init(UmiStudioRuntimeFilter *filter);

/**
 * Provide the studio runtime filter match operation used by this module and its client
 * applications.
 */
bool umi_studio_runtime_filter_match(
    const UmiStudioRuntimeFilter *filter,
    const UmiStudioRuntimeEntry *entry,
    UmiIntegrationApplicationState state
);

#ifdef __cplusplus
}
#endif

#endif
