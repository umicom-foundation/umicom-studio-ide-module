/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_theme.inc
 *
 * PURPOSE:
 *   Apply a compact professional GTK4 workbench skin without changing the
 *   Framework appearance model. The theme uses symbolic system colours and
 *   remains compatible with the user's light/dark preference.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static const char *RUNTIME_CSS =
    ".umicom-runtime-root {"
    "  background: @theme_bg_color; color: @theme_fg_color;"
    "}"
    ".umicom-runtime-appbar {"
    "  min-height: 32px; padding: 0 8px;"
    "  background: shade(@theme_bg_color,0.92);"
    "  border-bottom: 1px solid alpha(@theme_fg_color,0.16);"
    "}"
    ".umicom-runtime-appbar menubutton > button,"
    ".umicom-runtime-appbar > button {"
    "  min-height: 28px; padding: 2px 8px; border-radius: 3px;"
    "}"
    ".umicom-runtime-brand {"
    "  min-height: 28px; padding: 2px 7px 2px 2px;"
    "}"
    ".umicom-runtime-brand-title {"
    "  font-weight: 700; letter-spacing: 0.01em;"
    "}"
    ".umicom-runtime-toolbar {"
    "  min-height: 35px; padding: 2px 7px;"
    "  background: shade(@theme_bg_color,0.96);"
    "  border-bottom: 1px solid alpha(@theme_fg_color,0.12);"
    "}"
    ".umicom-runtime-toolbar button {"
    "  min-width: 30px; min-height: 28px; padding: 2px 5px;"
    "  border-radius: 3px;"
    "}"
    ".umicom-runtime-activity {"
    "  min-width: 43px; padding: 4px 3px;"
    "  background: shade(@theme_bg_color,0.90);"
    "  border-right: 1px solid alpha(@theme_fg_color,0.14);"
    "}"
    ".umicom-runtime-activity button {"
    "  min-width: 35px; min-height: 36px; padding: 4px;"
    "  border-radius: 3px;"
    "}"
    ".umicom-runtime-activity button:checked,"
    ".umicom-runtime-activity button.suggested-action {"
    "  background: alpha(@theme_selected_bg_color,0.32);"
    "  border-left: 2px solid @theme_selected_bg_color;"
    "}"
    ".umicom-runtime-tool-panel {"
    "  padding: 8px; background: shade(@theme_bg_color,0.97);"
    "}"
    ".umicom-runtime-primary {"
    "  border-right: 1px solid alpha(@theme_fg_color,0.12);"
    "}"
    ".umicom-runtime-secondary {"
    "  border-left: 1px solid alpha(@theme_fg_color,0.12);"
    "}"
    ".umicom-runtime-bottom {"
    "  border-top: 1px solid alpha(@theme_fg_color,0.14);"
    "}"
    ".umicom-runtime-centre {"
    "  background: shade(@theme_bg_color,0.94);"
    "}"
    ".umicom-runtime-split > separator {"
    "  min-width: 3px; min-height: 3px;"
    "  background: alpha(@theme_fg_color,0.06);"
    "}"
    ".umicom-runtime-split > separator:hover {"
    "  background: alpha(@theme_selected_bg_color,0.62);"
    "}"
    ".umicom-runtime-status {"
    "  min-height: 20px; padding: 1px 7px;"
    "  background: shade(@theme_bg_color,0.90);"
    "  border-top: 1px solid alpha(@theme_fg_color,0.14);"
    "  font-size: 0.90em;"
    "}"
    ".umicom-runtime-layout-tabs {"
    "  min-height: 29px; padding: 2px 7px;"
    "  background: shade(@theme_bg_color,0.94);"
    "  border-top: 1px solid alpha(@theme_fg_color,0.12);"
    "}"
    ".umicom-runtime-layout-tabs button {"
    "  min-height: 24px; padding: 1px 7px; border-radius: 3px;"
    "}"
    ".umicom-runtime-panel-actions {"
    "  min-height: 29px; padding: 1px 0 4px 0;"
    "  border-bottom: 1px solid alpha(@theme_fg_color,0.09);"
    "}"
    ".umicom-runtime-panel-actions button {"
    "  min-width: 28px; min-height: 26px; padding: 2px;"
    "}"
    ".umicom-runtime-card {"
    "  padding: 8px; border-radius: 4px;"
    "  background: alpha(@theme_fg_color,0.04);"
    "  border: 1px solid alpha(@theme_fg_color,0.07);"
    "}"
    ".umicom-runtime-section-title {"
    "  font-weight: 700; font-size: 0.96em;"
    "}"
    ".umicom-runtime-muted { opacity: 0.70; }"
    ".umicom-runtime-accent { font-weight: 700; }"
    ".umicom-runtime-badge-blocked { font-weight: 700; }"
    ".umicom-runtime-command-row { padding: 6px; border-radius: 3px; }"
    ".umicom-runtime-command-row:hover {"
    "  background: alpha(@theme_selected_bg_color,0.18);"
    "}"
    ".umicom-runtime-editor-command-bar {"
    "  min-height: 34px; padding: 3px 7px;"
    "  background: shade(@theme_bg_color,0.96);"
    "  border-bottom: 1px solid alpha(@theme_fg_color,0.12);"
    "}"
    ".umicom-runtime-editor-command-bar entry { min-height: 26px; }"
    ".umicom-runtime-editor-command-bar button {"
    "  min-height: 26px; padding: 2px 7px;"
    "}"
    ".umicom-runtime-detail-notebook {"
    "  background: alpha(@theme_fg_color,0.025);"
    "}"
    ".umicom-runtime-diff-pane { font-family: monospace; }"
    ".umicom-welcome-scroll {"
    "  background: linear-gradient(135deg,"
    "    alpha(@theme_selected_bg_color,0.08),"
    "    alpha(@theme_bg_color,0.0) 48%);"
    "}"
    ".umicom-welcome-page { padding: 42px 48px; }"
    ".umicom-welcome-title {"
    "  font-size: 2.05em; font-weight: 700;"
    "}"
    ".umicom-welcome-summary {"
    "  font-size: 1.05em; opacity: 0.72;"
    "}"
    ".umicom-welcome-card {"
    "  min-width: 230px; padding: 20px; border-radius: 6px;"
    "  background: alpha(@theme_fg_color,0.045);"
    "  border: 1px solid alpha(@theme_fg_color,0.10);"
    "}"
    ".umicom-welcome-card:hover {"
    "  background: alpha(@theme_fg_color,0.065);"
    "  border-color: alpha(@theme_selected_bg_color,0.42);"
    "}"
    ".umicom-welcome-card-title {"
    "  font-size: 1.18em; font-weight: 700;"
    "}"
    ".umicom-welcome-card button {"
    "  min-height: 32px; padding: 5px 9px;"
    "  border-radius: 3px;"
    "}";

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
