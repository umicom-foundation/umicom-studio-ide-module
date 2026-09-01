/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_theme.inc
 *
 * PURPOSE:
 *   Apply a compact professional GTK4 workbench skin without changing the
 *   Framework appearance model. The theme consumes semantic Umicom colours
 *   and therefore follows the active light, dark or high-contrast profile.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Keep each source literal below the ISO C translation limit. The two pieces
 * are joined once when the theme is installed, so GTK still receives one
 * complete and internally consistent stylesheet. */
static const char RUNTIME_CHROME_CSS[] =
    ".umicom-runtime-root {"
    "  background: @umi_background; color: @umi_foreground;"
    "}"
    ".umicom-runtime-appbar {"
    "  min-height: 29px; padding: 0 5px;"
    "  background: @umi_surface;"
    "  border-bottom: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-appbar menubutton > button,"
    ".umicom-runtime-appbar > button {"
    "  min-height: 24px; padding: 1px 6px; border: 0;"
    "  border-radius: 3px; background: transparent;"
    "  color: @umi_foreground; box-shadow: none; font-size: 0.94em;"
    "}"
    ".umicom-runtime-appbar menubutton > button:hover,"
    ".umicom-runtime-appbar > button:hover {"
    "  background: @umi_surface_hover;"
    "}"
    ".umicom-runtime-menu-scroll {"
    "  background: transparent; border: 0;"
    "}"
    ".umicom-runtime-menu-scroll undershoot { background: transparent; }"
    ".umicom-runtime-appbar popover contents {"
    "  padding: 4px; border: 1px solid @umi_border;"
    "  background: @umi_surface_raised;"
    "}"
    ".umicom-runtime-appbar popover button {"
    "  min-height: 27px; padding: 3px 8px;"
    "}"
    "button.umicom-runtime-appbar-action {"
    "  min-width: 26px; min-height: 26px; padding: 3px;"
    "}"
    ".umicom-runtime-brand {"
    "  min-height: 25px; padding: 1px 8px 1px 2px;"
    "}"
    ".umicom-runtime-brand-title {"
    "  color: @umi_foreground; font-weight: 700; letter-spacing: 0.01em;"
    "}"
    ".umicom-runtime-toolbar {"
    "  min-height: 31px; padding: 2px 6px;"
    "  background: @umi_surface_raised;"
    "  border-bottom: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-toolbar button {"
    "  min-width: 27px; min-height: 26px; padding: 2px 4px;"
    "  border-radius: 3px;"
    "}"
    ".umicom-runtime-activity {"
    "  min-width: 39px; padding: 3px 3px;"
    "  background: @umi_background;"
    "  border-right: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-activity button {"
    "  min-width: 31px; min-height: 32px; padding: 4px;"
    "  border-radius: 3px;"
    "}"
    ".umicom-runtime-activity button:checked,"
    ".umicom-runtime-activity button.suggested-action {"
    "  background: @umi_accent_surface; color: @umi_accent;"
    "  border-left: 2px solid @umi_accent;"
    "}"
    ".umicom-runtime-tool-panel {"
    "  padding: 6px; background: @umi_surface;"
    "}"
    ".umicom-runtime-primary {"
    "  border-right: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-secondary {"
    "  border-left: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-bottom {"
    "  border-top: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-centre {"
    "  background: @umi_editor;"
    "}"
    ".umicom-runtime-split > separator {"
    "  min-width: 3px; min-height: 3px;"
    "  background: @umi_border;"
    "}"
    ".umicom-runtime-split > separator:hover {"
    "  background: @umi_accent;"
    "}"
    ".umicom-runtime-status {"
    "  min-height: 20px; padding: 1px 7px;"
    "  background: @umi_background; color: @umi_muted;"
    "  border-top: 1px solid @umi_border;"
    "  font-size: 0.90em;"
    "}"
    ".umicom-runtime-layout-tabs {"
    "  min-height: 27px; padding: 1px 6px;"
    "  background: @umi_surface_raised;"
    "  border-top: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-layout-tabs button {"
    "  min-height: 24px; padding: 1px 7px; border-radius: 3px;"
    "}"
    ".umicom-runtime-layout-lock.umicom-layout-locked {"
    "  color: @umi_muted; background: transparent;"
    "}"
    ".umicom-runtime-layout-lock.umicom-layout-editing {"
    "  color: @umi_foreground; background: @umi_accent_surface;"
    "  border-color: @umi_accent;"
    "}"
    ".umicom-runtime-panel-actions {"
    "  min-height: 29px; padding: 1px 0 4px 0;"
    "  border-bottom: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-panel-actions button {"
    "  min-width: 28px; min-height: 26px; padding: 2px;"
    "}"
    ".umicom-runtime-card {"
    "  padding: 8px; border-radius: 4px;"
    "  background: @umi_surface_raised;"
    "  border: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-section-title {"
    "  font-weight: 700; font-size: 0.96em;"
    "}"
    ".umicom-runtime-muted { color: @umi_muted; }"
    ".umicom-runtime-accent { font-weight: 700; }"
    ".umicom-runtime-badge-blocked { font-weight: 700; }"
    ".umicom-runtime-command-row { padding: 6px; border-radius: 3px; }"
    ".umicom-runtime-command-row:hover {"
    "  background: @umi_surface_hover;"
    "}"
    ".umicom-runtime-editor-command-bar {"
    "  min-height: 31px; padding: 2px 6px;"
    "  background: @umi_surface_raised;"
    "  border-bottom: 1px solid @umi_border;"
    "}"
    ".umicom-runtime-editor-command-bar entry { min-height: 26px; }"
    ".umicom-runtime-editor-command-bar button {"
    "  min-height: 26px; padding: 2px 7px;"
    "}"
    ".umicom-runtime-detail-notebook {"
    "  background: @umi_surface;"
    "}"
    ".umicom-runtime-diff-pane { font-family: monospace; }";

static const char RUNTIME_WELCOME_CSS[] =
    ".umicom-welcome-scroll {"
    "  background: linear-gradient(135deg,"
    "    alpha(@umi_accent,0.08),"
    "    alpha(@umi_background,0.0) 48%);"
    "}"
    ".umicom-welcome-page { padding: 24px 28px; }"
    ".umicom-welcome-title {"
    "  color: @umi_foreground; font-size: 1.70em; font-weight: 700;"
    "}"
    ".umicom-welcome-summary {"
    "  color: @umi_muted; font-size: 1.00em;"
    "}"
    ".umicom-welcome-card {"
    "  min-width: 190px; padding: 16px; border-radius: 5px;"
    "  background: @umi_surface;"
    "  border: 1px solid @umi_border;"
    "}"
    ".umicom-welcome-card:hover {"
    "  background: @umi_surface_raised;"
    "  border-color: @umi_border_strong;"
    "}"
    ".umicom-welcome-card-title {"
    "  color: @umi_foreground; font-size: 1.10em; font-weight: 700;"
    "}"
    ".umicom-welcome-card button {"
    "  min-height: 30px; padding: 4px 8px;"
    "  border-radius: 3px;"
    "}";

_Static_assert(
    sizeof(RUNTIME_CHROME_CSS) <= 4096U,
    "Studio chrome CSS exceeds ISO C's portable string limit");
_Static_assert(
    sizeof(RUNTIME_WELCOME_CSS) <= 4096U,
    "Studio welcome CSS exceeds ISO C's portable string limit");

static UmiStatus runtime_apply_theme(UmiStudioGtkRuntimeChrome *runtime)
{
    GdkDisplay *display;
    char *complete_css;

    if (runtime == NULL || runtime->outer_root == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->css_provider = gtk_css_provider_new();
    if (runtime->css_provider == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    /* Join the bounded source fragments at runtime to avoid an overlength C
     * literal without weakening or dropping any visual styling rules. */
    complete_css = g_strconcat(
        RUNTIME_CHROME_CSS,
        RUNTIME_WELCOME_CSS,
        NULL);
    if (complete_css == NULL) {
        g_clear_object(&runtime->css_provider);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_css_provider_load_from_string(
        runtime->css_provider,
        complete_css);
    g_free(complete_css);

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
