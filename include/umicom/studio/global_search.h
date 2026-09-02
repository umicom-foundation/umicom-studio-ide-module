/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/global_search.h
 *
 * PURPOSE:
 *   Define Studio global-search composition for files, symbols, commands and products.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_GLOBAL_SEARCH_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_GLOBAL_SEARCH_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio global search snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioGlobalSearchSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char view_type[128];
    char title[256];
    char summary[512];
    uint64_t revision;
    size_t item_count;
    int available;
} UmiStudioGlobalSearchSnapshot;
/**
 * Provide the studio global search snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_global_search_snapshot(UmiStudioServices *services, UmiStudioGlobalSearchSnapshot *out_snapshot);
#ifdef __cplusplus
}
#endif
#endif
