/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/source_navigation_contribution.h
 *
 * PURPOSE:
 *   Define Studio menu placement for Framework-owned navigation commands
 *   without copying their identities, labels, keybindings or behaviour.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SOURCE_NAVIGATION_CONTRIBUTION_H
#define UMICOM_STUDIO_SOURCE_NAVIGATION_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/editor/navigation_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_SOURCE_NAVIGATION_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio source navigation contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioSourceNavigationContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioSourceNavigationContribution;

/**
 * Return the number of records represented by studio source navigation contribution
 * without changing their state.
 */
size_t umi_studio_source_navigation_contribution_count(void);
/**
 * Find studio source navigation contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioSourceNavigationContribution *
umi_studio_source_navigation_contribution_at(size_t index);
/**
 * Find studio source navigation contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioSourceNavigationContribution *
umi_studio_source_navigation_contribution_find(
    const char *framework_command_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_SOURCE_NAVIGATION_CONTRIBUTION_H */
