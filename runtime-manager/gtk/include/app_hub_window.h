/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/gtk/include/app_hub_window.h
 *
 * PURPOSE:
 *   Expose the GTK4 Application Hub window factory.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#ifndef UMICOM_STUDIO_APP_HUB_WINDOW_H
#define UMICOM_STUDIO_APP_HUB_WINDOW_H

#include <gtk/gtk.h>

GtkWidget *umi_studio_app_hub_window_new(GtkApplication *application);

#endif
