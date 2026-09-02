/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/src/runtime_manager.c
 *
 * PURPOSE:
 *   Implement Studio application registration, state, presence, favourites and executable metadata.
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

#include "umicom/studio/runtime_manager.h"

#include <stdio.h>
#include <string.h>

/* Provide the copy text operation used by this module and its client applications. */
static UmiStatus copy_text(char *destination,
                           size_t capacity,
                           const char *source)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(destination, capacity, "%s", source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= capacity) {
        destination[0] = '\0';
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    return UMI_STATUS_OK;
}

/*
 * Initialise studio runtime manager from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_runtime_manager_init(UmiStudioRuntimeManager *manager)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL) {
        return;
    }

    (void)memset(manager, 0, sizeof(*manager));
    umi_integration_registry_init(&manager->registry);
    umi_integration_presence_init(&manager->presence);
}

/*
 * Add studio runtime manager only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_runtime_manager_register(
    UmiStudioRuntimeManager *manager,
    const UmiIntegrationApplication *application,
    UmiStudioRuntimeCategory category,
    const char *description)
{
    UmiStatus status;
    UmiStudioRuntimeEntry *entry;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application == NULL ||
        application->id[0] == '\0' ||
        category <= UMI_STUDIO_RUNTIME_CATEGORY_ALL ||
        category >= UMI_STUDIO_RUNTIME_CATEGORY_COUNT) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (manager->count >= UMI_STUDIO_RUNTIME_MAX_ENTRIES) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    status = umi_integration_registry_register(&manager->registry, application);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    entry = &manager->entries[manager->count];
    (void)memset(entry, 0, sizeof(*entry));
    entry->application = *application;
    entry->category = category;

    status = copy_text(entry->description,
                       sizeof(entry->description),
                       description != NULL ? description : "");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)umi_integration_registry_unregister(&manager->registry,
                                                  application->id);
        return status;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (application->executable[0] != '\0') {
        status = copy_text(entry->resolved_executable,
                           sizeof(entry->resolved_executable),
                           application->executable);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            (void)umi_integration_registry_unregister(&manager->registry,
                                                      application->id);
            return status;
        }
    }

    ++manager->count;
    return UMI_STATUS_OK;
}

/*
 * Find studio runtime manager while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStudioRuntimeEntry *umi_studio_runtime_manager_find(
    UmiStudioRuntimeManager *manager,
    const char *application_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return NULL;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < manager->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(manager->entries[index].application.id,
                   application_id) == 0) {
            return &manager->entries[index];
        }
    }

    return NULL;
}

/*
 * Provide the studio runtime manager find const operation used by this module and its
 * client applications.
 */
const UmiStudioRuntimeEntry *umi_studio_runtime_manager_find_const(
    const UmiStudioRuntimeManager *manager,
    const char *application_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return NULL;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < manager->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(manager->entries[index].application.id,
                   application_id) == 0) {
            return &manager->entries[index];
        }
    }

    return NULL;
}

/*
 * Provide the studio runtime manager set state operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_state(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    UmiIntegrationApplicationState state)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    return umi_integration_registry_set_state(&manager->registry,
                                              application_id,
                                              state);
}

/*
 * Provide the studio runtime manager state operation used by this module and its client
 * applications.
 */
UmiIntegrationApplicationState umi_studio_runtime_manager_state(
    const UmiStudioRuntimeManager *manager,
    const char *application_id)
{
    const UmiIntegrationRegistryEntry *entry;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return UMI_INTEGRATION_APP_UNKNOWN;
    }

    entry = umi_integration_registry_find_const(&manager->registry,
                                                application_id);
    return entry != NULL ? entry->state : UMI_INTEGRATION_APP_UNKNOWN;
}

/*
 * Provide the studio runtime manager set executable operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_executable(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    const char *executable_path)
{
    UmiStudioRuntimeEntry *entry;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL ||
        executable_path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    entry = umi_studio_runtime_manager_find(manager, application_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (entry == NULL) {
        return UMI_STATUS_NOT_FOUND;
    }

    status = copy_text(entry->resolved_executable,
                       sizeof(entry->resolved_executable),
                       executable_path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        entry->installed = executable_path[0] != '\0';
        entry->application.enabled = entry->installed;
        (void)umi_studio_runtime_manager_set_state(
            manager,
            application_id,
            entry->installed
                ? UMI_INTEGRATION_APP_AVAILABLE
                : UMI_INTEGRATION_APP_DISCOVERED
        );
    }

    return status;
}

/*
 * Provide the studio runtime manager set favourite operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_set_favourite(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    bool favourite)
{
    UmiStudioRuntimeEntry *entry;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    entry = umi_studio_runtime_manager_find(manager, application_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (entry == NULL) {
        return UMI_STATUS_NOT_FOUND;
    }

    entry->favourite = favourite;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio runtime manager heartbeat operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_runtime_manager_heartbeat(
    UmiStudioRuntimeManager *manager,
    const char *application_id,
    uint64_t now_ms,
    uint64_t lease_ms)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL || lease_ms == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (umi_studio_runtime_manager_find(manager, application_id) == NULL) {
        return UMI_STATUS_NOT_FOUND;
    }

    status = umi_integration_presence_heartbeat(&manager->presence,
                                                application_id,
                                                now_ms,
                                                lease_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_manager_set_state(
            manager,
            application_id,
            UMI_INTEGRATION_APP_RUNNING
        );
    }

    return status;
}

/*
 * Provide the studio runtime manager is present operation used by this module and its
 * client applications.
 */
bool umi_studio_runtime_manager_is_present(
    const UmiStudioRuntimeManager *manager,
    const char *application_id,
    uint64_t now_ms)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manager == NULL || application_id == NULL) {
        return false;
    }

    return umi_integration_presence_is_active(&manager->presence,
                                              application_id,
                                              now_ms);
}

/*
 * Return the number of records represented by studio runtime manager without changing
 * their state.
 */
size_t umi_studio_runtime_manager_count(
    const UmiStudioRuntimeManager *manager)
{
    return manager != NULL ? manager->count : 0U;
}

/*
 * Provide the studio runtime category text operation used by this module and its client
 * applications.
 */
const char *umi_studio_runtime_category_text(
    UmiStudioRuntimeCategory category)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (category) {
        case UMI_STUDIO_RUNTIME_CATEGORY_ALL: return "All";
        case UMI_STUDIO_RUNTIME_CATEGORY_DEVELOPMENT: return "Development";
        case UMI_STUDIO_RUNTIME_CATEGORY_FINANCE: return "Finance";
        case UMI_STUDIO_RUNTIME_CATEGORY_AI: return "AI";
        case UMI_STUDIO_RUNTIME_CATEGORY_CREATOR: return "Creator";
        case UMI_STUDIO_RUNTIME_CATEGORY_SYSTEM: return "System";
        case UMI_STUDIO_RUNTIME_CATEGORY_SERVICE: return "Service";
        default: return "Unknown";
    }
}
