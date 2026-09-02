/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/workspace_persistence.c
 *
 * PURPOSE:
 *   Implement the workspace persistence behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_persistence.h"

/* Keep Studio thin by asking Framework to encode its active customisation. */
UmiStatus umi_studio_workspace_persistence_export_active(
    UmiStudioProfessionalWorkspace *workspace,
    uint64_t saved_at_ns,
    char *out_text,
    size_t capacity)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL || out_text == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_ui_workspace_customisation_export_active(
        model, saved_at_ns, out_text, capacity);
}

/* Preserve the simple Studio API by delegating to the detailed import path. */
UmiStatus umi_studio_workspace_persistence_import(
    UmiStudioProfessionalWorkspace *workspace,
    const char *text,
    bool activate)
{
    return umi_studio_workspace_persistence_import_with_report(
        workspace, text, activate, false, NULL);
}

/* Translate Studio choices into Framework policy without duplicating parsing. */
UmiStatus umi_studio_workspace_persistence_import_with_report(
    UmiStudioProfessionalWorkspace *workspace,
    const char *text,
    bool activate,
    bool replace_existing,
    UmiUiWorkspaceImportReport *out_report)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    UmiUiWorkspaceImportOptions options =
        umi_ui_workspace_import_options_default();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL || text == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Studio can import user-created layouts in addition to its canonical
     * views; explicit replacement is required before overwriting one. */
    options.allow_new_layout = true;
    options.activate_imported_layout = activate;
    options.conflict_policy = replace_existing
        ? UMI_UI_WORKSPACE_IMPORT_REPLACE_CONFLICT
        : UMI_UI_WORKSPACE_IMPORT_REJECT_CONFLICT;
    return umi_ui_workspace_customisation_import(
        model, text, &options, out_report);
}
