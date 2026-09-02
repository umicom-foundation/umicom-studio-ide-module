/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/include/umicom/studio/runtime_snapshot.h
 *
 * PURPOSE:
 *   Summarise application counts and runtime states for dashboard display.
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

#ifndef UMICOM_STUDIO_RUNTIME_SNAPSHOT_H
#define UMICOM_STUDIO_RUNTIME_SNAPSHOT_H

#include <stddef.h>

#include "umicom/base/status.h"
#include "umicom/studio/runtime_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio runtime snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioRuntimeSnapshot {
    size_t total;
    size_t installed;
    size_t running;
    size_t stopped;
    size_t failed;
    size_t favourites;
    size_t by_category[UMI_STUDIO_RUNTIME_CATEGORY_COUNT];
} UmiStudioRuntimeSnapshot;

/**
 * Provide the studio runtime snapshot build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_runtime_snapshot_build(
    const UmiStudioRuntimeManager *manager,
    UmiStudioRuntimeSnapshot *snapshot
);

#ifdef __cplusplus
}
#endif

#endif
