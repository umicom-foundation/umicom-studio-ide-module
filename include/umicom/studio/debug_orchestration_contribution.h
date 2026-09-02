/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/debug_orchestration_contribution.h
 *
 * PURPOSE:
 *   Define Studio placement for Framework-owned professional debugger commands
 *   and views without duplicating configuration, query or session logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEBUG_ORCHESTRATION_CONTRIBUTION_H
#define UMICOM_STUDIO_DEBUG_ORCHESTRATION_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/debug/command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_DEBUG_ORCHESTRATION_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio debug command contribution data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDebugCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioDebugCommandContribution;

/**
 * Represent the studio debug view contribution data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDebugViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioDebugViewContribution;

/**
 * Return the number of records represented by studio debug command contribution without
 * changing their state.
 */
size_t umi_studio_debug_command_contribution_count(void);
/**
 * Find studio debug command contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDebugCommandContribution *
umi_studio_debug_command_contribution_at(size_t index);
/**
 * Find studio debug command contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDebugCommandContribution *
umi_studio_debug_command_contribution_find(const char *framework_command_id);
/**
 * Return the number of records represented by studio debug view contribution without
 * changing their state.
 */
size_t umi_studio_debug_view_contribution_count(void);
/**
 * Find studio debug view contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDebugViewContribution *
umi_studio_debug_view_contribution_at(size_t index);
/**
 * Find studio debug view contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDebugViewContribution *
umi_studio_debug_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DEBUG_ORCHESTRATION_CONTRIBUTION_H */
