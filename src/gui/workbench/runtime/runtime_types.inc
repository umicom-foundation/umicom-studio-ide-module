/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_types.inc
 *
 * PURPOSE:
 *   Private GTK4 composition state for the functional Studio workbench. All
 *   reusable developer behavior remains in Umicom Framework or the established
 *   Studio service facades; this structure owns only GTK presentation state,
 *   user selections and non-owning service references.
 *
 * PRESERVATION:
 *   Phase 2 extends the Phase 1 structure. Existing fields and behaviors remain
 *   available; no feature is removed to make room for the interactive panels.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <glib/gstdio.h>

#include "umicom/application_shell/persona.h"
#include "umicom/ide_integration/ide_integration.h"
#include "umicom/diagnostics/store.h"
#include "umicom/debug/workspace.h"
#include "umicom/test_platform/result.h"
#include "umicom/test_platform/coverage.h"
#include "umicom/test_platform/output.h"
#include "umicom/platform/file_index.h"
#include "umicom/ui/workbench.h"
#include "umicom/ui/gtk4/workstation/command_bar.h"
#include "umicom/ui/gtk4/workstation/panel_frame.h"
#include "umicom/ui/gtk4/workstation/shell_header.h"
#include "umicom/studio/ai_platform.h"
#include "umicom/studio/build.h"
#include "umicom/studio/coding_assistant.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/language.h"
#include "umicom/studio/learning_centre.h"
#include "umicom/studio/new_project.h"
#include "umicom/studio/source_control.h"
#include "umicom/studio/terminal.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/view_models.h"
#include "umicom/studio/welcome_centre.h"
#include "umicom/studio_runtime/studio_runtime.h"
#include "umicom/terminal/transcript.h"
#include "umicom/test_platform/item.h"
#include "umicom/vcs/diff_document.h"
#include "umicom/vcs/workspace.h"

#define UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT 19U
#define UMI_STUDIO_GTK_RUNTIME_STATUS_COUNT 10U
#define UMI_STUDIO_GTK_RUNTIME_REFRESH_MS 750U
/* These bounds keep useful panels readable without allowing two tool regions
 * to push the central editor off a smaller display. */
#define UMI_STUDIO_GTK_RUNTIME_TOOL_WIDTH 264
#define UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH 168
#define UMI_STUDIO_GTK_RUNTIME_TOOL_MAX_WIDTH 420
#define UMI_STUDIO_GTK_RUNTIME_BOTTOM_HEIGHT 190
#define UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT 112
#define UMI_STUDIO_GTK_RUNTIME_BOTTOM_MAX_HEIGHT 320
#define UMI_STUDIO_GTK_RUNTIME_ACTIVITY_WIDTH 40
#define UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_WIDTH 360
#define UMI_STUDIO_GTK_RUNTIME_CENTRE_MIN_HEIGHT 240
#define UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT 600U
#define UMI_STUDIO_GTK_TEST_ROW_LIMIT 800U
#define UMI_STUDIO_GTK_VCS_ROW_LIMIT 800U
#define UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT 500U

typedef struct UmiStudioGtkRuntimeChrome {
    UmiStudioGtkWorkbench *owner;

    UmiApplicationShellRegistry *shell_registry;
    UmiApplicationShellState shell_state;
    UmiApplicationShellLayout shell_layout;
    UmiIdeIntegrationPlatform *ide_platform;
    UmiStudioRuntimeBootstrap *runtime_bootstrap;
    UmiStudioRuntimeHostController host_controller;
    UmiStudioRuntimeHostAdapter host_adapter;

    GtkWidget *outer_root;
    UmiGtk4WorkstationShellHeader *brand_identity;
    UmiGtk4WorkstationCommandBar *command_bar;
    UmiWsCommandBarModel command_bar_model;
    GtkWidget *application_bar;
    GtkWidget *menu_bar;
    GtkWidget *toolbar;
    GtkWidget *body;
    GtkWidget *workspace_paned;
    GtkWidget *primary_paned;
    GtkWidget *secondary_paned;
    GtkWidget *primary_cluster;
    GtkWidget *centre_stack;
    GtkWidget *welcome_surface;
    GtkWidget *workbench_surface;
    GtkWidget *activity_bar;
    GtkWidget *primary_revealer;
    GtkWidget *primary_stack;
    GtkWidget *secondary_revealer;
    GtkWidget *secondary_stack;
    GtkWidget *bottom_revealer;
    GtkWidget *bottom_stack;
    GtkWidget *document_tabs;
    GtkWidget *layout_tabs;
    GtkWidget *layout_lock_button;
    GtkWidget *status_bar;
    GtkWidget *workflow_badge;
    GtkWidget *notification_label;

    GtkWidget *surface_panels[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    GtkWidget *surface_windows[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    /* Store each close-request connection by its GTK-assigned identifier.
     * Programmatic redocking can then disconnect the exact callback without
     * converting a C function pointer to an object pointer. */
    gulong surface_close_handler_ids[
        UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    GtkWidget *activity_buttons[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    GtkWidget *status_labels[UMI_STUDIO_GTK_RUNTIME_STATUS_COUNT + 1U];

    GtkWidget *command_window;
    GtkWidget *command_entry;
    GtkWidget *command_list;
    GtkWidget *layout_window;
    GtkWidget *layout_list;
    GtkWidget *window_catalogue_window;
    GtkWidget *window_catalogue_search;
    GtkWidget *window_catalogue_list;
    GtkWidget *layout_cancel_button;
    GtkWidget *surface_location_labels[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];

    /* Phase 2 operational presentation widgets. */
    GtkWidget *editor_command_bar;
    GtkWidget *editor_find_entry;
    GtkWidget *editor_replace_entry;
    GtkWidget *editor_goto_entry;
    GtkWidget *editor_info_label;

    GtkWidget *explorer_search_entry;
    GtkWidget *explorer_list;
    GtkWidget *explorer_summary_label;

    GtkWidget *problems_list;
    GtkWidget *problems_summary_label;

    GtkWidget *test_search_entry;
    GtkWidget *test_list;
    GtkWidget *test_summary_label;
    GtkWidget *test_results_view;
    GtkWidget *test_coverage_view;
    GtkWidget *test_output_view;

    GtkWidget *source_control_list;
    GtkWidget *source_control_history_list;
    GtkWidget *source_control_branches_list;
    GtkWidget *source_control_commit_entry;
    GtkWidget *source_control_summary_label;

    GtkWidget *diff_window;
    GtkWidget *diff_text_view;
    GtkWidget *diff_left_view;
    GtkWidget *diff_right_view;
    GtkWidget *diff_summary_label;

    GtkWidget *terminal_text_view;
    GtkWidget *terminal_entry;
    GtkWidget *terminal_summary_label;

    GtkWidget *ai_prompt_view;
    GtkWidget *ai_result_view;
    GtkWidget *ai_summary_label;
    GtkWidget *ai_model_prompt_view;
    GtkWidget *ai_model_results_view;
    GtkWidget *ai_model_summary_label;

    GtkWidget *debug_thread_entry;
    GtkWidget *debug_summary_label;
    GtkWidget *debug_threads_list;
    GtkWidget *debug_frames_list;
    GtkWidget *debug_variables_list;
    GtkWidget *debug_watches_list;
    GtkWidget *debug_breakpoints_list;

    /* Documentation and lessons are ordinary workbench surfaces, so they use
     * the same docking, floating and layout persistence paths as every tool. */
    GtkWidget *documentation_location_entry;
    GtkWidget *documentation_summary_label;
    GtkWidget *documentation_preview_view;
    GtkWidget *learning_summary_label;

    GtkCssProvider *css_provider;
    guint refresh_source;
    uint32_t panel_refresh_tick;
    uint64_t appearance_revision;
    int force_close;
    int runtime_ready;
    int interaction_busy;
    int applying_workspace_geometry;
    int layout_locked;
    int layout_baseline_valid;
    int surface_regions[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_visible[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_floating[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_state_initialised[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int layout_surface_region_baseline[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int layout_surface_visible_baseline[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int layout_surface_floating_baseline[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];

    UmiUiWorkbenchState layout_edit_baseline;

    char workspace_root[UMI_STUDIO_RUNTIME_PATH_CAPACITY];
    char workspace_name[256];
    char session_path[UMI_STUDIO_RUNTIME_PATH_CAPACITY];

    char explorer_query[256];
    char selected_file_path[UMI_PATH_CAPACITY];
    char selected_test_id[128];
    char selected_vcs_path[UMI_VCS_PATH_CAPACITY];
    char last_ai_result[2048];
    char last_terminal_command[1024];
    char pending_operation_command[UMI_COMMAND_ID_CAPACITY];

    UmiStudioViewModelSnapshot view_model_snapshot;
    UmiStudioTestSnapshot test_snapshot;
    UmiStudioSourceControlSnapshot source_control_snapshot;
    UmiStudioDebuggerSnapshot debugger_snapshot;
    UmiStudioTerminalSnapshot terminal_snapshot;
    UmiAiAuthorEngineServiceSnapshot ai_snapshot;
} UmiStudioGtkRuntimeChrome;

/*
 * Provide the runtime show command palette operation used by this module and its client
 * applications.
 */
static void runtime_show_command_palette(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime show layout browser operation used by this module and its client
 * applications.
 */
static void runtime_show_layout_browser(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime show window catalogue operation used by this module and its client
 * applications.
 */
static void runtime_show_window_catalogue(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime workspace apply geometry operation used by this module and its
 * client applications.
 */
static void runtime_workspace_apply_geometry(
    UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime welcome update mode operation used by this module and its client
 * applications.
 */
static void runtime_welcome_update_mode(
    UmiStudioGtkRuntimeChrome *runtime);

/* Reveal a registered surface from local buttons such as lesson resources. */
static void runtime_show_surface(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind);


/*
 * Provide the runtime refresh services operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_refresh_services(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime on palette close operation used by this module and its client
 * applications.
 */
static gboolean runtime_on_palette_close(
    GtkWindow *window,
    gpointer user_data);

/*
 * Provide the runtime execute phase1 host command operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_execute_phase1_host_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument);
