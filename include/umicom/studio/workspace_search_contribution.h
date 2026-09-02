/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace_search_contribution.h
 *
 * PURPOSE:
 *   Define Studio menu and workbench placement for Framework-owned workspace
 *   search, replacement preview and atomic multi-document transactions.
 *
 * BOUNDARY:
 *   This application contribution contains no search, regex, exclusion,
 *   replacement or transaction logic. Those capabilities remain authoritative
 *   in Umicom Framework and are reusable by every future application.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_SEARCH_CONTRIBUTION_H
#define UMICOM_STUDIO_WORKSPACE_SEARCH_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/editor/workspace_search_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_WORKSPACE_SEARCH_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio workspace search command contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioWorkspaceSearchCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioWorkspaceSearchCommandContribution;

/**
 * Represent the studio workspace search view contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioWorkspaceSearchViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioWorkspaceSearchViewContribution;

/**
 * Return the number of records represented by studio workspace search command contribution
 * without changing their state.
 */
size_t umi_studio_workspace_search_command_contribution_count(void);
/**
 * Find studio workspace search command contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioWorkspaceSearchCommandContribution *
umi_studio_workspace_search_command_contribution_at(size_t position);
/**
 * Find studio workspace search command contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioWorkspaceSearchCommandContribution *
umi_studio_workspace_search_command_contribution_find(
    const char *framework_command_id);
/**
 * Return the number of records represented by studio workspace search view contribution
 * without changing their state.
 */
size_t umi_studio_workspace_search_view_contribution_count(void);
/**
 * Find studio workspace search view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioWorkspaceSearchViewContribution *
umi_studio_workspace_search_view_contribution_at(size_t position);
/**
 * Find studio workspace search view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioWorkspaceSearchViewContribution *
umi_studio_workspace_search_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_WORKSPACE_SEARCH_CONTRIBUTION_H */
