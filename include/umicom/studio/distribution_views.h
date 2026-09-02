/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/distribution_views.h
 *
 * PURPOSE:
 *   Catalogue the views shown by Studio's SDK and Distribution Centre.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DISTRIBUTION_VIEWS_H
#define UMICOM_STUDIO_DISTRIBUTION_VIEWS_H

#include <stddef.h>
#include "umicom/base/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_DISTRIBUTION_MAX_VIEWS 8U

/**
 * Represent the studio distribution view data shared with callers of this public contract.
 */
typedef struct UmiStudioDistributionView {
    const char *view_id;
    const char *title;
    const char *region;
} UmiStudioDistributionView;

/**
 * Represent the studio distribution views data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDistributionViews {
    UmiStudioDistributionView views[UMI_STUDIO_DISTRIBUTION_MAX_VIEWS];
    size_t count;
} UmiStudioDistributionViews;

/**
 * Initialise studio distribution views from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_distribution_views_init(UmiStudioDistributionViews *views);
/**
 * Find studio distribution views while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioDistributionView *umi_studio_distribution_views_find(
    const UmiStudioDistributionViews *views,
    const char *view_id);

#ifdef __cplusplus
}
#endif
#endif
