/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_refactoring_contribution.h
 *
 * PURPOSE:
 *   Define Studio placement for Framework-owned quick-fix and refactoring
 *   commands and views without duplicating any provider or edit logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_EDITOR_REFACTORING_CONTRIBUTION_H
#define UMICOM_STUDIO_EDITOR_REFACTORING_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>
#include "umicom/editor/refactoring_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_EDITOR_REFACTORING_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio editor refactoring command contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioEditorRefactoringCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioEditorRefactoringCommandContribution;

/**
 * Represent the studio editor refactoring view contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioEditorRefactoringViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioEditorRefactoringViewContribution;

/**
 * Return the number of records represented by studio editor refactoring command
 * contribution without changing their state.
 */
size_t umi_studio_editor_refactoring_command_contribution_count(void);
/**
 * Find studio editor refactoring command contribution while leaving the underlying
 * catalogue or model owned by this module.
 */
const UmiStudioEditorRefactoringCommandContribution *
umi_studio_editor_refactoring_command_contribution_at(size_t index);
/**
 * Find studio editor refactoring command contribution while leaving the underlying
 * catalogue or model owned by this module.
 */
const UmiStudioEditorRefactoringCommandContribution *
umi_studio_editor_refactoring_command_contribution_find(
    const char *framework_command_id);
/**
 * Return the number of records represented by studio editor refactoring view contribution
 * without changing their state.
 */
size_t umi_studio_editor_refactoring_view_contribution_count(void);
/**
 * Find studio editor refactoring view contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioEditorRefactoringViewContribution *
umi_studio_editor_refactoring_view_contribution_at(size_t index);
/**
 * Find studio editor refactoring view contribution while leaving the underlying catalogue
 * or model owned by this module.
 */
const UmiStudioEditorRefactoringViewContribution *
umi_studio_editor_refactoring_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_EDITOR_REFACTORING_CONTRIBUTION_H */
