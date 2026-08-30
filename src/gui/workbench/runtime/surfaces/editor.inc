/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/editor.inc
 *
 * PURPOSE:
 *   Build the Framework-backed editor presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_editor(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Editor");
    UmiDocumentWorkingCopySnapshot active;

    if (panel == NULL || runtime == NULL || runtime->owner == NULL) return panel;

    if (umi_document_coordinator_active_snapshot(
            umi_studio_ui_documents(runtime->owner->ui),
            &active) == UMI_STATUS_OK) {
        runtime_panel_add_stat(panel, "Document", active.display_name);
        runtime_panel_add_stat(panel, "Language", active.language_id);
        runtime_panel_add_stat(
            panel,
            "State",
            active.dirty ? "Modified" : "Saved");
    } else {
        runtime_panel_add_stat(panel, "Document", "No active document");
    }

    return runtime_scroll(panel);
}

