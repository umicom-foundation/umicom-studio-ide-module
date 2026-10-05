/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_line_edits_gtk4.c
 * PURPOSE: Exercise real Studio line-edit controls against shared document text and Undo.
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
    const char *cases[] = {"duplicate", "delete",    "move-up",          "move-down",  "join",
                           "indent",    "outdent",   "comment",          "trim",       "selection",
                           "read-only", "undo-redo", "retained-control", "no-neighbor"};
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

    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;

    directory = g_dir_make_tmp("umicom-line-edits-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.line-edits-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);

    const char *source = "a\nbb\nccc", *expected = "a\nbb\nbb\nccc", *action = "duplicate";
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 2U;
    view.selection_length = 0U;
    view.dirty = 1;
    if (strcmp(name, "delete") == 0)
    {
        action = "delete";
        expected = "a\nccc";
    }
    if (strcmp(name, "move-up") == 0)
    {
        action = "move-up";
        expected = "bb\na\nccc";
    }
    if (strcmp(name, "move-down") == 0)
    {
        action = "move-down";
        expected = "a\nccc\nbb";
    }
    if (strcmp(name, "join") == 0)
    {
        action = "join";
        expected = "a\nbb ccc";
    }
    if (strcmp(name, "indent") == 0)
    {
        action = "indent";
        expected = "a\n    bb\nccc";
    }
    if (strcmp(name, "outdent") == 0)
    {
        action = "outdent";
        source = "a\n    bb\nccc";
        expected = "a\nbb\nccc";
    }
    if (strcmp(name, "comment") == 0)
    {
        action = "comment";
        expected = "a\n// bb\nccc";
    }
    if (strcmp(name, "trim") == 0)
    {
        action = "trim";
        source = "a \nbb\t\nccc ";
        view.cursor_offset = 3U;
        expected = "a\nbb\nccc";
    }
    if (strcmp(name, "selection") == 0)
    {
        action = "indent";
        view.cursor_offset = 0U;
        view.selection_length = 5U;
        expected = "    a\n    bb\nccc";
    }
    if (strcmp(name, "read-only") == 0)
    {
        view.read_only = 1;
        expected = source;
    }
    if (strcmp(name, "no-neighbor") == 0)
    {
        action = "move-up";
        view.cursor_offset = 0U;
        expected = source;
    }
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = Find(root, "studio.editor.code-tools");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    CHECK(UmiGtk4ActionMenuSetFilter(menu, "line editing") == UMI_STATUS_OK);
    Pump();
    CHECK(UmiGtk4ActionMenuVisibleCount(menu) == 9U);
    char control[128];
    (void)snprintf(control, sizeof(control), "studio.editor.line-%s", action);
    GtkWidget *button = Find(root, control);
    CHECK(GTK_IS_BUTTON(button) && gtk_widget_is_ancestor(button, menu));
    retained = g_object_ref(button);
    UmiDocumentWorkingCopySnapshot before, after;
    CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
    if (strcmp(name, "retained-control") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        expected = source;
    }
    g_signal_emit_by_name(retained, "clicked");
    Pump();
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(expected) && strcmp(output, expected) == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK);
    CHECK(after.undo_count == before.undo_count + (strcmp(source, expected) == 0 ? 0U : 1U));
    if (strcmp(name, "selection") == 0)
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
              view.cursor_offset == 0U && view.selection_length == 13U);
    if (strcmp(name, "undo-redo") == 0)
    {
        CHECK(umi_document_coordinator_undo(documents) == UMI_STATUS_OK);
        UmiUiDocumentViewModelFreeText(output);
        output = NULL;
        CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
              strcmp(output, source) == 0);
        CHECK(umi_document_coordinator_redo(documents) == UMI_STATUS_OK);
        UmiUiDocumentViewModelFreeText(output);
        output = NULL;
        CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
              strcmp(output, expected) == 0);
    }
cleanup:
    UmiUiDocumentViewModelFreeText(output);
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
