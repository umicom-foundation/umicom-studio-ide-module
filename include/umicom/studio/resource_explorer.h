/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/resource_explorer.h
 *
 * PURPOSE:
 *   Define Studio resource-explorer composition over Framework resource navigation.
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
#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_RESOURCE_EXPLORER_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_RESOURCE_EXPLORER_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio resource explorer snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioResourceExplorerSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char view_type[128];
    char title[256];
    char summary[512];
    uint64_t revision;
    size_t item_count;
    int available;
} UmiStudioResourceExplorerSnapshot;
/**
 * Provide the studio resource explorer snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_resource_explorer_snapshot(UmiStudioServices *services, UmiStudioResourceExplorerSnapshot *out_snapshot);
#ifdef __cplusplus
}
#endif
#endif
