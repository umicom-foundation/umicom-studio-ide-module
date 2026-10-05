/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_COMMANDS_H
#define UMICOM_STUDIO_WORKSPACE_COMMANDS_H
#include "umicom/application/runtime/context_review.h"
#include <stddef.h>
#include "umicom/application/runtime/session_snapshot.h"

#include "umicom/studio/workspace_themes.h"
/**
 * List the named studio workspace command values accepted by this public contract.
 */
typedef enum UmiStudioWorkspaceCommand {
    UMI_STUDIO_WORKSPACE_COMMAND_SEED = 1,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEVELOP,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_FOCUS,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEBUG,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_OPERATIONS,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_TRADING,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_COMPARE,
    UMI_STUDIO_WORKSPACE_COMMAND_UNLOCK,
    UMI_STUDIO_WORKSPACE_COMMAND_LOCK,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_LIGHT,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_DARK,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_HIGH_CONTRAST
} UmiStudioWorkspaceCommand;
/**
 * Provide the studio workspace seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_seed(UmiStudioProfessionalWorkspace *workspace);
/**
 * Perform studio workspace through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_workspace_execute(UmiStudioProfessionalWorkspace *workspace,UmiStudioWorkspaceCommand command);
/* Forward a complete panel edit to Framework without Studio-owned layout logic. */
UmiStatus umi_studio_workspace_apply_panel_settings(
    UmiStudioProfessionalWorkspace *workspace,
    const UmiUiWorkspacePanelSettings *settings);
/* Apply several Studio panel changes without publishing a partial edit. */
UmiStatus umi_studio_workspace_apply_panel_batch(
    UmiStudioProfessionalWorkspace *workspace,
    const UmiUiWorkspacePanelSettings *settings,
    size_t setting_count);
#ifdef __cplusplus
extern "C" {
#endif
/* Capture, preview and apply this product's passive application session.
 * The session must borrow the canonical Umicom Studio IDE experience. Preview resolves
 * every saved panel against that catalogue without changing a live session.
 * Apply requires the revision observed before review and advances it once.
 * These APIs perform no storage, GUI activation, build or trading operation.
 * A host prepares its presentation separately before publishing its session. */
UmiStatus umi_studio_workspace_session_capture(const UmiApplicationSession *session,
    void *bytes, size_t capacity, size_t *out_size);
UmiStatus umi_studio_workspace_session_preview(const void *bytes, size_t byte_count,
    UmiApplicationSession *out_candidate);
UmiStatus umi_studio_workspace_session_apply(UmiApplicationSession *session,
    uint64_t expected_revision, const void *bytes, size_t byte_count);

/* Review several linked groups without changing this product's runtime.
 * Framework owns validation, copied review rows and atomic UI publication.
 * The runtime must borrow this product's canonical experience. Use the shared
 * review summary/row functions to display the proposal, then explicitly apply.
 * Destroy the review after use; context values never authorise commands. */
UmiStatus umi_studio_workspace_context_review(
    UmiApplicationWorkspaceRuntime *runtime,
    const UmiApplicationContextChange *changes, size_t count,
    UmiApplicationContextReview **out_review);
UmiStatus umi_studio_workspace_context_apply(
    UmiApplicationWorkspaceRuntime *runtime, UmiApplicationContextReview *review);
/* Remove a linked group through the same Framework transaction. */
UmiStatus umi_studio_workspace_clear_context(
    UmiApplicationWorkspaceRuntime *runtime, const char *group_id);

#ifdef __cplusplus
}
#endif
#endif
