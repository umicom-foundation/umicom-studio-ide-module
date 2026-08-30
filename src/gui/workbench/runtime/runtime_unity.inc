/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_unity.inc
 *
 * PURPOSE:
 *   Private GTK4 unity include for the functional Studio Runtime composition.
 *
 * BUILD NOTE:
 *   workbench_window.c remains the established GTK4 executable translation
 *   unit. Phase 2 extends its presentation additively and does not restructure
 *   CMake, remove the historical adapter, or duplicate Framework business
 *   services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "runtime_types.inc"
#include "runtime_helpers.inc"
#include "runtime_profile_catalogues.inc"
#include "runtime_theme.inc"
#include "runtime_notifications.inc"

#include "interaction/selection.inc"
#include "interaction/registered_command.inc"
#include "interaction/prompt_dialog.inc"
#include "interaction/confirm_dialog.inc"
#include "interaction/file_dialog.inc"
#include "interaction/documents.inc"
#include "interaction/explorer.inc"
#include "interaction/problems.inc"
#include "interaction/tests.inc"
#include "interaction/terminal.inc"
#include "interaction/debug.inc"
#include "interaction/ai.inc"
#include "interaction/source_control.inc"
#include "interaction/source_control_details.inc"
#include "interaction/operation_catalogue.inc"
#include "interaction/context_actions.inc"
#include "interaction/command_router.inc"
#include "interaction/editor_controls.inc"
#include "interaction/test_details.inc"
#include "interaction/debug_details.inc"

#include "runtime_actions.inc"
#include "runtime_service_snapshots.inc"
#include "runtime_panel_helpers.inc"

#include "surfaces/explorer.inc"
#include "surfaces/editor.inc"
#include "surfaces/outline.inc"
#include "surfaces/search.inc"
#include "surfaces/problems.inc"
#include "surfaces/output.inc"
#include "surfaces/terminal.inc"
#include "surfaces/source_control.inc"
#include "surfaces/test_explorer.inc"
#include "surfaces/debug.inc"
#include "surfaces/build.inc"
#include "surfaces/ai_chat.inc"
#include "surfaces/ai_review.inc"
#include "surfaces/ai_approvals.inc"
#include "surfaces/ai_tools.inc"
#include "surfaces/ai_validation.inc"

#include "runtime_surface_registry.inc"
#include "runtime_command_palette.inc"
#include "runtime_layout_browser.inc"
#include "runtime_keyboard.inc"
#include "runtime_menu_bar.inc"
#include "runtime_application_bar.inc"
#include "runtime_toolbar.inc"
#include "runtime_activity_bar.inc"
#include "runtime_document_tabs.inc"
#include "runtime_layout_tabs.inc"
#include "runtime_status_bar.inc"
#include "runtime_host_adapter.inc"
#include "runtime_close_guard.inc"
#include "runtime_framework.inc"
#include "runtime_shell.inc"
#include "interaction/refresh.inc"
#include "runtime_refresh.inc"
#include "runtime_attach.inc"
