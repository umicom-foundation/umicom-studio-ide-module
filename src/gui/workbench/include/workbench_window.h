/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/include/workbench_window.h
 *
 * PURPOSE:
 *   Declare the private GTK4 host for the Framework-backed Studio workbench and its context-link integration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_GTK_WORKBENCH_WINDOW_H
#define UMICOM_STUDIO_GTK_WORKBENCH_WINDOW_H

#include <gtk/gtk.h>

#include "umicom/studio/ui.h"
#include "umicom/studio_runtime/types.h"
#include "umicom/ui/gtk4/workstation/workspace_layout_host.h"
#include "umicom/ui/gtk4/workstation/shell_header.h"
#include "umicom/ui/workspace_checkpoint.h"

/**
 * Represent the studio gtk workbench data shared with callers of this public contract.
 */
typedef struct UmiStudioGtkWorkbench UmiStudioGtkWorkbench;

/**
 * Native host policy. Zero-initialize for an unpresented, non-persistent host
 * without automatic refresh; use the defaults function for normal startup.
 * persist_session zero disables all GTK runtime session storage and restore,
 * regardless of restore_session. It does not discard any saved user files.
 * These options do not reconfigure the caller-owned Studio services or invoke
 * external tools. Service fixtures must provide their own isolated storage.
 */
typedef struct UmiStudioGtkWorkbenchOptions {
    int present_window;
    int persist_session;
    int restore_session;
    int automatic_refresh;
} UmiStudioGtkWorkbenchOptions;

/** Return the existing interactive startup and session behaviour. */
UmiStudioGtkWorkbenchOptions umi_studio_gtk_workbench_options_default(void);

/**
 * Construct the real Studio adapter with explicit native lifetime policy.
 * Options are copied; NULL selects normal defaults. With present_window zero,
 * construction never presents the main window. Test callers should disable
 * all four options and avoid commands which intentionally open other windows.
 */
UmiStatus umi_studio_gtk_workbench_create_with_options(
    GtkApplication *application,
    UmiStudioUi *ui,
    UmiDesktopShellModel *desktop_shell,
    const UmiStudioGtkWorkbenchOptions *options,
    UmiStudioGtkWorkbench **out_workbench);

/**
 * Initialise studio gtk workbench from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_gtk_workbench_create(
    GtkApplication *application,
    UmiStudioUi *ui,
    UmiDesktopShellModel *desktop_shell,
    UmiStudioGtkWorkbench **out_workbench);
/**
 * Release or reset state held by studio gtk workbench so the same storage can be reused
 * safely.
 */
void umi_studio_gtk_workbench_destroy(
    UmiStudioGtkWorkbench *workbench);
/**
 * Provide the studio gtk workbench window operation used by this module and its client
 * applications.
 */
GtkWindow *umi_studio_gtk_workbench_window(
    UmiStudioGtkWorkbench *workbench);
/**
 * Copy the live in-process UAT driver used by Studio's automated-test panel.
 * The returned interface borrows the workbench and is valid until destruction.
 */
UmiStatus umi_studio_gtk_workbench_automation(
    UmiStudioGtkWorkbench *workbench,
    UmiUiAutomationDriver *out_driver);
/**
 * Provide the studio gtk workbench refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_gtk_workbench_refresh(
    UmiStudioGtkWorkbench *workbench);

/** Refresh the topmost product identity from the existing appearance owner.
 * Window-title notifications independently update centred document/project
 * context. No inner application strip or layout model is created. */
UmiStatus umi_studio_gtk_workbench_refresh_titlebar(UmiStudioGtkWorkbench *workbench);

/** Copy the authoritative outer-panel layout into caller-owned storage. The
 * record is large; acceptance tools should allocate it on the heap. */
UmiStatus umi_studio_gtk_workbench_workspace_snapshot(
    UmiStudioGtkWorkbench *workbench, UmiUiWorkspaceLayout *out_layout);
/** Observe the actual native host without exposing its mutable owner. */
UmiStatus umi_studio_gtk_workbench_workspace_host_snapshot(
    UmiStudioGtkWorkbench *workbench, UmiGtk4WorkspaceLayoutHostSnapshot *out_snapshot);
/** Use the same transactions as the native New Layout and Edit Layout UI.
 * A blank layout starts an edit and requires a qualified Studio layout ID.
 * These operations do not save a durable canvas checkpoint. */
UmiStatus umi_studio_gtk_workbench_workspace_create_blank(
    UmiStudioGtkWorkbench *workbench, const char *layout_id, const char *name);
/** Capture the active arrangement as the baseline for one reversible edit. */
UmiStatus umi_studio_gtk_workbench_workspace_begin_edit(UmiStudioGtkWorkbench *workbench);
/** Apply the in-memory arrangement and lock further geometry changes. */
UmiStatus umi_studio_gtk_workbench_workspace_commit_edit(UmiStudioGtkWorkbench *workbench);
/** Restore the existing baseline, including panel visibility and geometry. */
UmiStatus umi_studio_gtk_workbench_workspace_cancel_edit(UmiStudioGtkWorkbench *workbench);
/** Open a real registered tool. NULL keeps its ordinary placement; an explicit
 * placement requires Edit Layout. Opening a detached tool may present it. */
UmiStatus umi_studio_gtk_workbench_workspace_open_surface(
    UmiStudioGtkWorkbench *workbench, UmiStudioRuntimeSurfaceKind kind,
    const char *placement_id);
/** Queue the same revision-checked request as a completed native canvas drag.
 * OK means queued, not accepted. Process the GTK context, then inspect the
 * copied layout to confirm acceptance. Rebuild or destruction cancels it. */
UmiStatus umi_studio_gtk_workbench_workspace_request_canvas_geometry(
    UmiStudioGtkWorkbench *workbench, const char *window_id,
    const UmiApplicationSuiteLayoutRect *rect, uint64_t expected_layout_revision);

/** Run the same status synchronization as the native refresh timer without
 * enabling a recurring source. Normal document reconciliation still applies. */
UmiStatus umi_studio_gtk_workbench_workspace_synchronise(UmiStudioGtkWorkbench *workbench);

/** Bind an existing Data Server for canvas checkpoints without opening any
 * files or user directories. The server is borrowed until workbench teardown
 * or another binding. An active edit is rejected. A read error is returned
 * while retaining the binding for a later recovery retry; live layout stays
 * unchanged. A missing checkpoint is a successful empty binding. */
UmiStatus umi_studio_gtk_workbench_workspace_bind_storage(
    UmiStudioGtkWorkbench *workbench, UmiDataServer *server, int restore_saved);
/** Save only an applied layout through the same command used by the GUI.
 * A stale storage revision reports a conflict instead of overwriting it. */
UmiStatus umi_studio_gtk_workbench_workspace_save(UmiStudioGtkWorkbench *workbench);
/** Restore the last explicit checkpoint through a validated native candidate.
 * Source documents and their unsaved buffers remain independently owned. */
UmiStatus umi_studio_gtk_workbench_workspace_restore(UmiStudioGtkWorkbench *workbench);
/** Copy storage evidence without exposing a mutable model or private path.
 * Returns UNAVAILABLE when no canvas Data Server has been bound. */
UmiStatus umi_studio_gtk_workbench_workspace_storage_snapshot(
    UmiStudioGtkWorkbench *workbench, UmiUiWorkspaceCheckpointReport *out_report);

/** Observe native background policy without publishing a personal file path. */
typedef struct UmiStudioGtkWorkbenchActivitySnapshot {
    int automatic_refresh_active;
    int session_storage_configured;
} UmiStudioGtkWorkbenchActivitySnapshot;
/** Copy observed runtime activity; this does not change either policy flag. */
UmiStatus umi_studio_gtk_workbench_activity_snapshot(
    UmiStudioGtkWorkbench *workbench, UmiStudioGtkWorkbenchActivitySnapshot *out_snapshot);

#endif
