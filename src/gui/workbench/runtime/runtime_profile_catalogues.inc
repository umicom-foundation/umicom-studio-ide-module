/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_profile_catalogues.inc
 *
 * PURPOSE:
 *   Compile the private GTK presentation profiles into deterministic catalogues.
 *   These catalogues contain only frontend metadata; commands, IDE behavior,
 *   semantic layouts and surface identity remain authoritative in Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

typedef enum UmiStudioGtkRuntimeRegion {
    UMI_STUDIO_GTK_REGION_PRIMARY = 1,
    UMI_STUDIO_GTK_REGION_SECONDARY = 2,
    UMI_STUDIO_GTK_REGION_BOTTOM = 3,
    UMI_STUDIO_GTK_REGION_EDITOR = 4
} UmiStudioGtkRuntimeRegion;

typedef struct UmiStudioGtkSurfaceProfile {
    UmiStudioRuntimeSurfaceKind kind;
    const char *stack_name;
    const char *title;
    const char *icon_name;
    const char *command_id;
    UmiStudioGtkRuntimeRegion region;
    int default_visible;
} UmiStudioGtkSurfaceProfile;

typedef struct UmiStudioGtkCommandProfile {
    const char *profile_id;
    const char *command_id;
    const char *label;
    const char *icon_name;
    const char *category;
    const char *tooltip;
} UmiStudioGtkCommandProfile;

typedef struct UmiStudioGtkLayoutProfile {
    const char *profile_id;
    const char *label;
    const char *command_id;
    const char *description;
} UmiStudioGtkLayoutProfile;

typedef struct UmiStudioGtkStatusProfile {
    UmiStudioRuntimeStatusKind kind;
    const char *profile_id;
    const char *label;
} UmiStudioGtkStatusProfile;

typedef struct UmiStudioGtkKeyProfile {
    guint key_lower;
    guint key_upper;
    GdkModifierType modifiers;
    const char *command_id;
    const char *label;
} UmiStudioGtkKeyProfile;

#define UMI_STUDIO_GTK_SURFACE_PROFILE(kind_, name_, title_, icon_, command_, region_, visible_) \
    {kind_, name_, title_, icon_, command_, region_, visible_},
static const UmiStudioGtkSurfaceProfile RUNTIME_SURFACE_PROFILES[] = {
#include "profiles/surfaces/explorer.inc"
#include "profiles/surfaces/editor.inc"
#include "profiles/surfaces/outline.inc"
#include "profiles/surfaces/search.inc"
#include "profiles/surfaces/problems.inc"
#include "profiles/surfaces/output.inc"
#include "profiles/surfaces/terminal.inc"
#include "profiles/surfaces/source_control.inc"
#include "profiles/surfaces/test_explorer.inc"
#include "profiles/surfaces/debug.inc"
#include "profiles/surfaces/build.inc"
#include "profiles/surfaces/ai_chat.inc"
#include "profiles/surfaces/ai_review.inc"
#include "profiles/surfaces/ai_approvals.inc"
#include "profiles/surfaces/ai_tools.inc"
#include "profiles/surfaces/ai_validation.inc"
};
#undef UMI_STUDIO_GTK_SURFACE_PROFILE

#define UMI_STUDIO_GTK_COMMAND_PROFILE(id_, command_, label_, icon_, category_, tooltip_) \
    {id_, command_, label_, icon_, category_, tooltip_},
static const UmiStudioGtkCommandProfile RUNTIME_COMMAND_PROFILES[] = {
#include "profiles/commands/new.inc"
#include "profiles/commands/open.inc"
#include "profiles/commands/save.inc"
#include "profiles/commands/save_all.inc"
#include "profiles/commands/build.inc"
#include "profiles/commands/clean.inc"
#include "profiles/commands/package.inc"
#include "profiles/commands/run.inc"
#include "profiles/commands/stop.inc"
#include "profiles/commands/test_all.inc"
#include "profiles/commands/test_failed.inc"
#include "profiles/commands/test_stop.inc"
#include "profiles/commands/debug_start.inc"
#include "profiles/commands/debug_continue.inc"
#include "profiles/commands/debug_stop.inc"
#include "profiles/commands/vcs_view.inc"
#include "profiles/commands/vcs_refresh.inc"
#include "profiles/commands/vcs_pull.inc"
#include "profiles/commands/vcs_push.inc"
#include "profiles/commands/problems.inc"
#include "profiles/commands/terminal.inc"
#include "profiles/commands/explorer.inc"
#include "profiles/commands/search.inc"
#include "profiles/commands/tests.inc"
#include "profiles/commands/debug_view.inc"
#include "profiles/commands/ai.inc"
#include "profiles/commands/ai_review.inc"
#include "profiles/commands/layout_manager.inc"
#include "profiles/commands/command_search.inc"
#include "profiles/commands/contract.inc"
};
#undef UMI_STUDIO_GTK_COMMAND_PROFILE

#define UMI_STUDIO_GTK_LAYOUT_PROFILE(id_, label_, command_, description_) \
    {id_, label_, command_, description_},
static const UmiStudioGtkLayoutProfile RUNTIME_LAYOUT_PROFILES[] = {
#include "profiles/layouts/default.inc"
#include "profiles/layouts/debugging.inc"
#include "profiles/layouts/testing.inc"
#include "profiles/layouts/source_control.inc"
#include "profiles/layouts/ai_development.inc"
#include "profiles/layouts/review.inc"
#include "profiles/layouts/zen.inc"
};
#undef UMI_STUDIO_GTK_LAYOUT_PROFILE

#define UMI_STUDIO_GTK_STATUS_PROFILE(kind_, id_, label_) {kind_, id_, label_},
static const UmiStudioGtkStatusProfile RUNTIME_STATUS_PROFILES[] = {
#include "profiles/status/workspace.inc"
#include "profiles/status/branch.inc"
#include "profiles/status/problems.inc"
#include "profiles/status/tests.inc"
#include "profiles/status/build.inc"
#include "profiles/status/debug.inc"
#include "profiles/status/ai.inc"
#include "profiles/status/language.inc"
#include "profiles/status/encoding.inc"
#include "profiles/status/cursor.inc"
};
#undef UMI_STUDIO_GTK_STATUS_PROFILE

#define UMI_STUDIO_GTK_KEY_PROFILE(lower_, upper_, modifiers_, command_, label_) \
    {lower_, upper_, modifiers_, command_, label_},
static const UmiStudioGtkKeyProfile RUNTIME_KEY_PROFILES[] = {
#include "profiles/keybindings/command_palette.inc"
#include "profiles/keybindings/layout_browser.inc"
#include "profiles/keybindings/save.inc"
#include "profiles/keybindings/build.inc"
#include "profiles/keybindings/run.inc"
#include "profiles/keybindings/debug.inc"
#include "profiles/keybindings/debug_stop.inc"
#include "profiles/keybindings/tests.inc"
#include "profiles/keybindings/source_control.inc"
#include "profiles/keybindings/problems.inc"
#include "profiles/keybindings/terminal.inc"
};
#undef UMI_STUDIO_GTK_KEY_PROFILE

static const UmiStudioGtkSurfaceProfile *runtime_surface_profile(
    UmiStudioRuntimeSurfaceKind kind)
{
    size_t index;

    for (index = 0U;
         index < sizeof(RUNTIME_SURFACE_PROFILES) /
                 sizeof(RUNTIME_SURFACE_PROFILES[0]);
         ++index) {
        if (RUNTIME_SURFACE_PROFILES[index].kind == kind) {
            return &RUNTIME_SURFACE_PROFILES[index];
        }
    }
    return NULL;
}

static const UmiStudioGtkCommandProfile *runtime_command_profile(
    const char *profile_id)
{
    size_t index;

    if (profile_id == NULL) return NULL;

    for (index = 0U;
         index < sizeof(RUNTIME_COMMAND_PROFILES) /
                 sizeof(RUNTIME_COMMAND_PROFILES[0]);
         ++index) {
        if (strcmp(RUNTIME_COMMAND_PROFILES[index].profile_id, profile_id) == 0) {
            return &RUNTIME_COMMAND_PROFILES[index];
        }
    }
    return NULL;
}
