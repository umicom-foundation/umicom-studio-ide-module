/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/editor/include/editor_host_view.h
 *
 * PURPOSE:
 *   Declare a GTK4 editor-host projection backed by the Framework document
 *   store and editor presentation model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_GTK_EDITOR_HOST_VIEW_H
#define UMICOM_STUDIO_GTK_EDITOR_HOST_VIEW_H

#include <gtk/gtk.h>
#include "umicom/studio/ui.h"

/**
 * Provide the studio editor host view new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_editor_host_view_new(UmiStudioUi *ui);

#endif
