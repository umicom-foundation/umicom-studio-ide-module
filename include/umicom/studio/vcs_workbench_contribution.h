/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/vcs_workbench_contribution.h
 *
 * PURPOSE:
 *   Place Framework-owned compare, merge, partial-staging and review commands
 *   and views in the Studio IDE without copying operational VCS logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_VCS_WORKBENCH_CONTRIBUTION_H
#define UMICOM_STUDIO_VCS_WORKBENCH_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/vcs/workbench/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_VCS_WORKBENCH_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio vcs workbench command contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioVcsWorkbenchCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioVcsWorkbenchCommandContribution;

/**
 * Represent the studio vcs workbench view contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioVcsWorkbenchViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioVcsWorkbenchViewContribution;

/**
 * Return the number of records represented by studio vcs workbench command without
 * changing their state.
 */
size_t umi_studio_vcs_workbench_command_count(void);
/**
 * Find studio vcs workbench command while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioVcsWorkbenchCommandContribution *
umi_studio_vcs_workbench_command_at(size_t index);
/**
 * Find studio vcs workbench command while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioVcsWorkbenchCommandContribution *
umi_studio_vcs_workbench_command_find(const char *framework_command_id);
/**
 * Return the number of records represented by studio vcs workbench view without changing
 * their state.
 */
size_t umi_studio_vcs_workbench_view_count(void);
/**
 * Find studio vcs workbench view while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioVcsWorkbenchViewContribution *
umi_studio_vcs_workbench_view_at(size_t index);
/**
 * Find studio vcs workbench view while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioVcsWorkbenchViewContribution *
umi_studio_vcs_workbench_view_find(const char *view_id);
/**
 * Initialise studio vcs workbench from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_vcs_workbench_create(
    UmiVcsWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_VCS_WORKBENCH_CONTRIBUTION_H */
