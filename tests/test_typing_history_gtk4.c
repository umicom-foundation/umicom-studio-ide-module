/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_typing_history_gtk4.c
 * PURPOSE: Verify native Studio typing restores captured selections through Framework Undo.
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

/* Follow the actual native editor binding; no test-owned text widget stands
 * in for Studio's document surface. Retained buffers are released separately. */
static GtkWidget *FindEditor(GtkWidget *root)
{
    if (GTK_IS_TEXT_VIEW(root) && gtk_widget_has_css_class(root, "umicom-editor"))
        return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkWidget *found = FindEditor(child);
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
    const char *cases[] = {"selection", "reverse", "unicode",        "multiline",
                           "caret",     "nested",  "retained-buffer"};
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
    GtkTextBuffer *buffer = NULL;

    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;

    directory = g_dir_make_tmp("umicom-typing-history-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.typing-history-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);

    const char *source = "alpha\nbravo", *expected = "aXa\nbravo";
    int first = 1, last = 4;
    size_t byte_first = 1U, selected = 3U;
    if (strcmp(name, "unicode") == 0)
    {
        source = "\xe9\x9b\xaa"
                 "alpha";
        expected = "\xe9\x9b\xaa"
                   "Xha";
        byte_first = 3U;
    }
    if (strcmp(name, "multiline") == 0)
    {
        last = 8;
        selected = 7U;
        expected = "aXavo";
    }
    if (strcmp(name, "caret") == 0)
    {
        last = first;
        selected = 0U;
        expected = "aXlpha\nbravo";
    }
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    view.cursor_offset = byte_first;
    view.selection_length = selected;
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *editor = FindEditor(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)));
    CHECK(editor != NULL);
    buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)));
    GtkTextIter a, b;
    gtk_text_buffer_get_iter_at_offset(buffer, &a, first);
    gtk_text_buffer_get_iter_at_offset(buffer, &b, last);
    gtk_text_buffer_select_range(buffer, strcmp(name, "reverse") == 0 ? &b : &a,
                                 strcmp(name, "reverse") == 0 ? &a : &b);
    UmiDocumentWorkingCopySnapshot before, after;
    CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
    gtk_text_buffer_begin_user_action(buffer);
    if (strcmp(name, "nested") == 0)
        gtk_text_buffer_begin_user_action(buffer);
    if (first != last)
        gtk_text_buffer_delete(buffer, &a, &b);
    gtk_text_buffer_insert(buffer, &a, "X", 1);
    gtk_text_buffer_place_cursor(buffer, &a);
    if (strcmp(name, "nested") == 0)
        gtk_text_buffer_end_user_action(buffer);
    gtk_text_buffer_end_user_action(buffer);
    Pump();
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
          strcmp(output, expected) == 0);
    CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK &&
          after.undo_count == before.undo_count + 1U);
    g_signal_emit_by_name(buffer, "undo");
    Pump();
    UmiUiDocumentViewModelFreeText(output);
    output = NULL;
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
          strcmp(output, source) == 0);
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
          view.cursor_offset == byte_first && view.selection_length == selected);
    gtk_text_buffer_get_iter_at_mark(buffer, &a, gtk_text_buffer_get_insert(buffer));
    gtk_text_buffer_get_iter_at_mark(buffer, &b, gtk_text_buffer_get_selection_bound(buffer));
    CHECK(MIN(gtk_text_iter_get_offset(&a), gtk_text_iter_get_offset(&b)) == first &&
          MAX(gtk_text_iter_get_offset(&a), gtk_text_iter_get_offset(&b)) == last);
    g_signal_emit_by_name(buffer, "redo");
    Pump();
    UmiUiDocumentViewModelFreeText(output);
    output = NULL;
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
          strcmp(output, expected) == 0);
    if (strcmp(name, "retained-buffer") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        gtk_text_buffer_begin_user_action(buffer);
        gtk_text_buffer_get_end_iter(buffer, &a);
        gtk_text_buffer_insert(buffer, &a, "late", 4);
        gtk_text_buffer_end_user_action(buffer);
        Pump();
        UmiUiDocumentViewModelFreeText(output);
        output = NULL;
        CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK &&
              strcmp(output, expected) == 0);
    }
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&buffer);
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
