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

/**
 * Represent the studio gtk workbench data shared with callers of this public contract.
 */
typedef struct UmiStudioGtkWorkbench UmiStudioGtkWorkbench;

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

#endif
