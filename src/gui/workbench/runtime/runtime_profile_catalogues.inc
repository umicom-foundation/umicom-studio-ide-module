/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_profile_catalogues.inc
 *
 * PURPOSE:
 *   Compile the private GTK presentation profiles into deterministic catalogues.
 *   These catalogues contain only frontend metadata; commands, IDE behavior,
 *   semantic layouts and surface identity remain authoritative in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
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

typedef enum UmiStudioGtkOperationArgumentKind {
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE = 0,
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_OPTIONAL = 1,
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT = 2,
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_PATH = 3,
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_THREAD = 4,
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEST = 5
} UmiStudioGtkOperationArgumentKind;


typedef struct UmiStudioGtkContextActionProfile {
    const char *panel_id;
    const char *command_id;
    uint32_t priority;
} UmiStudioGtkContextActionProfile;

typedef struct UmiStudioGtkOperationProfile {
    const char *profile_id;
    const char *command_id;
    const char *title;
    const char *category;
    const char *description;
    UmiStudioGtkOperationArgumentKind argument_kind;
} UmiStudioGtkOperationProfile;



#define UMI_STUDIO_GTK_CONTEXT_ACTION(panel_, command_, priority_) \
    {panel_, command_, priority_},
static const UmiStudioGtkContextActionProfile RUNTIME_CONTEXT_ACTION_PROFILES[] = {
#include "profiles/context_actions/ai_code_chat.inc"
#include "profiles/context_actions/ai_patch_apply.inc"
#include "profiles/context_actions/ai_patch_approve.inc"
#include "profiles/context_actions/ai_complete_code.inc"
#include "profiles/context_actions/ai_explain_code.inc"
#include "profiles/context_actions/ai_generate_tests.inc"
#include "profiles/context_actions/ai_new_session.inc"
#include "profiles/context_actions/ai_refactor_code.inc"
#include "profiles/context_actions/ai_refresh_health.inc"
#include "profiles/context_actions/ai_patch_revert.inc"
#include "profiles/context_actions/ai_save_session.inc"
#include "profiles/context_actions/build_compile.inc"
#include "profiles/context_actions/build_test.inc"
#include "profiles/context_actions/build_cancel.inc"
#include "profiles/context_actions/build_clean.inc"
#include "profiles/context_actions/build_clear_history.inc"
#include "profiles/context_actions/build_configure.inc"
#include "profiles/context_actions/build_filter.inc"
#include "profiles/context_actions/build_install.inc"
#include "profiles/context_actions/build_invalidate.inc"
#include "profiles/context_actions/build_refresh.inc"
#include "profiles/context_actions/build_retry.inc"
#include "profiles/context_actions/build_run.inc"
#include "profiles/context_actions/build_run_all.inc"
#include "profiles/context_actions/build_run_next.inc"
#include "profiles/context_actions/build_select_artifact.inc"
#include "profiles/context_actions/build_select_node.inc"
#include "profiles/context_actions/build_select_operation.inc"
#include "profiles/context_actions/data_integrity.inc"
#include "profiles/context_actions/debug_add_breakpoint.inc"
#include "profiles/context_actions/debug_add_watch.inc"
#include "profiles/context_actions/debug_clear_console.inc"
#include "profiles/context_actions/debug_continue.inc"
#include "profiles/context_actions/debug_set_breakpoint_enabled.inc"
#include "profiles/context_actions/debug_initialize.inc"
#include "profiles/context_actions/debug_pause.inc"
#include "profiles/context_actions/debug_remove_breakpoint.inc"
#include "profiles/context_actions/debug_remove_watch.inc"
#include "profiles/context_actions/debug_select_scope.inc"
#include "profiles/context_actions/debug_select_frame.inc"
#include "profiles/context_actions/debug_select_thread.inc"
#include "profiles/context_actions/debug_start.inc"
#include "profiles/context_actions/debug_step_in.inc"
#include "profiles/context_actions/debug_step_out.inc"
#include "profiles/context_actions/debug_next.inc"
#include "profiles/context_actions/debug_stop.inc"
#include "profiles/context_actions/developer_report.inc"
#include "profiles/context_actions/plugins_report.inc"
#include "profiles/context_actions/documents_save_all.inc"
#include "profiles/context_actions/language_initialize.inc"
#include "profiles/context_actions/language_workspace_symbols.inc"
#include "profiles/context_actions/messages_flush.inc"
#include "profiles/context_actions/messages_replay.inc"
#include "profiles/context_actions/observability_report.inc"
#include "profiles/context_actions/resilience_report.inc"
#include "profiles/context_actions/output_clear.inc"
#include "profiles/context_actions/diagnostics_clear.inc"
#include "profiles/context_actions/process_cancel.inc"
#include "profiles/context_actions/process_report.inc"
#include "profiles/context_actions/recovery_purge.inc"
#include "profiles/context_actions/security_report.inc"
#include "profiles/context_actions/session_save.inc"
#include "profiles/context_actions/vcs_branch_checkout.inc"
#include "profiles/context_actions/vcs_commit.inc"
#include "profiles/context_actions/vcs_commit_composed.inc"
#include "profiles/context_actions/vcs_branch_create.inc"
#include "profiles/context_actions/vcs_branch_delete.inc"
#include "profiles/context_actions/vcs_diff_selected.inc"
#include "profiles/context_actions/vcs_diff_selected_staged.inc"
#include "profiles/context_actions/vcs_discard.inc"
#include "profiles/context_actions/vcs_discard_selected.inc"
#include "profiles/context_actions/vcs_fetch.inc"
#include "profiles/context_actions/vcs_filter.inc"
#include "profiles/context_actions/vcs_diff.inc"
#include "profiles/context_actions/vcs_pull.inc"
#include "profiles/context_actions/vcs_push.inc"
#include "profiles/context_actions/vcs_refresh.inc"
#include "profiles/context_actions/vcs_select_branch.inc"
#include "profiles/context_actions/vcs_select_change.inc"
#include "profiles/context_actions/vcs_select_commit.inc"
#include "profiles/context_actions/vcs_select_remote.inc"
#include "profiles/context_actions/vcs_set_commit_message.inc"
#include "profiles/context_actions/vcs_stage_all.inc"
#include "profiles/context_actions/vcs_stage.inc"
#include "profiles/context_actions/vcs_stage_selected.inc"
#include "profiles/context_actions/vcs_unstage_all.inc"
#include "profiles/context_actions/vcs_unstage.inc"
#include "profiles/context_actions/vcs_unstage_selected.inc"
#include "profiles/context_actions/tasks_report.inc"
#include "profiles/context_actions/tasks_wait_idle.inc"
#include "profiles/context_actions/terminal_clear.inc"
#include "profiles/context_actions/terminal_history_clear.inc"
#include "profiles/context_actions/terminal_close.inc"
#include "profiles/context_actions/terminal_execute.inc"
#include "profiles/context_actions/terminal_new.inc"
#include "profiles/context_actions/terminal_next.inc"
#include "profiles/context_actions/terminal_previous.inc"
#include "profiles/context_actions/terminal_search.inc"
#include "profiles/context_actions/terminal_split_horizontal.inc"
#include "profiles/context_actions/terminal_split_vertical.inc"
#include "profiles/context_actions/tests_clear_coverage.inc"
#include "profiles/context_actions/tests_clear_output.inc"
#include "profiles/context_actions/tests_clear_results.inc"
#include "profiles/context_actions/tests_debug_selected.inc"
#include "profiles/context_actions/tests_discover.inc"
#include "profiles/context_actions/tests_filter.inc"
#include "profiles/context_actions/tests_repeat_selected.inc"
#include "profiles/context_actions/tests_rerun_failed.inc"
#include "profiles/context_actions/tests_run_all.inc"
#include "profiles/context_actions/tests_run_selected.inc"
#include "profiles/context_actions/tests_run_coverage.inc"
#include "profiles/context_actions/tests_select.inc"
#include "profiles/context_actions/tests_stop.inc"
#include "profiles/context_actions/workspace_close.inc"
#include "profiles/context_actions/workspace_refresh.inc"
#include "profiles/context_actions/watcher_scan.inc"
};
#undef UMI_STUDIO_GTK_CONTEXT_ACTION

#define UMI_STUDIO_GTK_OPERATION_PROFILE(id_, command_, title_, category_, description_, argument_) \
    {id_, command_, title_, category_, description_, argument_},
static const UmiStudioGtkOperationProfile RUNTIME_OPERATION_PROFILES[] = {
#include "profiles/operations/session_save.inc"
#include "profiles/operations/documents_save_all.inc"
#include "profiles/operations/tasks_wait_idle.inc"
#include "profiles/operations/tasks_report.inc"
#include "profiles/operations/recovery_purge.inc"
#include "profiles/operations/workspace_refresh.inc"
#include "profiles/operations/workspace_close.inc"
#include "profiles/operations/watcher_scan.inc"
#include "profiles/operations/data_integrity.inc"
#include "profiles/operations/messages_flush.inc"
#include "profiles/operations/messages_replay.inc"
#include "profiles/operations/security_report.inc"
#include "profiles/operations/plugins_report.inc"
#include "profiles/operations/observability_report.inc"
#include "profiles/operations/resilience_report.inc"
#include "profiles/operations/developer_report.inc"
#include "profiles/operations/build_configure.inc"
#include "profiles/operations/build_compile.inc"
#include "profiles/operations/build_test.inc"
#include "profiles/operations/build_clean.inc"
#include "profiles/operations/build_run.inc"
#include "profiles/operations/build_install.inc"
#include "profiles/operations/build_retry.inc"
#include "profiles/operations/build_cancel.inc"
#include "profiles/operations/build_filter.inc"
#include "profiles/operations/build_select_node.inc"
#include "profiles/operations/build_select_operation.inc"
#include "profiles/operations/build_select_artifact.inc"
#include "profiles/operations/build_run_next.inc"
#include "profiles/operations/build_run_all.inc"
#include "profiles/operations/build_invalidate.inc"
#include "profiles/operations/build_refresh.inc"
#include "profiles/operations/build_clear_history.inc"
#include "profiles/operations/tests_discover.inc"
#include "profiles/operations/tests_filter.inc"
#include "profiles/operations/tests_select.inc"
#include "profiles/operations/tests_run_all.inc"
#include "profiles/operations/tests_run_selected.inc"
#include "profiles/operations/tests_debug_selected.inc"
#include "profiles/operations/tests_run_coverage.inc"
#include "profiles/operations/tests_rerun_failed.inc"
#include "profiles/operations/tests_repeat_selected.inc"
#include "profiles/operations/tests_stop.inc"
#include "profiles/operations/tests_clear_results.inc"
#include "profiles/operations/tests_clear_output.inc"
#include "profiles/operations/tests_clear_coverage.inc"
#include "profiles/operations/terminal_execute.inc"
#include "profiles/operations/terminal_clear.inc"
#include "profiles/operations/terminal_new.inc"
#include "profiles/operations/terminal_close.inc"
#include "profiles/operations/terminal_next.inc"
#include "profiles/operations/terminal_previous.inc"
#include "profiles/operations/terminal_split_horizontal.inc"
#include "profiles/operations/terminal_split_vertical.inc"
#include "profiles/operations/terminal_history_clear.inc"
#include "profiles/operations/terminal_search.inc"
#include "profiles/operations/process_report.inc"
#include "profiles/operations/process_cancel.inc"
#include "profiles/operations/diagnostics_clear.inc"
#include "profiles/operations/output_clear.inc"
#include "profiles/operations/language_initialize.inc"
#include "profiles/operations/language_workspace_symbols.inc"
#include "profiles/operations/debug_initialize.inc"
#include "profiles/operations/debug_start.inc"
#include "profiles/operations/debug_continue.inc"
#include "profiles/operations/debug_pause.inc"
#include "profiles/operations/debug_next.inc"
#include "profiles/operations/debug_step_in.inc"
#include "profiles/operations/debug_step_out.inc"
#include "profiles/operations/debug_stop.inc"
#include "profiles/operations/debug_add_breakpoint.inc"
#include "profiles/operations/debug_set_breakpoint_enabled.inc"
#include "profiles/operations/debug_remove_breakpoint.inc"
#include "profiles/operations/debug_add_watch.inc"
#include "profiles/operations/debug_remove_watch.inc"
#include "profiles/operations/debug_select_thread.inc"
#include "profiles/operations/debug_select_frame.inc"
#include "profiles/operations/debug_select_scope.inc"
#include "profiles/operations/debug_clear_console.inc"
#include "profiles/operations/vcs_refresh.inc"
#include "profiles/operations/vcs_stage.inc"
#include "profiles/operations/vcs_unstage.inc"
#include "profiles/operations/vcs_stage_all.inc"
#include "profiles/operations/vcs_unstage_all.inc"
#include "profiles/operations/vcs_discard.inc"
#include "profiles/operations/vcs_commit.inc"
#include "profiles/operations/vcs_fetch.inc"
#include "profiles/operations/vcs_pull.inc"
#include "profiles/operations/vcs_push.inc"
#include "profiles/operations/vcs_branch_create.inc"
#include "profiles/operations/vcs_branch_checkout.inc"
#include "profiles/operations/vcs_branch_delete.inc"
#include "profiles/operations/vcs_diff.inc"
#include "profiles/operations/vcs_filter.inc"
#include "profiles/operations/vcs_select_change.inc"
#include "profiles/operations/vcs_select_commit.inc"
#include "profiles/operations/vcs_select_branch.inc"
#include "profiles/operations/vcs_select_remote.inc"
#include "profiles/operations/vcs_set_commit_message.inc"
#include "profiles/operations/vcs_stage_selected.inc"
#include "profiles/operations/vcs_unstage_selected.inc"
#include "profiles/operations/vcs_discard_selected.inc"
#include "profiles/operations/vcs_commit_composed.inc"
#include "profiles/operations/vcs_diff_selected.inc"
#include "profiles/operations/vcs_diff_selected_staged.inc"
#include "profiles/operations/ai_refresh_health.inc"
#include "profiles/operations/ai_new_session.inc"
#include "profiles/operations/ai_save_session.inc"
#include "profiles/operations/ai_code_chat.inc"
#include "profiles/operations/ai_complete_code.inc"
#include "profiles/operations/ai_explain_code.inc"
#include "profiles/operations/ai_refactor_code.inc"
#include "profiles/operations/ai_generate_tests.inc"
#include "profiles/operations/ai_patch_approve.inc"
#include "profiles/operations/ai_patch_apply.inc"
#include "profiles/operations/ai_patch_revert.inc"
};
#undef UMI_STUDIO_GTK_OPERATION_PROFILE

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
#include "profiles/surfaces/ai_model_comparison.inc"
#include "profiles/surfaces/learning.inc"
#include "profiles/surfaces/documentation.inc"
#include "profiles/surfaces/visual_designer.inc"
#include "profiles/surfaces/component_palette.inc"
#include "profiles/surfaces/component_structure.inc"
#include "profiles/surfaces/object_inspector.inc"
#include "profiles/surfaces/design_code.inc"
#include "profiles/surfaces/design_mixed.inc"
#include "profiles/surfaces/design_preview.inc"
#include "profiles/surfaces/live_preview.inc"
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
#include "profiles/commands/live_preview.inc"
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
#include "profiles/layouts/learning.inc"
#include "profiles/layouts/visual_design.inc"
#include "profiles/layouts/code_preview.inc"
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

/*
 * Provide the runtime surface profile operation used by this module and its client
 * applications.
 */
static const UmiStudioGtkSurfaceProfile *runtime_surface_profile(
    UmiStudioRuntimeSurfaceKind kind)
{
    size_t index;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_SURFACE_PROFILES) /
                 sizeof(RUNTIME_SURFACE_PROFILES[0]);
         ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (RUNTIME_SURFACE_PROFILES[index].kind == kind) {
            return &RUNTIME_SURFACE_PROFILES[index];
        }
    }
    return NULL;
}

/*
 * Provide the runtime command profile operation used by this module and its client
 * applications.
 */
static const UmiStudioGtkCommandProfile *runtime_command_profile(
    const char *profile_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (profile_id == NULL) return NULL;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_COMMAND_PROFILES) /
                 sizeof(RUNTIME_COMMAND_PROFILES[0]);
         ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(RUNTIME_COMMAND_PROFILES[index].profile_id, profile_id) == 0) {
            return &RUNTIME_COMMAND_PROFILES[index];
        }
    }
    return NULL;
}


/*
 * Provide the runtime operation profile operation used by this module and its client
 * applications.
 */
static const UmiStudioGtkOperationProfile *runtime_operation_profile(
    const char *command_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id == NULL) return NULL;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_OPERATION_PROFILES) /
                 sizeof(RUNTIME_OPERATION_PROFILES[0]);
         ++index) {
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (strcmp(
                RUNTIME_OPERATION_PROFILES[index].command_id,
                command_id) == 0) {
            return &RUNTIME_OPERATION_PROFILES[index];
        }
    }
    return NULL;
}

/*
 * Return the number of records represented by runtime operation profile without changing
 * their state.
 */
static size_t runtime_operation_profile_count(void)
{
    return sizeof(RUNTIME_OPERATION_PROFILES) /
           sizeof(RUNTIME_OPERATION_PROFILES[0]);
}


/*
 * Return the number of records represented by runtime context action profile without
 * changing their state.
 */
static size_t runtime_context_action_profile_count(void)
{
    return sizeof(RUNTIME_CONTEXT_ACTION_PROFILES) /
           sizeof(RUNTIME_CONTEXT_ACTION_PROFILES[0]);
}

