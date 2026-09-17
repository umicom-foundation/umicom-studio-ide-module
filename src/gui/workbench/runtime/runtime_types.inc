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
 *   Real editor and tool bodies remain intact. Legacy outer splitter fields
 *   are replaced by one Framework host; compatibility arrays are projections.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <glib/gstdio.h>
#include "umicom/ui/gtk4/developer_dialog.h"
#include "umicom/studio/security.h"

#include "umicom/application_shell/persona.h"
#include "umicom/application/component_catalogue.h"
#include "umicom/application/presentation/presentation.h"
#include "umicom/browser/source_preview.h"
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
#include "umicom/ui/gtk4/workstation/workspace_layout_host.h"
#include "umicom/ui/gtk4/workstation/workspace_storage.h"
#include "umicom/ui/gtk4/workstation/layout_library.h"
#include "umicom/ui/workspace_checkpoint.h"
#include "umicom/ui/workspace_library_checkpoint.h"
#include "umicom/studio/professional_workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/studio/ai_platform.h"
#include "umicom/studio/build.h"
#include "umicom/studio/coding_assistant.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/designer.h"
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

#define UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT 27U
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
    /* The professional workspace is the only outer-panel layout owner. GTK
     * retains presentation through one Framework host, not a second canvas. */
    UmiStudioProfessionalWorkspace *professional_workspace;
    UmiGtk4WorkspaceLayoutHost *workspace_host;
    /* The library borrows the existing workspace owner and stores copied rows only. */
    UmiGtk4WorkspaceLayoutLibrary *workspace_library;
    /* One optional Data Server connection stores explicit layout checkpoints.
     * A caller-supplied server is borrowed; only the native default is owned. */
    UmiDataServer *workspace_storage;
    int owns_workspace_storage;
    UmiUiWorkspaceCheckpointReport workspace_checkpoint;
    /* Whole-library evidence has its own storage namespace and never replaces
     * the active checkpoint's CAS token or silently restores at startup. */
    UmiUiWorkspaceLibraryCheckpointReport workspace_library_checkpoint;
    UmiStatus workspace_library_storage_status;
    uint64_t workspace_library_storage_generation;
    bool workspace_library_has_saved;
    bool workspace_library_save_conflict;
    UmiStatus workspace_storage_status;
    int syncing_workspace_controls;
    UmiStudioRuntimeSurfaceKind last_presented_surface;
    /* Framework owns provider selection and preview safety; Studio stores the
     * service handle and only presents its bounded result. */
    UmiBrowserSourcePreviewService *source_preview_service;

    GtkWidget *outer_root;
    UmiGtk4WorkstationCommandBar *command_bar;
    UmiWsCommandBarModel command_bar_model;
    GtkWidget *application_bar;
    GtkWidget *menu_bar;
    GtkWidget *toolbar;
    GtkWidget *body;
    GtkWidget *centre_stack;
    GtkWidget *welcome_surface;
    GtkWidget *workbench_surface;
    GtkWidget *editor_adapter_root;
    GtkWidget *activity_bar;
    GtkWidget *document_tabs;
    GtkWidget *layout_tabs;
    GtkWidget *workspace_layout_selector;
    GtkWidget *workspace_save_button;
    GtkWidget *workspace_restore_button;
    GtkWidget *workspace_storage_label;
    GtkWidget *layout_lock_button;
    GtkWidget *status_bar;
    GtkWidget *workflow_badge;
    GtkWidget *notification_label;

    GtkWidget *surface_panels[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
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

    GtkWidget *live_preview_summary_label;
    GtkWidget *live_preview_text_view;
    uint64_t live_preview_model_revision;
    char live_preview_view_id[UMI_UI_ID_CAPACITY];

    GtkWidget *explorer_search_entry;
    GtkWidget *explorer_list;
    GtkWidget *explorer_summary_label;

    GtkWidget *problems_list;
    uint64_t problemsModelRevision;
    int problemsRendered;
    char problemsSelectedId[UMI_DIAGNOSTIC_ID_CAPACITY];
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

    UmiGtk4DeveloperDialog *project_dialog;
    UmiGtk4DeveloperDialog *build_settings_dialog;
    GtkWidget *build_output_view;
    GtkWidget *build_progress_label;
    uint64_t build_output_operation;

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
     * the same docking and floating owner as every tool. Canvas edits are session-only. */
    GtkWidget *documentation_location_entry;
    GtkWidget *documentation_search_entry;
    GtkWidget *documentation_summary_label;
    GtkWidget *documentation_preview_view;
    GtkWidget *learning_summary_label;

    GtkCssProvider *css_provider;
    guint refresh_source;
    guint designer_refresh_source;
    guint notification_source;
    /* Avoid rebuilding search presentation on unchanged timer ticks. */
    int application_bar_width;
    uint32_t panel_refresh_tick;
    int force_close;
    int runtime_ready;
    int interaction_busy;
    int layout_locked;
    int layout_baseline_valid;
    UmiStudioRuntimeSurfaceKind designer_active_surface;
    int surface_regions[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_visible[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_floating[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];
    int surface_state_initialised[UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT + 1U];

    char workspace_root[UMI_STUDIO_RUNTIME_PATH_CAPACITY];
    char workspace_name[256];
    char session_path[UMI_STUDIO_RUNTIME_PATH_CAPACITY];

    char explorer_query[256];
    char designer_palette_query[UMI_DESIGNER_WORKSPACE_QUERY_CAPACITY];
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
/* Rebuild shared workspace chrome while retaining its real operational bodies. */
static UmiStatus runtime_rebuild_surface_panels(
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

/* Select a native preset or custom named layout through the sole owner. */
static UmiStatus runtime_workspace_select_layout(
    UmiStudioGtkRuntimeChrome *runtime, const char *layout_id);

/* Borrow the sole professional layout owner for command policy checks. */
static UmiUiWorkspaceCustomisation *runtime_workspace_model(UmiStudioGtkRuntimeChrome *runtime);
/* Open the strict native singleton through the existing layout owner. */
static UmiStatus runtime_workspace_open_surface(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind, const char *placement_id);
/* Project the existing professional model through the single shared host. */
static UmiStatus runtime_workspace_rebuild(UmiStudioGtkRuntimeChrome *runtime);
/* Explicit save and staged restore share Framework checkpoint validation. */
static UmiStatus runtime_workspace_save(UmiStudioGtkRuntimeChrome *runtime);
static UmiStatus runtime_workspace_restore(UmiStudioGtkRuntimeChrome *runtime);
static UmiStatus runtime_workspace_bind_storage(UmiStudioGtkRuntimeChrome *runtime,
    UmiDataServer *server, int restore_saved);
static void runtime_workspace_prepare_storage(UmiStudioGtkRuntimeChrome *runtime);
static void runtime_workspace_update_storage_controls(UmiStudioGtkRuntimeChrome *runtime);
/* Focus an already visible model surface without changing its visibility. */
static void runtime_workspace_focus_surface(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind);
/* Invalidate borrowed runtime callbacks before releasing or replacing a body. */
static void runtime_invalidate_surface_tree(GtkWidget *root, UmiStudioGtkRuntimeChrome *runtime);
/* Refresh only deliberately rebuilt designer providers, retaining other drafts. */
static UmiStatus runtime_workspace_refresh_designer(UmiStudioGtkRuntimeChrome *runtime);

/* Refresh the active-document preview from Framework provider output. */
static UmiStatus runtime_live_preview_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    int force);


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
