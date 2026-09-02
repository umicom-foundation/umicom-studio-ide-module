/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_productivity_contribution.h
 *
 * PURPOSE:
 *   Define Studio placement for Framework-owned formatting, import, snippet
 *   and linked-editing commands and views without duplicating their logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_EDITOR_PRODUCTIVITY_CONTRIBUTION_H
#define UMICOM_STUDIO_EDITOR_PRODUCTIVITY_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/editor/productivity_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_EDITOR_PRODUCTIVITY_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio editor productivity command contribution data shared with callers
 * of this public contract.
 */
typedef struct UmiStudioEditorProductivityCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioEditorProductivityCommandContribution;

/**
 * Represent the studio editor productivity view contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioEditorProductivityViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioEditorProductivityViewContribution;

/**
 * Return the number of records represented by studio editor productivity command
 * contribution without changing their state.
 */
size_t umi_studio_editor_productivity_command_contribution_count(void);
/**
 * Find studio editor productivity command contribution while leaving the underlying
 * catalogue or model owned by this module.
 */
const UmiStudioEditorProductivityCommandContribution *
umi_studio_editor_productivity_command_contribution_at(size_t index);
/**
 * Find studio editor productivity command contribution while leaving the underlying
 * catalogue or model owned by this module.
 */
const UmiStudioEditorProductivityCommandContribution *
umi_studio_editor_productivity_command_contribution_find(
    const char *framework_command_id);
/**
 * Return the number of records represented by studio editor productivity view contribution
 * without changing their state.
 */
size_t umi_studio_editor_productivity_view_contribution_count(void);
/**
 * Find studio editor productivity view contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioEditorProductivityViewContribution *
umi_studio_editor_productivity_view_contribution_at(size_t index);
/**
 * Find studio editor productivity view contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioEditorProductivityViewContribution *
umi_studio_editor_productivity_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_EDITOR_PRODUCTIVITY_CONTRIBUTION_H */
