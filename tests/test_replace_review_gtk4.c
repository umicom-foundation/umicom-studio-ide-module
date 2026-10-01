/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_replace_review_gtk4.c
 * PURPOSE: Exercise real Replace All review, cancellation, stale plans and retained buttons.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/document/edit.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)

/* Find semantic controls without depending on child positions or labels. */
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    if (root == NULL) return NULL;
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = Find(child, tag);
        if (found != NULL) return found;
    }
    return NULL;
}

/* The returned dialog reference belongs to the fixture, not the toplevel list. */
static GtkWindow *FindReview(void)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
        GtkWindow *window = g_list_model_get_item(windows, i);
        const char *title = gtk_window_get_title(window);
        if (title != NULL && strcmp(title, "Review Replace All") == 0) return window;
        g_object_unref(window);
    }
    return NULL;
}

/* Compare all visible bytes and assert comparison panes cannot edit them. */
static int PaneEquals(GtkWidget *widget, const char *expected)
{
    if (!GTK_IS_TEXT_VIEW(widget) || gtk_text_view_get_editable(GTK_TEXT_VIEW(widget))) return 0;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter first, last; gtk_text_buffer_get_bounds(buffer, &first, &last);
    char *text = gtk_text_buffer_get_text(buffer, &first, &last, TRUE);
    int same = text != NULL && strcmp(text, expected) == 0;
    g_free(text); return same;
}

/* Publish a real unsaved draft, without writing a source file. */
static UmiStatus Draft(UmiUiDocumentViewModel *views, const char *id, const char *text)
{
    UmiUiDocumentViewSnapshot view;
    UmiStatus status = umi_ui_document_view_model_find(views, id, &view);
    if (status != UMI_STATUS_OK) return status;
    view.dirty = 1; view.cursor_offset = 0U; view.selection_length = 0U;
    return UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text));
}

/* Inspect the shared model, independent of the comparison's captured text. */
static int TextEquals(UmiUiDocumentViewModel *views, const char *id, const char *expected)
{
    char *text = NULL; size_t bytes = 0U;
    UmiStatus status = UmiUiDocumentViewModelCopyText(views, id, &text, &bytes);
    int same = status == UMI_STATUS_OK && bytes == strlen(expected) && strcmp(text, expected) == 0;
    UmiUiDocumentViewModelFreeText(text); return same;
}

/* Each case uses its own product graph and temporary settings directory. The
 * owner stays unpresented, while the actual modal review is briefly shown.
 * No session restore, discovery, build process or personal database is used. */
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *name = argv[1];
    const char *initial = "note NOTE\n";
    const char *proposed = "saved saved\n";
    int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkWindow *dialog = NULL;
    GtkWidget *accept = NULL, *cancel = NULL, *retained = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    char viewId[UMI_UI_ID_CAPACITY];
    UmiDocumentWorkingCopySnapshot original, current;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) { puts("SKIP: GTK display unavailable."); return 77; }
    originalDirectory = g_get_current_dir(); originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-replacement-review-XXXXXX", &error);
    CHECK(originalDirectory != NULL && directory != NULL && g_chdir(directory) == 0); changedDirectory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiWorkbench *model = umi_studio_ui_workbench(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(model);
    CHECK(umi_document_coordinator_new(documents, "Replacement review.c", viewId, sizeof viewId) == UMI_STATUS_OK);
    CHECK(Draft(views, viewId, initial) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents, &original) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.replacement-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *find = Find(root, "studio.editor.find-text"), *replace = Find(root, "studio.editor.replace-text");
    GtkWidget *replaceAll = Find(root, "studio.editor.replace-all"); CHECK(GTK_IS_BUTTON(replaceAll));
    retained = g_object_ref(replaceAll);
    CHECK(GTK_IS_EDITABLE(find) && GTK_IS_EDITABLE(replace));
    gtk_editable_set_text(GTK_EDITABLE(find), strcmp(name, "no-match") == 0 ? "missing" : "note");
    gtk_editable_set_text(GTK_EDITABLE(replace), "saved");
    CHECK(umi_document_coordinator_active_snapshot(documents, &original) == UMI_STATUS_OK);
    g_signal_emit_by_name(retained, "clicked");
    dialog = FindReview();
    CHECK(TextEquals(views, viewId, initial));
    if (strcmp(name, "no-match") == 0) { CHECK(dialog == NULL); goto cleanup; }
    CHECK(dialog != NULL);
    CHECK(PaneEquals(Find(GTK_WIDGET(dialog), "umicom.comparison.left"), initial));
    CHECK(PaneEquals(Find(GTK_WIDGET(dialog), "umicom.comparison.right"), proposed));
    CHECK(umi_document_coordinator_active_snapshot(documents, &current) == UMI_STATUS_OK);
    CHECK(current.undo_count == original.undo_count && current.revision == original.revision);
    GtkWidget *acceptButton = Find(GTK_WIDGET(dialog), "studio.confirm.accept");
    GtkWidget *cancelButton = Find(GTK_WIDGET(dialog), "studio.confirm.cancel");
    CHECK(GTK_IS_BUTTON(acceptButton) && GTK_IS_BUTTON(cancelButton));
    accept = g_object_ref(acceptButton); cancel = g_object_ref(cancelButton);
    CHECK(gtk_window_get_default_widget(dialog) == cancel);
    if (strcmp(name, "preview") == 0 || strcmp(name, "cancel") == 0) {
        if (strcmp(name, "preview") == 0) gtk_window_destroy(dialog);
        else g_signal_emit_by_name(cancel, "clicked");
        g_signal_emit_by_name(accept, "clicked"); g_signal_emit_by_name(cancel, "clicked");
        CHECK(TextEquals(views, viewId, initial));
    } else if (strcmp(name, "stale") == 0) {
        CHECK(Draft(views, viewId, "typing after the question") == UMI_STATUS_OK);
        CHECK(PaneEquals(Find(GTK_WIDGET(dialog), "umicom.comparison.right"), proposed));
        g_signal_emit_by_name(accept, "clicked");
        CHECK(TextEquals(views, viewId, "typing after the question"));
    } else if (strcmp(name, "closed") == 0) {
        CHECK(UmiDocumentCoordinatorClose(documents, original.document_id, 1) == UMI_STATUS_OK);
        char otherView[UMI_UI_ID_CAPACITY];
        CHECK(umi_document_coordinator_new(documents, "Replacement review.c", otherView, sizeof otherView) == UMI_STATUS_OK);
        CHECK(Draft(views, otherView, "keep new document") == UMI_STATUS_OK);
        g_signal_emit_by_name(accept, "clicked");
        CHECK(TextEquals(views, otherView, "keep new document"));
    } else if (strcmp(name, "teardown") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap); bootstrap = NULL;
        g_signal_emit_by_name(accept, "clicked"); g_signal_emit_by_name(cancel, "clicked");
        g_signal_emit_by_name(retained, "clicked");
        GtkWindow *unexpected = FindReview(); int absent = unexpected == NULL;
        g_clear_object(&unexpected); CHECK(absent);
    } else if (strcmp(name, "apply") == 0 || strcmp(name, "inactive") == 0) {
        char otherView[UMI_UI_ID_CAPACITY] = {0};
        if (strcmp(name, "inactive") == 0) {
            CHECK(umi_document_coordinator_new(documents, "side note", otherView, sizeof otherView) == UMI_STATUS_OK);
            CHECK(Draft(views, otherView, "unrelated unsaved text") == UMI_STATUS_OK);
        }
        /* Later field edits cannot change the already-reviewed proposal. */
        gtk_editable_set_text(GTK_EDITABLE(replace), "unreviewed text");
        g_signal_emit_by_name(accept, "clicked");
        CHECK(TextEquals(views, viewId, proposed));
        if (otherView[0] != '\0') {
            CHECK(umi_document_coordinator_active_snapshot(documents, &current) == UMI_STATUS_OK);
            CHECK(strcmp(current.view_id, otherView) == 0 && TextEquals(views, otherView, "unrelated unsaved text"));
        }
        CHECK(UmiDocumentCoordinatorUndo(documents, original.document_id) == UMI_STATUS_OK);
        CHECK(TextEquals(views, viewId, initial));
    } else { failed = 2; goto cleanup; }
cleanup:
    if (dialog != NULL) gtk_window_destroy(dialog);
    g_clear_object(&dialog);
    if (accept != NULL) g_object_unref(accept);
    if (cancel != NULL) g_object_unref(cancel);
    if (retained != NULL) g_object_unref(retained);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0) failed = 1;
    if (isolated) {
        if (originalData != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated Studio review fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(originalDirectory); g_free(originalData);
    return failed;
}
