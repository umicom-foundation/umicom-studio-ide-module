/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_startup_failure_gtk4.c
 * PURPOSE: Preserve failed startup status, diagnostic evidence and partial-owner cleanup.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#define CHECK(value) do { if (!(value)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #value); failed = 1; goto cleanup; \
} } while (0)

typedef struct StartupProbe {
    unsigned windows, diagnostics;
    UmiDiagnosticSeverity severity;
    char message[256];
} StartupProbe;

/* A competing titlebar is a real, deterministic constructor precondition
 * failure. The callback runs after GtkApplication has accepted its window;
 * no global allocation hook or application implementation is replaced. */
static void occupy_titlebar(GtkApplication *application, GtkWindow *window, gpointer data)
{
    (void)application;
    StartupProbe *probe = data;
    ++probe->windows;
    gtk_window_set_titlebar(window, gtk_label_new("Existing host titlebar"));
}

static void capture_failure(const UmiDiagnostic *event, void *data)
{
    StartupProbe *probe = data;
    if (strcmp(event->source, "studio-startup") != 0) return;
    ++probe->diagnostics;
    probe->severity = event->severity;
    (void)snprintf(probe->message, sizeof probe->message, "%s", event->message);
}

int main(void)
{
    int failed = 0, changed = 0, isolated = 0, observing = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServices *services = NULL;
    UmiStudioServicesOptions services_options = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    StartupProbe probe = {0};
    GError *error = NULL;
    char *original = NULL, *directory = NULL, *original_data = NULL;
    gulong window_observer = 0U;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    original = g_get_current_dir();
    original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_build_filename(original, "startup-fixture-XXXXXX", NULL);
    CHECK(directory != NULL && g_mkdtemp(directory) != NULL && g_chdir(directory) == 0);
    changed = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    services = umi_studio_bootstrap_services(bootstrap);
    CHECK(umi_studio_services_add_diagnostic_sink(services, capture_failure, &probe) == UMI_STATUS_OK);
    observing = 1;
    application = gtk_application_new("org.umicom.studio.startup-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    guint windows_before = g_list_model_get_n_items(gtk_window_get_toplevels());
    window_observer = g_signal_connect(application, "window-added", G_CALLBACK(occupy_titlebar), &probe);
    UmiStatus status = umi_studio_gtk_workbench_create_with_options(application,
        umi_studio_bootstrap_ui(bootstrap), umi_studio_bootstrap_desktop_shell(bootstrap),
        &options, &workbench);
    CHECK(status == UMI_STATUS_INVALID_STATE && workbench == NULL);
    CHECK(probe.windows == 1U && probe.diagnostics == 1U);
    CHECK(probe.severity == UMI_DIAGNOSTIC_ERROR);
    CHECK(strcmp(probe.message, "Operation window.titlebar failed: Invalid state (2).") == 0);
    CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == windows_before);
cleanup:
    if (window_observer != 0U) g_signal_handler_disconnect(application, window_observer);
    if (observing) (void)umi_studio_services_remove_diagnostic_sink(services, capture_failure, &probe);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed && g_chdir(original) != 0) failed = 1;
    if (isolated) {
        if (original_data != NULL) {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data, TRUE)) failed = 1;
        } else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    g_clear_error(&error);
    g_free(original_data);
    g_free(original);
    g_free(directory);
    return failed;
}
