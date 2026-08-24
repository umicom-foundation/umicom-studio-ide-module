/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_types.inc
 *
 * PURPOSE:
 *   Private GTK4 composition state for the first visible Studio-runtime phase.
 *   All reusable IDE behaviour remains in Umicom Framework; this structure owns
 *   only GTK widgets and non-owning references to the existing Studio services.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include <glib/gstdio.h>

#include "umicom/application_shell/persona.h"
#include "umicom/ide_integration/ide_integration.h"
#include "umicom/studio/ai_platform.h"
#include "umicom/studio/build.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/language.h"
#include "umicom/studio/source_control.h"
#include "umicom/studio/terminal.h"
#include "umicom/studio/tests.h"
#include "umicom/studio/view_models.h"
#include "umicom/studio_runtime/studio_runtime.h"

#define UMI_STUDIO_GTK_RUNTIME_SURFACE_COUNT 16U
#define UMI_STUDIO_GTK_RUNTIME_STATUS_COUNT 10U
#define UMI_STUDIO_GTK_RUNTIME_REFRESH_MS 750U
#define UMI_STUDIO_GTK_RUNTIME_TOOL_WIDTH 310
#define UMI_STUDIO_GTK_RUNTIME_BOTTOM_HEIGHT 220

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
    GtkWidget *application_bar;
    GtkWidget *menu_bar;
    GtkWidget *toolbar;
    GtkWidget *body;
    GtkWidget *activity_bar;
    GtkWidget *primary_revealer;
    GtkWidget *primary_stack;
    GtkWidget *secondary_revealer;
    GtkWidget *secondary_stack;
    GtkWidget *bottom_revealer;
    GtkWidget *bottom_stack;
    GtkWidget *document_tabs;
    GtkWidget *layout_tabs;
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

    GtkCssProvider *css_provider;
    guint refresh_source;
    uint32_t panel_refresh_tick;
    int force_close;
    int runtime_ready;

    char workspace_root[UMI_STUDIO_RUNTIME_PATH_CAPACITY];
    char workspace_name[256];
    char session_path[UMI_STUDIO_RUNTIME_PATH_CAPACITY];

    UmiStudioViewModelSnapshot view_model_snapshot;
    UmiStudioTestSnapshot test_snapshot;
    UmiStudioSourceControlSnapshot source_control_snapshot;
    UmiStudioDebuggerSnapshot debugger_snapshot;
    UmiStudioTerminalSnapshot terminal_snapshot;
    UmiAiAuthorEngineServiceSnapshot ai_snapshot;
} UmiStudioGtkRuntimeChrome;

static void runtime_show_command_palette(UmiStudioGtkRuntimeChrome *runtime);
static void runtime_show_layout_browser(UmiStudioGtkRuntimeChrome *runtime);
