/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/designer/include/tree_view.h
 *
 * PURPOSE:
 *   Declare the GTK4 component-tree view for the active semantic designer document.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is a presentation adapter.  GTK widgets are created here, while
 * authoritative application/designer state remains in the shared Framework.
 */

#ifndef UMICOM_STUDIO_DESIGNER_TREE_VIEW_H
#define UMICOM_STUDIO_DESIGNER_TREE_VIEW_H
#include <gtk/gtk.h>
#include "umicom/studio/designer.h"
/**
 * Provide the studio designer tree view new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_designer_tree_view_new(UmiStudioDesigner *designer);
#endif
