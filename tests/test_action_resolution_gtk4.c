/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_action_resolution_gtk4.c
 * PURPOSE: Exercise Studio editor code-actions discovery, explicit launch boundary and retained controls.
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

/* Use the production code-actions window and document transaction. No provider,
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
    const char *cases[] = {"open", "cancel", "retained", "empty", "parent-close"};
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
    directory = g_dir_make_tmp("umicom-source-formatting-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.source-formatting-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *button = Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.actions");
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
        if (Find(GTK_WIDGET(candidate), "document.completion.review") != NULL)
        {
            dialog = candidate;
            break;
        }
        g_object_unref(candidate);
    }

    if (strcmp(name, "retained") == 0)
    {
        CHECK(dialog == NULL);
        goto inspect;
    }
    CHECK(dialog != NULL);
    /* Studio delegates preparation and approval to the shared form. Opting in
     * configures a future request and does not start a server or modify text. */
    GtkWidget *resolve = Find(GTK_WIDGET(dialog), "document.code-actions.resolve");
    CHECK(GTK_IS_CHECK_BUTTON(resolve) && !gtk_check_button_get_active(GTK_CHECK_BUTTON(resolve)));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(resolve), TRUE);
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);

    /* Selecting a requested action family configures the capture only. The
     * user must still request, inspect and approve a complete proposal. */
    GtkWidget *family = Find(GTK_WIDGET(dialog), "document.code-actions.kind");
    CHECK(GTK_IS_DROP_DOWN(family) && gtk_drop_down_get_selected(GTK_DROP_DOWN(family)) == 0U);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(family), 3U);
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);

    GtkWidget *pull_context = Find(GTK_WIDGET(dialog), "document.code-actions.pull");
    CHECK(GTK_IS_CHECK_BUTTON(pull_context) && !gtk_check_button_get_active(GTK_CHECK_BUTTON(pull_context)));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(pull_context), TRUE);
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(pull_context), FALSE);

    /* Scope is an explicit choice; changing it must neither start a process
     * nor acquire permission to edit the current document. */
    GtkWidget *include_open = Find(GTK_WIDGET(dialog), "document.code-actions.open-drafts");
    CHECK(GTK_IS_CHECK_BUTTON(include_open));
    CHECK(!gtk_check_button_get_active(GTK_CHECK_BUTTON(include_open)));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(include_open), TRUE);
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(include_open), FALSE);

    /* Source information uses a read-only form; formatter assertions are
     * retained below to make the corrected fixture expectation reviewable. */
#if 0
    CHECK(strcmp(gtk_window_get_title(dialog), "Review document formatting") == 0);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.tab-size") != NULL);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.insert-spaces") != NULL);
#endif
    CHECK(strcmp(gtk_window_get_title(dialog), "Review code actions") == 0);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.tab-size") == NULL);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.insert-spaces") == NULL);
    GtkWidget *program = Find(GTK_WIDGET(dialog), "document.completion.program");
    GtkWidget *request = Find(GTK_WIDGET(dialog), "document.completion.request");
    GtkWidget *apply = Find(GTK_WIDGET(dialog), "document.completion.apply");
    GtkWidget *cancel = Find(GTK_WIDGET(dialog), "document.completion.cancel");
    CHECK(program != NULL && request != NULL && apply != NULL && cancel != NULL);
    CHECK(gtk_editable_get_text(GTK_EDITABLE(program))[0] == '\0');
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    CHECK(!gtk_widget_get_sensitive(apply) && gtk_window_get_default_widget(dialog) == cancel);
    CHECK(gtk_widget_get_visible(apply));
    CHECK(Find(GTK_WIDGET(dialog), "document.rename.name") == NULL);
    CHECK(gtk_widget_get_visible(Find(GTK_WIDGET(dialog), "document.completion.choice")));
    if (strcmp(name, "cancel") == 0)
        g_signal_emit_by_name(cancel, "clicked");
    if (strcmp(name, "parent-close") == 0)
    {
        g_object_ref(request);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(request, "clicked");
        CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
        g_object_unref(request);
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
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
