/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/history.h
 *
 * PURPOSE:
 *   Declare persistent Studio operational-history records stored through the Framework message store for build, diagnostic, task and application events.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_HISTORY_H
#define UMICOM_STUDIO_HISTORY_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio history record data shared with callers of this public contract.
 */
typedef struct UmiStudioHistoryRecord {
    uint64_t sequence;
    uint64_t correlation_id;
    char category[64];
    UmiStatus status;
    char message[512];
} UmiStudioHistoryRecord;

/**
 * Add studio history only after its inputs and available capacity have been checked.
 */
UmiStatus umi_studio_history_append(UmiStudioServices *services,
                                    const char *category,
                                    UmiStatus status,
                                    const char *message,
                                    uint64_t correlation_id,
                                    uint64_t *out_sequence);
/**
 * Return the number of records represented by studio history without changing their state.
 */
size_t umi_studio_history_count(UmiStudioServices *services);
/**
 * Read studio history into validated module state and return a status when input cannot be
 * used.
 */
UmiStatus umi_studio_history_read(UmiStudioServices *services,
                                  size_t index,
                                  UmiStudioHistoryRecord *out_record);

#ifdef __cplusplus
}
#endif

#endif
