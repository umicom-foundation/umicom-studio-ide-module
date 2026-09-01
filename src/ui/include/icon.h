/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/ui/include/icon.h
 *
 * PURPOSE:
 *   Provide the small Studio adapter that presents the Framework-owned Umicom
 *   vector icon in GTK widgets. The application supplies a packaged asset
 *   path during startup; callers do not need to know where branding is stored.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_IDE_ICON_H
#define UMICOM_STUDIO_IDE_ICON_H

#include <gtk/gtk.h>
#include <stddef.h>

/* Store the canonical SVG path used by subsequently created Studio widgets.
 * Call this on the GTK main thread after locating the packaged branding
 * directory. The function returns FALSE when the file cannot be used. */
gboolean umi_icon_set_brand_path(const char *image_path);

/* Return the current read-only path, or NULL before branding is configured.
 * The returned pointer belongs to this module and must not be freed. */
const char *umi_icon_brand_path(void);

/* Release the stored path during application shutdown. Existing GtkPicture
 * widgets keep their own loaded image and remain independent of this value. */
void umi_icon_clear_brand_path(void);

/* Build a responsive picture from the configured SVG. An empty picture is
 * returned when no asset is available, so missing optional artwork never
 * prevents the IDE from opening. */
GtkWidget *umi_icon_image_logo(int size_px);

/* Add the configured mark to a native GtkHeaderBar when the window uses one.
 * Studio's custom application bar may call umi_icon_image_logo directly. */
void umi_icon_try_apply_headerbar_logo(GtkWindow *window, int desired_px);

/* GTK4 obtains its operating-system icon from the executable resource. These
 * no-op entry points remain available for source compatibility. */
void umi_icon_apply_to_window(GtkWindow *window);
void umi_icon_try_apply_taskbar_icon_win32(GtkWindow *window);

/* These raster APIs are retained for older Studio callers. They return a tiny
 * transparent compatibility image, never the canonical Umicom artwork. New
 * code should use umi_icon_image_logo and the packaged SVG path. */
const unsigned char *umi_icon_logo_png_data(size_t *out_length);
GdkTexture *umi_icon_get_logo_texture(void);

#endif /* UMICOM_STUDIO_IDE_ICON_H */
