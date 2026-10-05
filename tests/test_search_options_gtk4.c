/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_search_options_gtk4.c
 * PURPOSE: Exercise explicit search policy through Studio editing controls and captured replacement reviews.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
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

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *name = argv[1],
               *cases[] = {"defaults",   "sensitive", "insensitive",      "smart-uppercase",
                           "whole-word", "previous",  "single-boundary",  "single-insensitive",
                           "all-review", "all-owned", "session-review",   "session-owned",
                           "set-review", "set-owned", "retained-options", "read-only-find"};
    int known = 0;
    for (size_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(name, cases[i]) == 0)
            known = 1;
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
    GtkWindow *dialog = NULL;
    GtkWidget *retained = NULL;
    GError *error = NULL;
    char *output = NULL;
    size_t output_bytes = 0U;
    directory = g_dir_make_tmp("umicom-search-options-XXXXXX", &error);
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
    CHECK(umi_document_coordinator_new(documents, "Search policy.c", id, sizeof(id)) == UMI_STATUS_OK);
    const char *source = "foobar foo FOO foo_ foo2 foo", *expectedText = source;
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    view.cursor_offset = 0U;
    view.selection_length = 0U;
    if (strcmp(name, "previous") == 0)
        view.cursor_offset = 25U;
    if (strcmp(name, "single-boundary") == 0)
        view.selection_length = 3U;
    if (strcmp(name, "single-insensitive") == 0)
    {
        view.cursor_offset = 11U;
        view.selection_length = 3U;
    }
    if (strcmp(name, "read-only-find") == 0)
        view.read_only = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.search-options-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *query = Find(root, "studio.editor.find-text"),
              *replacement = Find(root, "studio.editor.replace-text"),
              *mode = Find(root, "studio.editor.search-case"),
              *word = Find(root, "studio.editor.search-whole-word");
    CHECK(query != NULL && replacement != NULL && GTK_IS_DROP_DOWN(mode) && GTK_IS_CHECK_BUTTON(word));
    CHECK(gtk_drop_down_get_selected(GTK_DROP_DOWN(mode)) == 0U &&
          !gtk_check_button_get_active(GTK_CHECK_BUTTON(word)));
    gtk_editable_set_text(GTK_EDITABLE(query), "foo");
    gtk_editable_set_text(GTK_EDITABLE(replacement), "bar");
    int plain = strcmp(name, "defaults") == 0 || strcmp(name, "sensitive") == 0 ||
                strcmp(name, "insensitive") == 0 || strcmp(name, "smart-uppercase") == 0;
    if (!plain)
    {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(mode), 1U);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(word), TRUE);
    }
    if (strcmp(name, "sensitive") == 0)
        gtk_drop_down_set_selected(GTK_DROP_DOWN(mode), 1U);
    if (strcmp(name, "insensitive") == 0 || strcmp(name, "single-insensitive") == 0)
        gtk_drop_down_set_selected(GTK_DROP_DOWN(mode), 2U);
    if (strcmp(name, "sensitive") == 0 || strcmp(name, "insensitive") == 0 ||
        strcmp(name, "smart-uppercase") == 0)
        gtk_editable_set_text(GTK_EDITABLE(query), "FOO");
    const char *action = "studio.editor.find-next", *dialogId = NULL;
    if (strcmp(name, "previous") == 0)
        action = "studio.editor.find-previous";
    if (strncmp(name, "single-", 7U) == 0)
    {
        action = "studio.editor.replace-next";
        expectedText = strcmp(name, "single-insensitive") == 0 ? "foobar foo bar foo_ foo2 foo"
                                                               : "foobar bar FOO foo_ foo2 foo";
    }
    if (strncmp(name, "all-", 4U) == 0)
    {
        action = "studio.editor.replace-all";
        dialogId = "studio.confirm.accept";
    }
    if (strncmp(name, "session-", 8U) == 0)
    {
        action = "studio.editor.replace-open-documents";
        dialogId = "document.replacements.review";
    }
    if (strncmp(name, "set-", 4U) == 0)
    {
        action = "studio.editor.replace-complete-set";
        dialogId = "document.replacement-set.review";
    }
    GtkWidget *button = Find(root, action);
    CHECK(GTK_IS_BUTTON(button));
    if (strcmp(name, "retained-options") == 0)
    {
        retained = g_object_ref(mode);
        g_object_ref(button);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        gtk_drop_down_set_selected(GTK_DROP_DOWN(retained), 2U);
        g_signal_emit_by_name(button, "clicked");
        g_object_unref(button);
        Pump();
    }
    else
    {
        g_signal_emit_by_name(button, "clicked");
        if (strstr(name, "owned") != NULL)
        {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(mode), 2U);
            gtk_check_button_set_active(GTK_CHECK_BUTTON(word), FALSE);
        }
        Pump();
        if (dialogId != NULL)
        {
            GListModel *windows = gtk_window_get_toplevels();
            for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
            {
                GtkWindow *candidate = g_list_model_get_item(windows, i);
                if (Find(GTK_WIDGET(candidate), dialogId) != NULL)
                {
                    dialog = candidate;
                    break;
                }
                g_object_unref(candidate);
            }
            CHECK(dialog != NULL);
            expectedText = "foobar bar FOO foo_ foo2 bar";
            if (strncmp(name, "all-", 4U) == 0)
                g_signal_emit_by_name(Find(GTK_WIDGET(dialog), "studio.confirm.accept"), "clicked");
            else
            {
                GtkWidget *policy = Find(GTK_WIDGET(dialog), "document.replacements.search-policy");
                CHECK(GTK_IS_LABEL(policy) &&
                      strcmp(gtk_label_get_text(GTK_LABEL(policy)), "Search: Match case, whole word") == 0);
                if (strncmp(name, "session-", 8U) == 0)
                {
                    GtkWidget *apply = Find(GTK_WIDGET(dialog), "document.replacements.apply");
                    CHECK(gtk_widget_get_sensitive(apply));
                    g_signal_emit_by_name(apply, "clicked");
                }
                else
                {
                    g_signal_emit_by_name(Find(GTK_WIDGET(dialog), "document.replacement-set.reviewed"),
                                          "clicked");
                    gtk_check_button_set_active(
                        GTK_CHECK_BUTTON(Find(GTK_WIDGET(dialog), "document.replacement-set.approval")),
                        TRUE);
                    GtkWidget *apply = Find(GTK_WIDGET(dialog), "document.replacement-set.apply");
                    CHECK(gtk_widget_get_sensitive(apply));
                    g_signal_emit_by_name(apply, "clicked");
                }
            }
            Pump();
        }
        else if (strncmp(name, "single-", 7U) != 0)
        {
            CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
            size_t offset = (strcmp(name, "defaults") == 0 || strcmp(name, "insensitive") == 0)        ? 0U
                            : (strcmp(name, "sensitive") == 0 || strcmp(name, "smart-uppercase") == 0) ? 11U
                                                                                                       : 7U;
            CHECK(view.cursor_offset == offset && view.selection_length == 3U);
        }
    }
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(expectedText) && strcmp(output, expectedText) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    g_clear_object(&retained);
    if (dialog != NULL)
    {
        gtk_window_destroy(dialog);
        g_clear_object(&dialog);
    }
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
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
