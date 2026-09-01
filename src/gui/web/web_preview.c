/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/web/web_preview.c
 *
 * PURPOSE:
 *   Render Framework web-platform status in a small GTK4 preview panel.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * GTK4 formatting lives here; the data comes from the toolkit-neutral
 * UmiStudioWebPlatform snapshot.
 */

#include "web_preview.h"

GtkWidget *umi_studio_web_preview_new(
    UmiStudioWebPlatform *platform)
{
    UmiStudioWebPlatformSnapshot snapshot;
    GtkWidget *box;
    GtkWidget *label;
    const char *browser_text;
    gchar *summary_text;

    if (platform == NULL) {
        return gtk_label_new("Web platform unavailable");
    }

    (void)umi_studio_web_platform_snapshot(
        platform,
        &snapshot);

    browser_text = snapshot.browser_url[0] != '\0'
        ? snapshot.browser_url
        : "not navigated";

    /*
     * browser_url is intentionally much larger than the old 512-byte display
     * buffer. Allocate the exact presentation string rather than truncating a
     * valid URL or suppressing -Wformat-truncation.
     */
    summary_text = g_strdup_printf(
        "Routes: %zu\n"
        "Endpoints: %zu\n"
        "Requests: %" G_GUINT64_FORMAT "\n"
        "Browser: %s",
        snapshot.routes,
        snapshot.endpoints,
        (guint64)snapshot.requests,
        browser_text);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    label = gtk_label_new(
        summary_text != NULL
            ? summary_text
            : "Web platform status unavailable");
    g_free(summary_text);

    gtk_label_set_xalign(GTK_LABEL(label), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_box_append(GTK_BOX(box), label);
    return box;
}
