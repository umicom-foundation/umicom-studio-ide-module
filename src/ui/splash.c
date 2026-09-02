/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/ui/splash.c
 *
 * PURPOSE:
 *   Present a small, accessible startup window while Studio creates Framework
 *   services. The splash exposes progress and branding without depending on
 *   Studio business logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "splash.h"

#include "icon.h"

struct UmiSplash {
    GtkWidget *window;
    GtkWidget *brand_picture;
    GtkWidget *status_label;
    GtkWidget *progress_bar;
    GtkCssProvider *style_provider;
    GdkDisplay *display;
    guint auto_close_id;
};

static const char *SPLASH_CSS =
    ".umicom-studio-splash {"
    "  background: linear-gradient(135deg,#121c2a,#0b1220);"
    "  color: #f4f7fb; border: 1px solid #41556d;"
    "}"
    ".umicom-studio-splash-brand { padding: 24px 28px 18px 28px; }"
    ".umicom-studio-splash-title {"
    "  font-size: 30px; font-weight: 700; letter-spacing: 0.01em;"
    "}"
    ".umicom-studio-splash-edition { color: #a9b6c6; }"
    ".umicom-studio-splash-content {"
    "  padding: 20px 28px 24px 28px; border-top: 1px solid #2b3a4d;"
    "}"
    ".umicom-studio-splash-status { color: #f4f7fb; }"
    ".umicom-studio-splash progressbar trough {"
    "  min-height: 3px; background: #223247;"
    "}"
    ".umicom-studio-splash progressbar progress { background: #4c8ed9; }";

/* Provide the clamp progress operation used by this module and its client applications. */
static double clamp_progress(double value)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (value < 0.0) return 0.0;
    /* Apply this branch only when its contract condition is satisfied. */
    if (value > 1.0) return 1.0;
    return value;
}

/* Provide the auto close operation used by this module and its client applications. */
static gboolean auto_close(gpointer user_data)
{
    UmiSplash *splash = (UmiSplash *)user_data;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL) return G_SOURCE_REMOVE;
    splash->auto_close_id = 0U;
    umi_splash_close(splash);
    return G_SOURCE_REMOVE;
}

/* Provide the make brand area operation used by this module and its client applications. */
static GtkWidget *make_brand_area(UmiSplash *splash, const char *title)
{
    GtkWidget *area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *brand_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    GtkWidget *text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *heading = gtk_label_new(
        title != NULL && title[0] != '\0' ? title : "Umicom Studio IDE");
    GtkWidget *edition = gtk_label_new(
        "Umicom Framework • C23 • GTK4 • Community Edition");

    splash->brand_picture = gtk_picture_new();
    /* The splash uses the symbol beside real text. Text remains sharp,
     * accessible and easy to localise at every display scale. */
    gtk_widget_set_size_request(splash->brand_picture, 72, 72);
    gtk_picture_set_content_fit(
        GTK_PICTURE(splash->brand_picture), GTK_CONTENT_FIT_CONTAIN);
    gtk_widget_add_css_class(heading, "umicom-studio-splash-title");
    gtk_widget_add_css_class(edition, "umicom-studio-splash-edition");
    gtk_label_set_xalign(GTK_LABEL(heading), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(edition), 0.0F);
    gtk_widget_set_valign(text, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(text), heading);
    gtk_box_append(GTK_BOX(text), edition);
    gtk_box_append(GTK_BOX(brand_row), splash->brand_picture);
    gtk_box_append(GTK_BOX(brand_row), text);
    gtk_box_append(GTK_BOX(area), brand_row);
    gtk_widget_add_css_class(area, "umicom-studio-splash-brand");
    return area;
}

/*
 * Provide the make progress area operation used by this module and its client
 * applications.
 */
static GtkWidget *make_progress_area(UmiSplash *splash, const char *subtitle)
{
    GtkWidget *area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *detail = gtk_label_new(
        "Loading Framework contracts, developer services and workspace state");

    splash->status_label = gtk_label_new(
        subtitle != NULL && subtitle[0] != '\0'
            ? subtitle
            : "Starting Umicom Studio…");
    splash->progress_bar = gtk_progress_bar_new();
    gtk_label_set_xalign(GTK_LABEL(splash->status_label), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(detail), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(detail), TRUE);
    gtk_widget_add_css_class(
        splash->status_label, "umicom-studio-splash-status");
    gtk_widget_add_css_class(detail, "umicom-studio-splash-edition");
    gtk_box_append(GTK_BOX(area), splash->status_label);
    gtk_box_append(GTK_BOX(area), splash->progress_bar);
    gtk_box_append(GTK_BOX(area), detail);
    gtk_widget_add_css_class(area, "umicom-studio-splash-content");
    return area;
}

/* Provide the splash png operation used by this module and its client applications. */
const unsigned char *umi_splash_png(size_t *out_size)
{
    /* Older callers may still request raster bytes. Keep their ABI stable,
     * but use no raster image as the primary Studio identity. */
    return umi_icon_logo_png_data(out_size);
}

/* Provide the splash new operation used by this module and its client applications. */
UmiSplash *umi_splash_new(
    const char *title,
    const char *subtitle,
    unsigned int auto_close_ms)
{
    UmiSplash *splash = g_new0(UmiSplash, 1U);
    GtkWidget *root;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL) return NULL;

    splash->window = gtk_window_new();
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash->window == NULL || root == NULL) {
        umi_splash_free(splash);
        return NULL;
    }

    g_object_ref_sink(splash->window);
    gtk_window_set_title(GTK_WINDOW(splash->window), "Umicom Studio IDE");
    gtk_window_set_decorated(GTK_WINDOW(splash->window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(splash->window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(splash->window), 620, 300);
    gtk_widget_add_css_class(splash->window, "umicom-studio-splash");
    {
        GtkWidget *brand_area = make_brand_area(splash, title);
        GtkWidget *progress_area = make_progress_area(splash, subtitle);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (brand_area == NULL || progress_area == NULL) {
            umi_splash_free(splash);
            return NULL;
        }
        gtk_box_append(GTK_BOX(root), brand_area);
        gtk_box_append(GTK_BOX(root), progress_area);
    }
    gtk_window_set_child(GTK_WINDOW(splash->window), root);

    splash->style_provider = gtk_css_provider_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash->style_provider == NULL) {
        umi_splash_free(splash);
        return NULL;
    }
    gtk_css_provider_load_from_string(splash->style_provider, SPLASH_CSS);
    splash->display = gtk_widget_get_display(splash->window);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash->display != NULL) {
        gtk_style_context_add_provider_for_display(
            splash->display,
            GTK_STYLE_PROVIDER(splash->style_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    umi_splash_set_progress(splash, 0.05, subtitle);
    /* Apply this branch only when its contract condition is satisfied. */
    if (auto_close_ms > 0U) {
        splash->auto_close_id = g_timeout_add(
            auto_close_ms, auto_close, splash);
    }
    return splash;
}

/*
 * Provide the splash set brand image operation used by this module and its client
 * applications.
 */
void umi_splash_set_brand_image(UmiSplash *splash, const char *image_path)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL || splash->brand_picture == NULL ||
        image_path == NULL || image_path[0] == '\0') {
        return;
    }
    /* GtkPicture asks the installed image loader to render the file. This is
     * the SVG-aware path used by Studio, while still accepting a raster file
     * from an older extension that calls the same API. */
    if (g_file_test(image_path, G_FILE_TEST_IS_REGULAR)) {
        gtk_picture_set_filename(
            GTK_PICTURE(splash->brand_picture), image_path);
    }
}

/* Provide the splash show operation used by this module and its client applications. */
void umi_splash_show(UmiSplash *splash, GtkWindow *parent)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL || splash->window == NULL) return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (parent != NULL) {
        gtk_window_set_transient_for(GTK_WINDOW(splash->window), parent);
    }
    gtk_window_present(GTK_WINDOW(splash->window));
}

/*
 * Provide the splash set progress operation used by this module and its client
 * applications.
 */
void umi_splash_set_progress(
    UmiSplash *splash,
    double fraction,
    const char *message)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL || splash->progress_bar == NULL) return;
    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(splash->progress_bar), clamp_progress(fraction));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message != NULL && message[0] != '\0') {
        gtk_label_set_text(GTK_LABEL(splash->status_label), message);
    }
}

/* Provide the splash close operation used by this module and its client applications. */
void umi_splash_close(UmiSplash *splash)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash != NULL && splash->window != NULL) {
        gtk_widget_set_visible(splash->window, FALSE);
    }
}

/* Provide the splash free operation used by this module and its client applications. */
void umi_splash_free(UmiSplash *splash)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL) return;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (splash->auto_close_id != 0U) {
        g_source_remove(splash->auto_close_id);
        splash->auto_close_id = 0U;
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash->display != NULL && splash->style_provider != NULL) {
        gtk_style_context_remove_provider_for_display(
            splash->display, GTK_STYLE_PROVIDER(splash->style_provider));
    }
    g_clear_object(&splash->style_provider);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash->window != NULL) {
        gtk_window_destroy(GTK_WINDOW(splash->window));
        g_object_unref(splash->window);
        splash->window = NULL;
    }
    g_free(splash);
}

/* Provide the splash window operation used by this module and its client applications. */
GtkWindow *umi_splash_window(UmiSplash *splash)
{
    return splash != NULL ? GTK_WINDOW(splash->window) : NULL;
}

/* Provide the uside splash show operation used by this module and its client applications. */
GtkWidget *uside_splash_show(GtkApplication *application, guint auto_close_ms)
{
    UmiSplash *splash = umi_splash_new(
        "Umicom Studio IDE", "Preparing your workspace…", auto_close_ms);
    GtkWidget *window;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL) return NULL;
    window = GTK_WIDGET(umi_splash_window(splash));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (application != NULL) {
        gtk_window_set_application(GTK_WINDOW(window), application);
    }
    g_object_set_data(G_OBJECT(window), "umicom-splash-handle", splash);
    umi_splash_show(splash, NULL);
    return window;
}

/* Provide the legacy close operation used by this module and its client applications. */
static gboolean legacy_close(gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    /* Apply this branch only when its contract condition is satisfied. */
    if (GTK_IS_WINDOW(window)) {
        UmiSplash *splash = (UmiSplash *)g_object_get_data(
            G_OBJECT(window), "umicom-splash-handle");
        g_object_set_data(G_OBJECT(window), "umicom-splash-handle", NULL);
        umi_splash_free(splash);
    }
    return G_SOURCE_REMOVE;
}

/*
 * Provide the uside splash close later operation used by this module and its client
 * applications.
 */
void uside_splash_close_later(GtkWidget *splash, guint grace_ms)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!GTK_IS_WINDOW(splash)) return;
    /* Apply this branch only when its contract condition is satisfied. */
    if (grace_ms == 0U) {
        (void)legacy_close(splash);
        return;
    }
    g_timeout_add_full(
        G_PRIORITY_DEFAULT,
        grace_ms,
        legacy_close,
        g_object_ref(splash),
        (GDestroyNotify)g_object_unref);
}
