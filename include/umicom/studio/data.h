/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/data.h
 *
 * PURPOSE:
 *   Declare Studio use cases over the authoritative Framework Data Server, including key/value state, transactions, integrity reporting and schema migration planning.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATA_H
#define UMICOM_STUDIO_DATA_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio data report data shared with callers of this public contract.
 */
typedef struct UmiStudioDataReport {
    UmiDataServerBackend backend;
    char backend_name[32];
    char path[UMI_PATH_CAPACITY];
    size_t records;
    uint32_t schema_version;
    int transaction_active;
    UmiStatus integrity_status;
} UmiStudioDataReport;

/**
 * Copy studio data into module-owned storage so callers keep ownership of their input
 * values.
 */
UmiStatus umi_studio_data_set(UmiStudioServices *services,
                              const char *key,
                              const char *value);
/**
 * Provide the studio data get operation used by this module and its client applications.
 */
UmiStatus umi_studio_data_get(UmiStudioServices *services,
                              const char *key,
                              char *value,
                              size_t value_capacity);
/**
 * Remove studio data while keeping the remaining records in a valid and discoverable
 * state.
 */
UmiStatus umi_studio_data_remove(UmiStudioServices *services,
                                 const char *key);
/**
 * Provide the studio data report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_data_report(UmiStudioServices *services,
                                 UmiStudioDataReport *out_report);
/**
 * Copy studio data transaction into module-owned storage so callers keep ownership of
 * their input values.
 */
UmiStatus umi_studio_data_transaction_set(UmiStudioServices *services,
                                          const char *key,
                                          const char *value);
/**
 * Provide the studio data apply core migrations operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_data_apply_core_migrations(UmiStudioServices *services,
                                                uint32_t target_version,
                                                size_t *out_applied);

#ifdef __cplusplus
}
#endif

#endif
