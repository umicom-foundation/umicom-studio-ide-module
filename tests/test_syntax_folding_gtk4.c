/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_syntax_folding_gtk4.c
 * PURPOSE: Exercise Studio editor syntax-folding discovery, explicit launch boundary and retained controls.
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

/* Use the production syntax-folding window and document transaction. No provider,
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

static int Wait(GtkWidget *window)
{
    gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
    while (g_object_get_data(G_OBJECT(window), "umicom-completion-pending") != NULL &&
           g_get_monotonic_time() < deadline)
    {
        g_main_context_iteration(NULL, FALSE);
        g_usleep(1000U);
    }
    return g_object_get_data(G_OBJECT(window), "umicom-completion-pending") == NULL;
}
static int ProgramMain(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    const char *name = argv[1];
    const char *cases[] = {"open",         "cancel",      "retained",        "empty",
                           "parent-close", "collapse",    "reveal",          "draft-history",
                           "readonly",     "single-line", "invalid-response"};
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
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.editor.syntax-folding");
    CHECK(GTK_IS_BUTTON(button));
    retained = g_object_ref(button);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = strcmp(name,"empty")==0?"":"head\nbody\nend\ntail";
    view.cursor_offset = strcmp(name,"empty")==0?0U:2U;
    view.selection_length = 0U;
    view.read_only = strcmp(name, "readonly") == 0;
    view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
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
    /* Source information uses a read-only form; formatter assertions are
     * retained below to make the corrected fixture expectation reviewable. */
#if 0
    CHECK(strcmp(gtk_window_get_title(dialog), "Review document formatting") == 0);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.tab-size") != NULL);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.insert-spaces") != NULL);
#endif
    CHECK(strcmp(gtk_window_get_title(dialog), "Syntax folding regions") == 0);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.tab-size") == NULL);
    CHECK(Find(GTK_WIDGET(dialog), "document.formatting.insert-spaces") == NULL);
    CHECK((Find(GTK_WIDGET(dialog), "document.source-navigation.include-declaration") != NULL) == 0);
    GtkWidget *program = Find(GTK_WIDGET(dialog), "document.completion.program");
    GtkWidget *request = Find(GTK_WIDGET(dialog), "document.completion.request");
    GtkWidget *apply = Find(GTK_WIDGET(dialog), "document.completion.apply");
    GtkWidget *cancel = Find(GTK_WIDGET(dialog), "document.completion.cancel");
    CHECK(program != NULL && request != NULL && apply != NULL && cancel != NULL);
    CHECK(gtk_editable_get_text(GTK_EDITABLE(program))[0] == '\0');
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    CHECK(!gtk_widget_get_sensitive(apply) && gtk_window_get_default_widget(dialog) == cancel);
    CHECK(!gtk_widget_get_visible(apply));
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
    if (strcmp(name, "collapse") == 0 || strcmp(name, "reveal") == 0 || strcmp(name, "draft-history") == 0 ||
        strcmp(name, "readonly") == 0 || strcmp(name, "single-line") == 0 ||
        strcmp(name, "invalid-response") == 0)
    {
        /* Use a deterministic process peer through the real native toolbar.
         * Discovery must not edit, move or hide the active draft by itself. */
        GtkWidget *arguments = Find(GTK_WIDGET(dialog), "document.completion.arguments");
        GtkWidget *choice = Find(GTK_WIDGET(dialog), "document.completion.choice");
        GtkWidget *select = Find(GTK_WIDGET(dialog), "document.completion.preview");
        CHECK(arguments != NULL && choice != NULL && select != NULL);
        gtk_editable_set_text(GTK_EDITABLE(program), argv[2]);
        gtk_editable_set_text(GTK_EDITABLE(arguments), strcmp(name, "single-line") == 0 ? "foldings-single"
                                                       : strcmp(name, "invalid-response") == 0
                                                           ? "foldings-invalid"
                                                           : "foldings-valid");
        g_signal_emit_by_name(request, "clicked");
        CHECK(Wait(GTK_WIDGET(dialog)));
        if (strcmp(name, "invalid-response") == 0)
        {
            CHECK(!gtk_widget_get_sensitive(select));
            goto inspect;
        }
        CHECK(gtk_widget_get_sensitive(select));
        GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
        GtkTextView *editor = Editor(root, id);
        CHECK(editor != NULL && UmiGtk4TextFoldCount(editor) == 0U);
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
              view.cursor_offset == 2U && view.selection_length == 0U);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(choice), 0U);
        g_signal_emit_by_name(select, "clicked");
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK &&
              view.selection_length == (strcmp(name, "single-line") == 0 ? 5U : 14U));
        UmiDocumentWorkingCopySnapshot before, after;
        CHECK(umi_document_coordinator_active_snapshot(documents, &before) == UMI_STATUS_OK);
        GtkWidget *fold = Find(root, "studio.editor.fold-selection");
        CHECK(GTK_IS_BUTTON(fold));
        g_signal_emit_by_name(fold, "clicked");
        CHECK(UmiGtk4TextFoldCount(editor) == (strcmp(name, "single-line") == 0 ? 0U : 1U));
        if (strcmp(name, "single-line") != 0)
            CHECK(UmiGtk4TextFoldLineHidden(editor, 2U) && UmiGtk4TextFoldLineHidden(editor, 3U) &&
                  !UmiGtk4TextFoldLineHidden(editor, 1U) && !UmiGtk4TextFoldLineHidden(editor, 4U));
        CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK);
        CHECK(before.revision == after.revision && before.undo_count == after.undo_count &&
              before.redo_count == after.redo_count && before.dirty == after.dirty);
        if (strcmp(name, "reveal") == 0)
        {
            g_signal_emit_by_name(Find(root, "studio.editor.reveal-folds"), "clicked");
            CHECK(UmiGtk4TextFoldCount(editor) == 0U);
        }
        if (strcmp(name, "draft-history") == 0)
        {
            CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK &&
                  UmiGtk4TextFoldCount(editor) == 1U);
            CHECK(umi_document_coordinator_active_snapshot(documents, &after) == UMI_STATUS_OK &&
                  before.undo_count == after.undo_count);
        }
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    if (dialog != NULL)
    {
        gtk_window_destroy(dialog);
        (void)Wait(GTK_WIDGET(dialog));
    }
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

#include "../../../framework/tests/native_process/utf8_entry.inc"
