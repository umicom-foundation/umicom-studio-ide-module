/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_folding_gtk4.c
 * PURPOSE: Verify Studio manual folding preserves authoritative drafts, history and retained control lifetime.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/provider_connections/gtk4.h"
#include "umicom/ui/gtk4/text_folding.h"
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

static GtkTextView *Editor(GtkWidget *root, const char *id)
{
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (actual != NULL && strcmp(actual, id) == 0)
    {
        GtkWidget *view = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
        if (GTK_IS_TEXT_VIEW(view))
            return GTK_TEXT_VIEW(view);
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkTextView *view = Editor(child, id);
        if (view != NULL)
            return view;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *name = argv[1];
    const char *cases[] = {"fold",         "reveal",       "source-history", "clean",           "dirty",
                           "caret",        "edit",         "refresh",        "second-document", "retained",
                           "parent-close", "no-selection", "single-line"};
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
    GtkTextView *native = NULL;
    GtkWidget *reveal = NULL;
    char *output = NULL;
    size_t output_bytes = 0U;
    GError *error = NULL;
    gchar *saved_path = NULL;
    directory = g_dir_make_tmp("umicom-folding-XXXXXX", &error);
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
    application = gtk_application_new("org.umicom.studio.folding-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.fold-selection");
    CHECK(GTK_IS_BUTTON(button));
    retained = g_object_ref(button);

    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = "header\nbody\ntail\n";
    view.cursor_offset = 0U;
    view.selection_length = strcmp(name, "no-selection") == 0  ? 0U
                            : strcmp(name, "single-line") == 0 ? 3U
                                                               : 12U;
    view.dirty = strcmp(name, "clean") == 0 ? 0 : 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    /* A new untitled draft is dirty even when its view flag is clear. Save
     * the clean-state fixture to its own temporary directory before folding. */
    if (strcmp(name, "clean") == 0)
    {
        saved_path = g_build_filename(directory, "folding.c", NULL);
        CHECK(umi_document_coordinator_save_active_as(documents, saved_path) == UMI_STATUS_OK);
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
        view.cursor_offset = 0U;
        view.selection_length = 12U;
        CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    native = Editor(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), id);
    CHECK(native != NULL);
    g_object_ref(native);
    reveal = Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.reveal-folds");
    CHECK(GTK_IS_BUTTON(reveal));
    g_object_ref(reveal);
    UmiDocumentWorkingCopySnapshot before, after;
    CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
    CHECK(before.dirty == (strcmp(name, "clean") != 0));
    if (strcmp(name, "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        g_signal_emit_by_name(reveal, "clicked");
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
        goto inspect;
    }
    g_signal_emit_by_name(button, "clicked");
    if (strcmp(name, "no-selection") == 0 || strcmp(name, "single-line") == 0)
    {
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
        goto inspect;
    }
    CHECK(UmiGtk4TextFoldCount(native) == 1U && UmiGtk4TextFoldLineHidden(native, 2U));
    CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK);
    CHECK(after.revision == before.revision && after.undo_count == before.undo_count &&
          after.redo_count == before.redo_count && after.dirty == before.dirty);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(native);
    GtkTextIter at;
    if (strcmp(name, "reveal") == 0)
    {
        g_signal_emit_by_name(reveal, "clicked");
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
    }
    if (strcmp(name, "caret") == 0)
    {
        gtk_text_buffer_get_iter_at_offset(buffer, &at, 8);
        gtk_text_buffer_place_cursor(buffer, &at);
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
    }
    if (strcmp(name, "edit") == 0)
    {
        gtk_text_buffer_get_end_iter(buffer, &at);
        gtk_text_buffer_begin_user_action(buffer);
        gtk_text_buffer_insert(buffer, &at, "more", 4);
        gtk_text_buffer_end_user_action(buffer);
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
        text = "header\nbody\ntail\nmore";
    }
    if (strcmp(name, "source-history") == 0)
    {
        gtk_text_buffer_get_end_iter(buffer, &at);
        gtk_text_buffer_begin_user_action(buffer);
        gtk_text_buffer_insert(buffer, &at, "more", 4);
        gtk_text_buffer_end_user_action(buffer);
        CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorUndo(documents, before.document_id) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(UmiGtk4TextFoldCount(native) == 0U);
    }
    if (strcmp(name, "refresh") == 0)
    {
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(Editor(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), id) == native &&
              UmiGtk4TextFoldCount(native) == 1U);
    }
    if (strcmp(name, "second-document") == 0)
    {
        char second[UMI_UI_ID_CAPACITY];
        CHECK(umi_document_coordinator_new(documents, "second.c", second, sizeof(second)) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        g_signal_emit_by_name(reveal, "clicked");
        CHECK(UmiGtk4TextFoldCount(native) == 1U);
    }
    if (strcmp(name, "parent-close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        g_signal_emit_by_name(reveal, "clicked");
        CHECK(UmiGtk4TextFoldCount(native) == 1U);
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    g_clear_object(&reveal);
    g_clear_object(&native);
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
    if (saved_path != NULL)
        (void)g_remove(saved_path);
    g_free(saved_path);
    g_free(directory);
    g_free(original);
    return failed;
}
