/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_recovery_gtk4.c
 * PURPOSE: Exercise Studio recovery discovery and restore through the shared native document owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/document/recovery_storage.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/provider_connections/gtk4.h"
#include "umicom/ui/gtk4/text_folding.h"
#include "umicom/ui/gtk4/action_menu.h"
#include "umicom/document/navigation_history.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(c))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "line %d: %s\n", __LINE__, #c);                                                  \
            failed = 1;                                                                                      \
            goto cleanup;                                                                                    \
        }                                                                                                    \
    } while (0)

/* Exercise the production editor toolbar and authoritative draft together.
 * The fixture uses private temporary settings and no provider or project. */
static GtkWidget *Find(GtkWidget *root, const char *id)
{
    if (root == NULL)
        return NULL;
    const char *tag = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (tag != NULL && strcmp(tag, id) == 0)
        return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkWidget *found = Find(child, id);
        if (found != NULL)
            return found;
    }
    return NULL;
}

static void Pump(void)
{
    for (unsigned step = 0U; step < 512U && g_main_context_pending(NULL); ++step)
        (void)g_main_context_iteration(NULL, FALSE);
}

static GtkWindow *RecoveryWindow(void)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
    {
        GtkWindow *window = g_list_model_get_item(windows, i);
        if (Find(GTK_WIDGET(window), "document.recovery.review") != NULL)
            return window;
        g_object_unref(window);
    }
    return NULL;
}
static int WaitRecovery(GtkWindow *window)
{
    gint64 deadline = g_get_monotonic_time() + 20 * G_TIME_SPAN_SECOND;
    while (g_object_get_data(G_OBJECT(window), "umicom-recovery-pending") != NULL &&
           g_get_monotonic_time() < deadline)
    {
        g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    return g_object_get_data(G_OBJECT(window), "umicom-recovery-pending") == NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *name = argv[1];
    const char *cases[] = {"open",         "empty-list",       "capture",
                           "restore",      "unicode",          "retained-control",
                           "parent-close", "automatic-enable", "automatic-pause"};
    bool known = false;
    for (size_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(name, cases[i]) == 0)
            known = true;
    if (!known)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool changed_directory = false;
    char *original = g_get_current_dir(), *directory = NULL,
         *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *retained = NULL;
    GtkWindow *dialog = NULL;
    UmiDocumentRecoveryCatalogue *catalogue = NULL;
    const char *environment_names[] = {"LOCALAPPDATA", "XDG_CONFIG_HOME", "XDG_DATA_HOME", "XDG_STATE_HOME",
                                       "XDG_CACHE_HOME"};
    gchar *environment_values[5] = {NULL, NULL, NULL, NULL, NULL};
    int environment_changed = 0;
    for (size_t i = 0U; i < 5U; ++i)
        environment_values[i] = g_strdup(g_getenv(environment_names[i]));

    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;

    directory = g_dir_make_tmp("umicom-studio-recovery-XXXXXX", &error);
    CHECK(directory != NULL && g_chdir(directory) == 0);
    changed_directory = true;
    environment_changed = 1;
    for (size_t i = 0U; i < 5U; ++i)
        CHECK(g_setenv(environment_names[i], directory, TRUE));
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions services = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char id[UMI_UI_ID_CAPACITY];
    CHECK(umi_document_coordinator_new(documents, "Private filename.c", id, sizeof(id)) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.recovery-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);

    const char *source = strcmp(name, "unicode") == 0 ? "caf\xc3\xa9\n\xe9\x9b\xaa" : "recover this draft";
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    view.cursor_offset = 0U;
    view.selection_length = 0U;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.recovery");
    CHECK(button != NULL);
    g_signal_emit_by_name(button, "clicked");
    dialog = RecoveryWindow();
    CHECK(dialog != NULL);
    char recovery_directory[UMI_PATH_CAPACITY];
    CHECK(UmiDocumentRecoveryDirectory("Studio", NULL, recovery_directory, sizeof(recovery_directory)) ==
              UMI_STATUS_OK &&
          umi_path_is_within(directory, recovery_directory));
    GtkWidget *save = Find(GTK_WIDGET(dialog), "document.recovery.save"),
              *refresh = Find(GTK_WIDGET(dialog), "document.recovery.refresh"),
              *load = Find(GTK_WIDGET(dialog), "document.recovery.load"),
              *restore = Find(GTK_WIDGET(dialog), "document.recovery.restore");
    CHECK(save && refresh && load && restore);
    if (strcmp(name, "automatic-enable") == 0 || strcmp(name, "automatic-pause") == 0)
    {
        GtkWidget *automatic = Find(GTK_WIDGET(dialog), "document.recovery.automatic");
        CHECK(automatic != NULL);
        g_signal_emit_by_name(automatic, "clicked");
        const char *label = gtk_button_get_label(GTK_BUTTON(automatic));
        CHECK(strcmp(label, "Pause automatic snapshots") == 0);
        if (strcmp(name, "automatic-pause") == 0)
        {
            g_signal_emit_by_name(automatic, "clicked");
            CHECK(strcmp(gtk_button_get_label(GTK_BUTTON(automatic)),
                         "Enable automatic snapshots (every minute)") == 0);
        }
        goto cleanup;
    }
    if (strcmp(name, "open") == 0)
    {
        CHECK(UmiDocumentRecoveryList(recovery_directory, NULL, &catalogue) == UMI_STATUS_OK &&
              UmiDocumentRecoveryCatalogueCount(catalogue) == 0U);
        goto cleanup;
    }
    if (strcmp(name, "empty-list") == 0)
    {
        g_signal_emit_by_name(refresh, "clicked");
        CHECK(WaitRecovery(dialog));
        CHECK(!gtk_widget_get_sensitive(restore));
        goto cleanup;
    }
    if (strcmp(name, "retained-control") == 0)
    {
        retained = g_object_ref(save);
        size_t count = umi_document_coordinator_count(documents);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(umi_document_coordinator_count(documents) == count);
        CHECK(UmiDocumentRecoveryList(recovery_directory, NULL, &catalogue) == UMI_STATUS_OK &&
              UmiDocumentRecoveryCatalogueCount(catalogue) == 0U);
        goto cleanup;
    }
    g_signal_emit_by_name(save, "clicked");
    if (strcmp(name, "parent-close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        CHECK(WaitRecovery(dialog));
        goto cleanup;
    }
    CHECK(WaitRecovery(dialog));
    CHECK(UmiDocumentRecoveryList(recovery_directory, NULL, &catalogue) == UMI_STATUS_OK &&
          UmiDocumentRecoveryCatalogueCount(catalogue) == 1U);
    if (strcmp(name, "capture") == 0)
        goto cleanup;
    g_signal_emit_by_name(refresh, "clicked");
    CHECK(WaitRecovery(dialog));
    g_signal_emit_by_name(load, "clicked");
    CHECK(WaitRecovery(dialog));
    CHECK(gtk_widget_get_sensitive(restore));
    size_t count = umi_document_coordinator_count(documents);
    g_signal_emit_by_name(restore, "clicked");
    Pump();
    CHECK(umi_document_coordinator_count(documents) == count + 1U);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK &&
          strcmp(active.view_id, id) != 0 && !active.has_path && active.dirty);
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &output, &output_bytes) == UMI_STATUS_OK &&
          strcmp(output, source) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    UmiDocumentRecoveryCatalogueDestroy(catalogue);
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
    if (dialog != NULL)
    {
        if (!WaitRecovery(dialog))
            failed = 1;
        gtk_window_destroy(dialog);
        g_object_unref(dialog);
    }
    g_clear_object(&retained);
    if (bootstrap != NULL)
        umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL)
        umi_settings_destroy(settings);
    g_clear_object(&application);
    g_clear_error(&error);
    if (changed_directory)
        (void)g_chdir(original);
    if (old_data != NULL)
        (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    for (size_t i = 0U; i < 5U; ++i)
    {
        if (environment_changed)
        {
            if (environment_values[i] != NULL)
                (void)g_setenv(environment_names[i], environment_values[i], TRUE);
            else
                g_unsetenv(environment_names[i]);
        }
        g_free(environment_values[i]);
    }
    g_free(old_data);
    g_free(directory);
    g_free(original);
    return failed;
}
