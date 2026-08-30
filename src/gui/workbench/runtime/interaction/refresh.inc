/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/refresh.inc
 *
 * PURPOSE:
 *   Refresh operational panel contents in place. Phase 2 deliberately stops
 *   destroying/recreating interactive widgets on the timer so typed terminal,
 *   Git commit, Explorer search and AI prompt text cannot disappear.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_refresh_interactive_panels(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->interaction_busy) return;

    if (runtime->explorer_list != NULL &&
        (runtime->explorer_search_entry == NULL ||
         !gtk_widget_has_focus(runtime->explorer_search_entry))) {
        runtime_explorer_refresh_list(runtime);
    }

    if (runtime->problems_list != NULL) {
        runtime_problems_refresh_list(runtime);
    }

    if (runtime->test_list != NULL &&
        (runtime->test_search_entry == NULL ||
         !gtk_widget_has_focus(runtime->test_search_entry))) {
        runtime_tests_refresh_list(runtime);
    }

    if (runtime->source_control_list != NULL &&
        (runtime->source_control_commit_entry == NULL ||
         !gtk_widget_has_focus(runtime->source_control_commit_entry))) {
        runtime_source_control_refresh_list(runtime);
        runtime_source_control_refresh_details(runtime);
    }

    if (runtime->terminal_text_view != NULL &&
        (runtime->terminal_entry == NULL ||
         !gtk_widget_has_focus(runtime->terminal_entry))) {
        runtime_terminal_refresh_transcript(runtime);
    }

    runtime_editor_update_info(runtime);
    runtime_test_details_refresh(runtime);
    runtime_debug_update_summary(runtime);
    runtime_debug_details_refresh(runtime);
}
