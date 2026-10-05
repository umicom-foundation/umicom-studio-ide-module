/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_action_groups_gtk4.c
 * PURPOSE: Verify grouped editor actions keep their original identities, behavior and host lifetime.
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
    const char *name = argv[1];
    const char *cases[] = {"discovery",    "code-filter",     "navigation-filter",
                           "clear-filter", "matching-action", "snippet-action",
                           "sensitivity",  "retained-filter", "caption"};
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
    GtkWidget *filter = NULL;
    GtkWindow *dialog = NULL;

    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;

    directory = g_dir_make_tmp("umicom-action-groups-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.action-groups-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.match-delimiter");
    CHECK(GTK_IS_BUTTON(button));
    retained = g_object_ref(button);

    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = "(x)";
    view.cursor_offset = 0U;
    view.selection_length = 0U;
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *code = Find(root, "studio.editor.code-tools"),
              *navigation = Find(root, "studio.editor.navigate-tools");
    CHECK(GTK_IS_MENU_BUTTON(code) && GTK_IS_MENU_BUTTON(navigation));
/* Syntax folding adds one code tool while retaining every earlier action.
 * Keep the former expectation for review. */
#if 0
    CHECK(UmiGtk4ActionMenuVisibleCount(code) == 16U && UmiGtk4ActionMenuVisibleCount(navigation) == 8U);
#endif
/* File-format actions join the existing code tools; retain the earlier
 * count for review while checking the expanded current catalogue. */
#if 0
    CHECK(UmiGtk4ActionMenuVisibleCount(code) == 26U && UmiGtk4ActionMenuVisibleCount(navigation) == 8U);
#endif
    CHECK(UmiGtk4ActionMenuVisibleCount(code) == 32U && UmiGtk4ActionMenuVisibleCount(navigation) == 8U);
    filter = g_object_ref(UmiGtk4ActionMenuFilterEntry(code));
    CHECK(GTK_IS_ENTRY(filter));
    const char *expected[] = {"completion",
                              "formatting",
                              "source-information",
                              "symbols",
                              "parameters",
                              "rename",
                              "actions",
                              "diagnostics",
                              "definition",
                              "references",
                              "type-definition",
                              "implementation",
                              "syntax-selection",
                              "syntax-folding",
                              "callers",
                              "callees",
                              "snippet",
                              "line-delete",
                              "line-duplicate",
                              "line-move-up",
                              "line-move-down",
                              "line-join",
                              "line-trim",
                              "line-indent",
                              "line-outdent",
                              "line-comment",
                              "format-lf",
                              "format-crlf",
                              "format-utf8",
                              "format-utf8-bom",
                              "format-utf16-le",
                              "format-utf16-be",
                              "match-delimiter",
                              "select-delimiter-content",
                              "select-delimiter-pair",
                              "fold-selection",
                              "reveal-folds",
                              "bookmark-toggle",
                              "bookmark-next",
                              "bookmark-previous"};
    for (size_t i = 0U; i < sizeof(expected) / sizeof(expected[0]); ++i)
    {
        char action[128];
        (void)snprintf(action, sizeof(action), "studio.editor.%s", expected[i]);
        GtkWidget *control = Find(root, action);
        CHECK(GTK_IS_BUTTON(control));
/* Syntax folding adds one code tool while retaining every earlier action.
 * Keep the former expectation for review. */
#if 0
        CHECK(gtk_widget_is_ancestor(control, i < 16U ? code : navigation));
#endif
/* File-format actions join the existing code tools; retain the earlier
 * count for review while checking the expanded current catalogue. */
#if 0
        CHECK(gtk_widget_is_ancestor(control, i < 26U ? code : navigation));
#endif
        CHECK(gtk_widget_is_ancestor(control, i < 32U ? code : navigation));
    }
    if (strcmp(name, "code-filter") == 0)
    {
        CHECK(UmiGtk4ActionMenuSetFilter(code, "call hierarchy") == UMI_STATUS_OK);
        Pump();
        CHECK(UmiGtk4ActionMenuVisibleCount(code) == 2U && UmiGtk4ActionMenuVisibleCount(navigation) == 8U);
    }
    if (strcmp(name, "navigation-filter") == 0)
    {
        CHECK(UmiGtk4ActionMenuSetFilter(navigation, "bookmark") == UMI_STATUS_OK);
        Pump();
        CHECK(UmiGtk4ActionMenuVisibleCount(navigation) == 3U);
    }
    if (strcmp(name, "clear-filter") == 0)
    {
        CHECK(UmiGtk4ActionMenuSetFilter(code, "unmatched input") == UMI_STATUS_OK);
        Pump();
        CHECK(UmiGtk4ActionMenuVisibleCount(code) == 0U);
        CHECK(UmiGtk4ActionMenuSetFilter(code, "") == UMI_STATUS_OK);
        Pump();
/* Syntax folding adds one code tool while retaining every earlier action.
 * Keep the former expectation for review. */
#if 0
        CHECK(UmiGtk4ActionMenuVisibleCount(code) == 16U);
#endif
/* File-format actions join the existing code tools; retain the earlier
 * count for review while checking the expanded current catalogue. */
#if 0
        CHECK(UmiGtk4ActionMenuVisibleCount(code) == 26U);
#endif
        CHECK(UmiGtk4ActionMenuVisibleCount(code) == 32U);
    }
    if (strcmp(name, "matching-action") == 0)
    {
        g_signal_emit_by_name(retained, "clicked");
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK && view.cursor_offset == 2U);
    }
    if (strcmp(name, "snippet-action") == 0)
    {
        g_signal_emit_by_name(Find(root, "studio.editor.snippet"), "clicked");
        GListModel *windows = gtk_window_get_toplevels();
        for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
        {
            GtkWindow *candidate = g_list_model_get_item(windows, i);
            if (Find(GTK_WIDGET(candidate), "document.snippet.review") != NULL)
            {
                dialog = candidate;
                break;
            }
            g_object_unref(candidate);
        }
        CHECK(dialog != NULL);
    }
    if (strcmp(name, "sensitivity") == 0)
    {
        GtkWidget *completion = Find(root, "studio.editor.completion");
        gtk_widget_set_sensitive(completion, FALSE);
        CHECK(UmiGtk4ActionMenuSetFilter(code, "completion") == UMI_STATUS_OK);
        Pump();
        CHECK(!gtk_widget_get_sensitive(completion) && UmiGtk4ActionMenuVisibleCount(code) == 1U);
    }
    if (strcmp(name, "caption") == 0)
        CHECK(strcmp(gtk_button_get_label(GTK_BUTTON(Find(root, "studio.editor.source-information"))),
                     "Source info\xe2\x80\xa6") == 0);
    if (strcmp(name, "retained-filter") == 0)
    {
        CHECK(UmiGtk4ActionMenuSetFilter(code, "completion") == UMI_STATUS_OK);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        gtk_editable_set_text(GTK_EDITABLE(filter), "late filter");
        g_signal_emit_by_name(retained, "clicked");
        Pump();
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK && view.cursor_offset == 0U);
    }
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);

    if (dialog != NULL)
        gtk_window_destroy(dialog);
    g_clear_object(&dialog);
    g_clear_object(&filter);
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
