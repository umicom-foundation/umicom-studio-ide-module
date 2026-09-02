/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_command_contribution.h
 *
 * PURPOSE:
 *   Define Studio menu placement for Framework-owned editing commands without
 *   copying their labels, keybindings or execution logic into the product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_EDITOR_COMMAND_CONTRIBUTION_H
#define UMICOM_STUDIO_EDITOR_COMMAND_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/editor/edit_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_EDITOR_COMMAND_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio editor command contribution data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioEditorCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioEditorCommandContribution;

/**
 * Return the number of records represented by studio editor command contribution without
 * changing their state.
 */
size_t umi_studio_editor_command_contribution_count(void);
/**
 * Find studio editor command contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioEditorCommandContribution *
umi_studio_editor_command_contribution_at(size_t index);
/**
 * Find studio editor command contribution while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioEditorCommandContribution *
umi_studio_editor_command_contribution_find(const char *framework_command_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_EDITOR_COMMAND_CONTRIBUTION_H */
