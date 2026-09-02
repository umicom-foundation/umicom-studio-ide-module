/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/include/status_strip.h
 *
 * PURPOSE:
 *   Declare a GTK4 status strip rendered from Framework status-model items.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_GTK_STATUS_STRIP_H
#define UMICOM_STUDIO_GTK_STATUS_STRIP_H

#include <gtk/gtk.h>
#include "umicom/studio/ui.h"

/**
 * Provide the studio status strip new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_status_strip_new(UmiStudioUi *ui);

#endif
