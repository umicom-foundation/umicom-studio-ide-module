/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/gtk/include/editor_workbench_window.h
 *
 * PURPOSE:
 *   Expose the GTK4/GtkSourceView productivity workbench.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#ifndef UMICOM_STUDIO_EDITOR_WORKBENCH_WINDOW_H
#define UMICOM_STUDIO_EDITOR_WORKBENCH_WINDOW_H

#include <gtk/gtk.h>

/**
 * Provide the studio editor workbench window new operation used by this module and its
 * client applications.
 */
GtkWidget *umi_studio_editor_workbench_window_new(GtkApplication *application);

#endif
