/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/dialogs/include/about.h
 *
 * PURPOSE:
 *   Compose actual Studio identity for shared native About presentation while retaining legacy entry points.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_ABOUT_H
#define UMICOM_ABOUT_H
#include <gtk/gtk.h>
#include "umicom/studio/version.h"
#include "umicom/ui/gtk4/workstation/about_dialog.h"

/* The original public helpers date from 2025-10-13. They remain non-blocking
 * compatibility entry points. No GTK dialog machinery lives in this product.
 * Keeping identity composition inline also serves the lightweight workbench
 * demo without introducing a dependency on the full StudioProduct library. */
static inline UmiStatus UmiStudioAboutPresent(GtkWindow *parent)
{
    const UmiGtk4AboutDialogConfig config = {
        {"Umicom Studio IDE", "Umicom Foundation", "umicom-studio"},
        UMICOM_STUDIO_VERSION_STRING,
        "Open-source IDE for Umicom projects.",
        GTK_LICENSE_MIT_X11
    };
    return UmiGtk4AboutDialogPresent(parent, &config);
}
/** Present the product's non-blocking native About window. */
void umi_about_show(GtkWindow *parent);
/** Legacy alias retained for existing callers. */
void umi_show_about(GtkWindow *parent);
#endif
