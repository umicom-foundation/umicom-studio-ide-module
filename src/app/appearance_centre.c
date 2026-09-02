/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/appearance_centre.c
 *
 * PURPOSE:
 *   Register the professional Umicom appearance family, keep the active theme
 *   reflected in workbench context, and persist the user's selection.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The trademark artwork remains an application asset.  Framework stores only
 * the resource locations and semantic presentation values, so another Umicom
 * application can provide its own brand while reusing the same theme engine.
 */

#include "umicom/studio/appearance_centre.h"

#include <stdio.h>
#include <string.h>

/* Provide the density name operation used by this module and its client applications. */
static const char *density_name(UmiUiDensity density)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (density) {
        case UMI_UI_DENSITY_COMPACT: return "compact";
        case UMI_UI_DENSITY_COMFORTABLE: return "comfortable";
        case UMI_UI_DENSITY_SPACIOUS: return "spacious";
        default: return "comfortable";
    }
}

/* Provide the sync context operation used by this module and its client applications. */
static UmiStatus sync_context(UmiUiWorkbench *workbench,
                              const UmiUiAppearanceProfile *profile)
{
    UmiUiContextStore *context = umi_ui_workbench_context(workbench);
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (context == NULL || profile == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = umi_ui_context_set_string(context, "studio.ui.theme",
                                       profile->profile_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_string(context, "studio.ui.density",
                                           density_name(profile->density));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_string(context, "studio.ui.font.interface",
                                           profile->interface_font);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_string(context, "studio.ui.font.editor",
                                           profile->editor_font);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_integer(
            context, "studio.ui.font.scale-per-thousand",
            (int64_t)(profile->font_scale * 1000.0));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_boolean(context, "studio.ui.reduce-motion",
                                            profile->reduce_motion);
    }
    return status;
}

/* Provide the add builtin operation used by this module and its client applications. */
static UmiStatus add_builtin(UmiUiWorkbench *workbench,
                             const char *profile_id,
                             const char *label,
                             const char *description,
                             UmiUiThemeMode mode,
                             UmiUiDensity density,
                             int active)
{
    UmiUiAppearanceProfile profile;
    UmiStatus status = umi_ui_appearance_profile_init(
        &profile, profile_id, label, mode, density);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    (void)snprintf(profile.description, sizeof(profile.description), "%s",
                   description);
    /* Dark profiles select artwork with a light foreground. Other profiles
     * use the darker primary artwork so the same identity remains readable. */
    if (mode == UMI_UI_THEME_MODE_DARK ||
        mode == UMI_UI_THEME_MODE_HIGH_CONTRAST) {
        (void)snprintf(
            profile.logo_resource, sizeof(profile.logo_resource), "%s",
            "branding/umicom-logo-on-dark.svg");
        (void)snprintf(
            profile.icon_resource, sizeof(profile.icon_resource), "%s",
            "branding/umicom-icon-on-dark.svg");
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        (void)snprintf(
            profile.logo_resource, sizeof(profile.logo_resource), "%s",
            "branding/umicom-logo.svg");
        (void)snprintf(
            profile.icon_resource, sizeof(profile.icon_resource), "%s",
            "branding/umicom-icon.svg");
    }
    profile.active = active != 0;
    profile.built_in = 1;
    profile.locked = 1;
    return umi_ui_appearance_model_upsert(
        umi_ui_workbench_appearance(workbench), &profile);
}

/* Add studio appearance only after its inputs and available capacity have been checked. */
UmiStatus umi_studio_appearance_register(UmiUiWorkbench *workbench)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = add_builtin(
        workbench, UMI_STUDIO_APPEARANCE_DARK, "Umicom Dark",
        "Deep neutral workspace with a clear blue interaction accent",
        UMI_UI_THEME_MODE_DARK, UMI_UI_DENSITY_COMPACT, 1);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = add_builtin(
            workbench, UMI_STUDIO_APPEARANCE_LIGHT, "Umicom Light",
            "Bright professional workspace using Umicom navy typography",
            UMI_UI_THEME_MODE_LIGHT, UMI_UI_DENSITY_COMFORTABLE, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = add_builtin(
            workbench, UMI_STUDIO_APPEARANCE_SYSTEM, "Follow System",
            "Use the operating-system preference with the Umicom brand palette",
            UMI_UI_THEME_MODE_SYSTEM, UMI_UI_DENSITY_COMFORTABLE, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = add_builtin(
            workbench, UMI_STUDIO_APPEARANCE_HIGH_CONTRAST,
            "Umicom High Contrast",
            "Maximum separation, strong focus indicators and reduced motion",
            UMI_UI_THEME_MODE_HIGH_CONTRAST, UMI_UI_DENSITY_SPACIOUS, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_studio_appearance_activate(workbench,
                                          UMI_STUDIO_APPEARANCE_DARK);
}

/*
 * Provide the studio appearance activate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_appearance_activate(UmiUiWorkbench *workbench,
                                         const char *profile_id)
{
    UmiUiAppearanceProfile profile;
    UmiUiAppearanceModel *model;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || profile_id == NULL || profile_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    model = umi_ui_workbench_appearance(workbench);
    status = umi_ui_appearance_model_set_active(model, profile_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_ui_appearance_model_active(model, &profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return sync_context(workbench, &profile);
}

/*
 * Provide the studio appearance restore operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_appearance_restore(UmiUiWorkbench *workbench,
                                        UmiSessionStore *session)
{
    char profile_id[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || session == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_session_store_get(session, UMI_STUDIO_APPEARANCE_SESSION_KEY,
                                   profile_id, sizeof(profile_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_NOT_FOUND) return UMI_STATUS_OK;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_studio_appearance_activate(workbench, profile_id);
    /* A deleted extension-owned custom profile must not prevent Studio from
     * starting.  The protected Umicom Dark profile is the safe fallback. */
    if (status == UMI_STATUS_NOT_FOUND) {
        return umi_studio_appearance_activate(
            workbench, UMI_STUDIO_APPEARANCE_DARK);
    }
    return status;
}

/*
 * Write studio appearance in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_appearance_save(UmiUiWorkbench *workbench,
                                     UmiSessionStore *session)
{
    UmiUiAppearanceProfile profile;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || session == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_ui_appearance_model_active(
        umi_ui_workbench_appearance(workbench), &profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    return umi_session_store_set(session, UMI_STUDIO_APPEARANCE_SESSION_KEY,
                                 profile.profile_id);
}

/*
 * Provide the studio appearance set brand resources operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_appearance_set_brand_resources(
    UmiUiWorkbench *workbench,
    const char *logo_path,
    const char *icon_path)
{
    UmiUiContextStore *context;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || logo_path == NULL || icon_path == NULL ||
        logo_path[0] == '\0' || icon_path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    context = umi_ui_workbench_context(workbench);
    status = umi_ui_context_set_string(context, "studio.brand.name", "Umicom");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_string(context, "studio.brand.logo-path",
                                           logo_path);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_context_set_string(context, "studio.brand.icon-path",
                                           icon_path);
    }
    return status;
}
