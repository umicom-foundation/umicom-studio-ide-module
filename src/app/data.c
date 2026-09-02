/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/data.c
 *
 * PURPOSE:
 *   Implement Studio persistence operations through Framework Data Server, transaction, integrity and migration contracts without exposing database handles to Studio panes or controllers.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/data.h"

#include <stdio.h>
#include <string.h>

/*
 * Copy studio data into module-owned storage so callers keep ownership of their input
 * values.
 */
UmiStatus umi_studio_data_set(UmiStudioServices *services,
                              const char *key,
                              const char *value)
{
    UmiStore *store = umi_studio_services_store(services);
    return store != NULL
        ? umi_store_set(store, key, value)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/* Provide the studio data get operation used by this module and its client applications. */
UmiStatus umi_studio_data_get(UmiStudioServices *services,
                              const char *key,
                              char *value,
                              size_t value_capacity)
{
    UmiStore *store = umi_studio_services_store(services);
    return store != NULL
        ? umi_store_get(store, key, value, value_capacity)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Remove studio data while keeping the remaining records in a valid and discoverable
 * state.
 */
UmiStatus umi_studio_data_remove(UmiStudioServices *services,
                                 const char *key)
{
    UmiStore *store = umi_studio_services_store(services);
    return store != NULL
        ? umi_store_remove(store, key)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Provide the studio data report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_data_report(UmiStudioServices *services,
                                 UmiStudioDataReport *out_report)
{
    UmiDataIntegrityReport report;
    UmiDataServer *server;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_report == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    server = umi_studio_services_data_server(services);
    status = umi_data_integrity_check(server, &report);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    (void)memset(out_report, 0, sizeof(*out_report));
    out_report->backend = report.backend;
    (void)snprintf(out_report->backend_name,
                   sizeof(out_report->backend_name),
                   "%s",
                   report.backend_name != NULL ? report.backend_name : "unknown");
    (void)snprintf(out_report->path,
                   sizeof(out_report->path),
                   "%s",
                   report.path != NULL ? report.path : "");
    out_report->records = report.records;
    out_report->schema_version = report.schema_version;
    out_report->transaction_active = report.transaction_active;
    out_report->integrity_status = report.integrity_status;
    return UMI_STATUS_OK;
}

/*
 * Copy studio data transaction into module-owned storage so callers keep ownership of
 * their input values.
 */
UmiStatus umi_studio_data_transaction_set(UmiStudioServices *services,
                                          const char *key,
                                          const char *value)
{
    UmiDataTransaction transaction;
    UmiDataServer *server;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || key == NULL || value == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    server = umi_studio_services_data_server(services);
    status = umi_data_transaction_begin(server, &transaction);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_data_server_set(server, key, value);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_data_transaction_commit(&transaction);
    }
    umi_data_transaction_dispose(&transaction);
    return status;
}

/*
 * Provide the studio data apply core migrations operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_data_apply_core_migrations(UmiStudioServices *services,
                                                uint32_t target_version,
                                                size_t *out_applied)
{
    UmiMigrationRegistry *registry = NULL;
    UmiDataServer *server;
    UmiStatus status;
    static const UmiMigration migrations[] = {
        { 1U, "Studio core state", "CREATE TABLE IF NOT EXISTS studio_state (key TEXT PRIMARY KEY, value TEXT NOT NULL);", "DROP TABLE studio_state;" },
        { 2U, "Studio history", "CREATE TABLE IF NOT EXISTS studio_history (sequence INTEGER PRIMARY KEY, category TEXT NOT NULL, status INTEGER NOT NULL, message TEXT NOT NULL);", "DROP TABLE studio_history;" }
    };
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || target_version > 2U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    server = umi_studio_services_data_server(services);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_data_server_backend(server) != UMI_DATA_BACKEND_SQLITE) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (out_applied != NULL) *out_applied = 0U;
        return UMI_STATUS_NOT_IMPLEMENTED;
    }
    status = umi_migration_registry_create(4U, &registry);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(migrations) / sizeof(migrations[0]); ++index) {
        status = umi_migration_registry_add(registry, &migrations[index]);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) break;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_migration_apply(registry, server, target_version, out_applied);
    }
    umi_migration_registry_destroy(registry);
    return status;
}
