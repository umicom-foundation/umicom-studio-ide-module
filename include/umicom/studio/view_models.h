/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/view_models.h
 *
 * PURPOSE:
 *   Define the Studio-owned toolkit-neutral view-model set that projects
 *   Framework workspace, document, diagnostic, search and task state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_VIEW_MODELS_H
#define UMICOM_STUDIO_VIEW_MODELS_H

#include "umicom/umicom.h"
#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio view models data shared with callers of this public contract.
 */
typedef struct UmiStudioViewModels UmiStudioViewModels;

/**
 * Represent the studio view model snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioViewModelSnapshot {
    size_t view_count;
    size_t workspace_files;
    size_t open_documents;
    size_t dirty_documents;
    size_t retained_diagnostics;
    size_t queued_tasks;
    size_t running_tasks;
    uint64_t revision;
} UmiStudioViewModelSnapshot;

/**
 * Initialise studio view models from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_view_models_create(UmiStudioServices *services,
                                        UmiStudioViewModels **out_models);
/**
 * Release or reset state held by studio view models so the same storage can be reused
 * safely.
 */
void umi_studio_view_models_destroy(UmiStudioViewModels *models);
/**
 * Provide the studio view models refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_view_models_refresh(UmiStudioViewModels *models);
/**
 * Provide the studio view models snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_view_models_snapshot(const UmiStudioViewModels *models,
                                          UmiStudioViewModelSnapshot *out_snapshot);
/**
 * Find studio view models while leaving the underlying catalogue or model owned by this
 * module.
 */
UmiUiViewModel *umi_studio_view_models_find(UmiStudioViewModels *models,
                                           const char *view_id);
/**
 * Return the number of records represented by studio view models without changing their
 * state.
 */
size_t umi_studio_view_models_count(const UmiStudioViewModels *models);

#ifdef __cplusplus
}
#endif

#endif
