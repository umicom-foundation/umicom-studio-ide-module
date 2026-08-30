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

static GtkWidget *runtime_build_surface_outline(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Outline");
    UmiStudioServices *services;
    UmiStudioLanguageService *language;
    UmiStudioLanguageSnapshot snapshot;
    char value[64];

    if (panel == NULL || runtime == NULL || runtime->owner == NULL) return panel;

    services = umi_studio_ui_services(runtime->owner->ui);
    language = services != NULL
        ? umi_studio_services_language(services)
        : NULL;

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
    } else {
        runtime_panel_add_stat(panel, "Language service", "Unavailable");
    }

    return runtime_scroll(panel);
}

