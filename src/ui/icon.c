/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/ui/icon.c
 *
 * PURPOSE:
 *   Render the Framework-owned SVG icon in Studio widgets while preserving the
 *   established icon helper API used by older parts of the application.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "icon.h"

/* The configured path is written and read only on GTK's main thread. Keeping
 * one owned copy avoids hard-coded repository paths in individual widgets. */
static char *g_brand_icon_path = NULL;

/* This transparent pixel preserves the older byte-oriented API. It is not a
 * brand source and is never selected by the current Studio interface. */
static const unsigned char COMPATIBILITY_PNG[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
    0x89, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41,
    0x54, 0x78, 0x9C, 0x63, 0x60, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0xE5, 0x27, 0xD4, 0xA2, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
    0x42, 0x60, 0x82
};

gboolean umi_icon_set_brand_path(const char *image_path)
{
    char *canonical_path;

    /* Reject an absent file early so every widget either receives valid
     * artwork or follows the harmless empty-picture fallback. */
    if (image_path == NULL || image_path[0] == '\0' ||
        !g_file_test(image_path, G_FILE_TEST_IS_REGULAR)) {
        return FALSE;
    }
    canonical_path = g_canonicalize_filename(image_path, NULL);
    if (canonical_path == NULL) return FALSE;

    g_free(g_brand_icon_path);
    g_brand_icon_path = canonical_path;
    return TRUE;
}

const char *umi_icon_brand_path(void)
{
    return g_brand_icon_path;
}

void umi_icon_clear_brand_path(void)
{
    g_clear_pointer(&g_brand_icon_path, g_free);
}

GtkWidget *umi_icon_image_logo(int size_px)
{
    GtkWidget *picture;
    int requested_size = size_px > 0 ? size_px : 64;

    /* GtkPicture delegates SVG rendering to the installed GTK image loader,
     * which keeps the mark crisp on normal and high-density displays. */
    picture = g_brand_icon_path != NULL
        ? gtk_picture_new_for_filename(g_brand_icon_path)
        : gtk_picture_new();
    if (picture == NULL) return NULL;

    gtk_widget_set_size_request(picture, requested_size, requested_size);
    gtk_picture_set_can_shrink(GTK_PICTURE(picture), TRUE);
    gtk_picture_set_content_fit(
        GTK_PICTURE(picture), GTK_CONTENT_FIT_CONTAIN);
    return picture;
}

const unsigned char *umi_icon_logo_png_data(size_t *out_length)
{
    /* The static byte array has process lifetime and must not be freed by the
     * compatibility caller. */
    if (out_length != NULL) *out_length = sizeof(COMPATIBILITY_PNG);
    return COMPATIBILITY_PNG;
}

GdkTexture *umi_icon_get_logo_texture(void)
{
    GBytes *bytes;
    GError *error = NULL;
    GdkTexture *texture;

    /* This decoder exists only for callers that still require GdkTexture.
     * Current interface code uses GtkPicture so SVG remains scalable. */
    bytes = g_bytes_new_static(COMPATIBILITY_PNG, sizeof(COMPATIBILITY_PNG));
    if (bytes == NULL) return NULL;
    texture = gdk_texture_new_from_bytes(bytes, &error);
    g_bytes_unref(bytes);
    if (error != NULL) {
        g_warning("Unable to decode the compatibility icon: %s",
                  error->message);
        g_error_free(error);
    }
    return texture;
}

void umi_icon_apply_to_window(GtkWindow *window)
{
    /* GTK4 reads the native icon embedded by the shared CMake branding helper.
     * Keeping this function avoids breaking older callers during migration. */
    (void)window;
}

void umi_icon_try_apply_headerbar_logo(GtkWindow *window, int desired_px)
{
    GtkWidget *titlebar;
    GtkWidget *old_title;
    GtkWidget *row;
    GtkWidget *picture;

    if (window == NULL) return;
    titlebar = gtk_window_get_titlebar(window);
    if (!GTK_IS_HEADER_BAR(titlebar)) return;

    picture = umi_icon_image_logo(desired_px > 0 ? desired_px : 16);
    if (picture == NULL) return;
    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    if (row == NULL) {
        g_object_unref(picture);
        return;
    }
    gtk_widget_set_hexpand(row, TRUE);
    gtk_box_append(GTK_BOX(row), picture);

    /* Preserve an existing title widget while moving it into the branded row.
     * The temporary reference keeps it alive between the two containers. */
    old_title = gtk_header_bar_get_title_widget(GTK_HEADER_BAR(titlebar));
    if (old_title != NULL) {
        g_object_ref(old_title);
        gtk_header_bar_set_title_widget(GTK_HEADER_BAR(titlebar), NULL);
        gtk_box_append(GTK_BOX(row), old_title);
        g_object_unref(old_title);
    }
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(titlebar), row);
}

void umi_icon_try_apply_taskbar_icon_win32(GtkWindow *window)
{
    /* Windows chooses the multi-size ICO compiled into the executable. */
    (void)window;
}
