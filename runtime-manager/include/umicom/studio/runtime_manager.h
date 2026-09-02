/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/include/umicom/studio/runtime_manager.h
 *
 * PURPOSE:
 *   Define Studio's application catalogue and runtime-state controller over public Framework integration contracts.
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

#ifndef UMICOM_STUDIO_RUNTIME_MANAGER_H
#define UMICOM_STUDIO_RUNTIME_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "umicom/base/status.h"
#include "umicom/integration/application.h"
#include "umicom/integration/presence.h"
#include "umicom/integration/registry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_RUNTIME_DESCRIPTION_CAPACITY 384U
#define UMI_STUDIO_RUNTIME_MAX_ENTRIES 64U

/**
 * List the named studio runtime category values accepted by this public contract.
 */
typedef enum UmiStudioRuntimeCategory {
    UMI_STUDIO_RUNTIME_CATEGORY_ALL = 0,
    UMI_STUDIO_RUNTIME_CATEGORY_DEVELOPMENT,
    UMI_STUDIO_RUNTIME_CATEGORY_FINANCE,
    UMI_STUDIO_RUNTIME_CATEGORY_AI,
    UMI_STUDIO_RUNTIME_CATEGORY_CREATOR,
    UMI_STUDIO_RUNTIME_CATEGORY_SYSTEM,
    UMI_STUDIO_RUNTIME_CATEGORY_SERVICE,
    UMI_STUDIO_RUNTIME_CATEGORY_COUNT
} UmiStudioRuntimeCategory;

/**
 * Represent the studio runtime entry data shared with callers of this public contract.
 */
typedef struct UmiStudioRuntimeEntry {
    UmiIntegrationApplication application;
    UmiStudioRuntimeCategory category;
    char description[UMI_STUDIO_RUNTIME_DESCRIPTION_CAPACITY];
    char resolved_executable[UMI_INTEGRATION_PATH_CAPACITY];
    bool installed;
    bool favourite;
} UmiStudioRuntimeEntry;

/**
 * Represent the studio runtime manager data shared with callers of this public contract.
 */
typedef struct UmiStudioRuntimeManager {
    UmiIntegrationRegistry registry;
    UmiIntegrationPresence presence;
    UmiStudioRuntimeEntry entries[UMI_STUDIO_RUNTIME_MAX_ENTRIES];
    size_t count;
} UmiStudioRuntimeManager;

/**
 * Initialise studio runtime manager from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_runtime_manager_init(UmiStudioRuntimeManager *manager);

/**
 * Add studio runtime manager only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_runtime_manager_register(
    UmiStudioRuntimeManager *manager,
    const UmiIntegrationApplication *application,
    UmiStudioRuntimeCategory category,
    const char *description
);

/**
 * Find studio runtime manager while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStudioRuntimeEntry *umi_studio_runtime_manager_find(
    UmiStudioRuntimeManager *manager,
    const char *application_id
);

/**
 * Provide the studio runtime manager find const operation used by this module and its
 * client applications.
 */
const UmiStudioRuntimeEntry *umi_studio_runtime_manager_find_const(
    const UmiStudioRuntimeManager *manager,
    const char *application_id
);

/**
 * Provide the studio runtime manager set state operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_state(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    UmiIntegrationApplicationState state
);

/**
 * Provide the studio runtime manager state operation used by this module and its client
 * applications.
 */
UmiIntegrationApplicationState umi_studio_runtime_manager_state(
    const UmiStudioRuntimeManager *manager,
    const char *application_id
);

/**
 * Provide the studio runtime manager set executable operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_executable(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    const char *executable_path
);

/**
 * Provide the studio runtime manager set favourite operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_favourite(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    bool favourite
);

/**
 * Provide the studio runtime manager heartbeat operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_heartbeat(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    uint64_t now_ms,
    uint64_t lease_ms
);

/**
 * Provide the studio runtime manager is present operation used by this module and its
 * client applications.
 */
bool umi_studio_runtime_manager_is_present(
    const UmiStudioRuntimeManager *manager,
    const char *application_id,
    uint64_t now_ms
);

/**
 * Return the number of records represented by studio runtime manager without changing
 * their state.
 */
size_t umi_studio_runtime_manager_count(
    const UmiStudioRuntimeManager *manager
);

/**
 * Provide the studio runtime category text operation used by this module and its client
 * applications.
 */
const char *umi_studio_runtime_category_text(
    UmiStudioRuntimeCategory category
);

#ifdef __cplusplus
}
#endif

#endif
