/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_theme.inc
 *
 * PURPOSE:
 *   Apply a compact professional GTK4 workbench skin without changing the
 *   Framework appearance model. The theme uses symbolic system colours and
 *   remains compatible with the user's light/dark preference.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static const char *RUNTIME_CSS =
    ".umicom-runtime-root { background: @theme_bg_color; color: @theme_fg_color; }"
    ".umicom-runtime-appbar { padding: 4px 8px; border-bottom: 1px solid alpha(@theme_fg_color,0.12); }"
    ".umicom-runtime-toolbar { padding: 3px 6px; border-bottom: 1px solid alpha(@theme_fg_color,0.08); }"
    ".umicom-runtime-activity { padding: 4px 3px; border-right: 1px solid alpha(@theme_fg_color,0.10); }"
    ".umicom-runtime-activity button { min-width: 38px; min-height: 38px; padding: 5px; }"
    ".umicom-runtime-tool-panel { padding: 8px; border-right: 1px solid alpha(@theme_fg_color,0.10); }"
    ".umicom-runtime-secondary { padding: 8px; border-left: 1px solid alpha(@theme_fg_color,0.10); }"
    ".umicom-runtime-bottom { border-top: 1px solid alpha(@theme_fg_color,0.10); }"
    ".umicom-runtime-status { padding: 2px 6px; border-top: 1px solid alpha(@theme_fg_color,0.12); font-size: 0.92em; }"
    ".umicom-runtime-layout-tabs { padding: 2px 6px; border-top: 1px solid alpha(@theme_fg_color,0.08); }"
    ".umicom-runtime-card { padding: 8px; border-radius: 6px; background: alpha(@theme_fg_color,0.045); }"
    ".umicom-runtime-section-title { font-weight: 700; }"
    ".umicom-runtime-muted { opacity: 0.72; }"
    ".umicom-runtime-accent { font-weight: 700; }"
    ".umicom-runtime-badge-blocked { font-weight: 700; }"
    ".umicom-runtime-command-row { padding: 6px; }"
    ".umicom-runtime-command-row:hover { background: alpha(@theme_selected_bg_color,0.18); }";

static UmiStatus runtime_apply_theme(UmiStudioGtkRuntimeChrome *runtime)
{
    GdkDisplay *display;

    if (runtime == NULL || runtime->outer_root == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->css_provider = gtk_css_provider_new();
    if (runtime->css_provider == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    gtk_css_provider_load_from_string(
        runtime->css_provider,
        RUNTIME_CSS);

    display = gtk_widget_get_display(runtime->outer_root);
    if (display != NULL) {
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(runtime->css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    gtk_widget_add_css_class(runtime->outer_root, "umicom-runtime-root");
    return UMI_STATUS_OK;
}
