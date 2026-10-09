/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/outline.inc
 *
 * PURPOSE:
 *   Build the Framework-backed outline presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/language_connection.h"

static GtkWidget *runtime_build_surface_outline(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Outline");
    UmiStudioServices *services;
    UmiStudioLanguageService *language;
    UmiStudioLanguageSnapshot snapshot;
    char value[64];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL || runtime->owner == NULL) return panel;

    services = umi_studio_ui_services(runtime->owner->ui);
    language = services != NULL
        ? umi_studio_services_language(services)
        : NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (language != NULL &&
        umi_studio_language_service_snapshot(
            language,
            &snapshot) == UMI_STATUS_OK) {
        (void)snprintf(value, sizeof(value), "%zu", snapshot.language_definitions);
        runtime_panel_add_stat(panel, "Languages", value);
        (void)snprintf(value, sizeof(value), "%zu", snapshot.diagnostics);
        runtime_panel_add_stat(panel, "Diagnostics", value);
        (void)snprintf(value, sizeof(value), "%zu", snapshot.pending_requests);
        runtime_panel_add_stat(panel, "Pending requests", value);
        runtime_panel_add_stat(panel, "LSP session", snapshot.session_state);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_panel_add_stat(panel, "Language service", "Unavailable");
    }

    /* Reuse the editor's weak owner binding. The outline action captures the
     * active draft only when clicked and still requires an explicit request
     * before running a server or sending source text. */
    GtkWidget *symbols=gtk_button_new_with_label("Document symbols…");
    (void)umi_gtk4_automation_tag_widget(symbols,"studio.outline.symbols");
    runtime_bind_editor_action(runtime,symbols,RuntimeDocumentSymbols);
    gtk_box_append(GTK_BOX(panel),symbols);

    /* Framework owns native launching, protocol checks and worker lifetime.
     * Studio supplies only the current folder as an editable draft. Opening
     * the Outline view must never start a workspace-selected executable. */
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder = NULL;
    if (services != NULL && umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK &&
        workspace.graph.open) folder = workspace.graph.root;
    GtkWidget *connection = NULL;
/* Use the project tool folder as an editable connection-check draft. Framework copies it into the panel; Studio does not keep a borrowed settings pointer or start a server on opening Outline. The previous implementation is retained for engineering review. */
#if 0
    if (UmiGtk4LanguageConnectionPanelCreate(folder, &connection) == UMI_STATUS_OK)
        gtk_box_append(GTK_BOX(panel), connection);
#endif
    const UmiBuildProfile *buildProfile = services != NULL
        ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
    if (UmiGtk4LanguageConnectionPanelCreateWithToolDirectory(folder,
        buildProfile != NULL ? buildProfile->tool_directory : NULL, &connection) == UMI_STATUS_OK)
        gtk_box_append(GTK_BOX(panel), connection);

    return runtime_scroll(panel);
}

