/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/web/include/frontend_preview.h
 *
 * PURPOSE:
 *   Declare a GTK4 preview of the semantic multi-frontend plan.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The widget displays plan metadata rather than becoming the source of truth for pages, routes or assets.
 */

#ifndef UMICOM_STUDIO_GUI_FRONTEND_PREVIEW_H
#define UMICOM_STUDIO_GUI_FRONTEND_PREVIEW_H
#include <gtk/gtk.h>
#include "umicom/studio/web_platform.h"
/**
 * Provide the studio frontend preview new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_frontend_preview_new(UmiStudioWebPlatform *platform);
#endif
