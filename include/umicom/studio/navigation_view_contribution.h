/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/navigation_view_contribution.h
 *
 * PURPOSE:
 *   Define Studio placement for Framework-owned source insight models without
 *   duplicating navigation, grouping or hierarchy behaviour in the product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_NAVIGATION_VIEW_CONTRIBUTION_H
#define UMICOM_STUDIO_NAVIGATION_VIEW_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_NAVIGATION_VIEW_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio navigation view contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioNavigationViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioNavigationViewContribution;

/**
 * Return the number of records represented by studio navigation view contribution without
 * changing their state.
 */
size_t umi_studio_navigation_view_contribution_count(void);
/**
 * Find studio navigation view contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioNavigationViewContribution *
umi_studio_navigation_view_contribution_at(size_t index);
/**
 * Find studio navigation view contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioNavigationViewContribution *
umi_studio_navigation_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_NAVIGATION_VIEW_CONTRIBUTION_H */
