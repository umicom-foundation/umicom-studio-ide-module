/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_selected_code_proposal_gtk4.c
 * PURPOSE: Exercise the actual Studio real selected-code proposal action, reviewed application and retained-button lifetime.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/provider_connections/gtk4.h"
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

/* Use the production proposal window and document transaction. No provider,
 * credential store or user project is opened by this integration fixture. */
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
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *name = argv[1];
    const char *cases[] = {"apply", "stale", "cancel", "retained", "empty", "parent-close"};
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
    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;
    directory = g_dir_make_tmp("umicom-selected-proposal-XXXXXX", &error);
    CHECK(directory != NULL && g_chdir(directory) == 0);
    changed_directory = true;
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
    application = gtk_application_new("org.umicom.studio.selected-proposal-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_AI_CHAT, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.ai.selected-code-proposal");
    CHECK(GTK_IS_BUTTON(button));
    retained = g_object_ref(button);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = "first chosen second";
    view.cursor_offset = 6U;
    view.selection_length = strcmp(name, "empty") == 0 ? 0U : 6U;
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    if (strcmp(name, "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
    }
    else
        g_signal_emit_by_name(button, "clicked");
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
    {
        GtkWindow *candidate = g_list_model_get_item(windows, i);
        if (Find(GTK_WIDGET(candidate), "document.proposal.review") != NULL)
        {
            dialog = candidate;
            break;
        }
        g_object_unref(candidate);
    }
    if (strcmp(name, "empty") == 0 || strcmp(name, "retained") == 0)
    {
        CHECK(dialog == NULL);
        goto inspect;
    }
    CHECK(dialog != NULL);
    GtkWidget *input = Find(GTK_WIDGET(dialog), "document.proposal.input");
    GtkWidget *preview = Find(GTK_WIDGET(dialog), "document.proposal.preview");
    GtkWidget *approve = Find(GTK_WIDGET(dialog), "document.proposal.approve");
    GtkWidget *apply = Find(GTK_WIDGET(dialog), "document.proposal.apply");
    GtkWidget *cancel = Find(GTK_WIDGET(dialog), "document.proposal.cancel");
    CHECK(input != NULL && preview != NULL && approve != NULL && apply != NULL && cancel != NULL);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(input)), "replacement", -1);
    g_signal_emit_by_name(preview, "clicked");
    CHECK(gtk_widget_get_sensitive(apply));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(approve), TRUE);
    if (strcmp(name, "cancel") == 0)
        g_signal_emit_by_name(cancel, "clicked");
    else if (strcmp(name, "parent-close") == 0)
    {
        g_object_ref(apply);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(apply, "clicked");
        g_object_unref(apply);
    }
    else
    {
        if (strcmp(name, "stale") == 0)
            CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "first edited second", 19U) ==
                  UMI_STATUS_OK);
        g_signal_emit_by_name(apply, "clicked");
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    const char *expected = strcmp(name, "apply") == 0   ? "first replacement second"
                           : strcmp(name, "stale") == 0 ? "first edited second"
                                                        : text;
    CHECK(output_bytes == strlen(expected) && strcmp(output, expected) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    if (dialog != NULL)
        gtk_window_destroy(dialog);
    g_clear_object(&dialog);
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
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
    g_free(old_data);
    g_free(directory);
    g_free(original);
    return failed;
}
