/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_bookmarks_gtk4.c
 * PURPOSE: Use real Studio editor controls to navigate session bookmarks across drafts.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/document/edit.h"
#include "umicom/document/bookmarks.h"
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



int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *name = argv[1];
    const char *cases[] = {"toggle", "next", "previous", "cross-document", "retained"};
    int known = 0;
    for (size_t index = 0U; index < sizeof cases / sizeof cases[0]; ++index) if (strcmp(name, cases[index]) == 0) known = 1;
    if (!known) return 2;
    const char *initial = "first\nsecond\nthird\n";
    int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *retained = NULL;
    char otherView[UMI_UI_ID_CAPACITY], viewId[UMI_UI_ID_CAPACITY];
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL;
    GError *error = NULL;
    UmiDocumentWorkingCopySnapshot original, current;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) { puts("SKIP: GTK display unavailable."); return 77; }
    originalDirectory = g_get_current_dir(); originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-bookmarks-XXXXXX", &error);
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
    CHECK(umi_document_coordinator_new(documents, "Second review.c", otherView, sizeof otherView) == UMI_STATUS_OK);
    CHECK(Draft(views, otherView, "note second") == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.bookmarks-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui, umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));

    GtkWidget *toggle = Find(root, "studio.editor.bookmark-toggle");
    GtkWidget *next = Find(root, "studio.editor.bookmark-next"), *previous = Find(root, "studio.editor.bookmark-previous");
    CHECK(GTK_IS_BUTTON(toggle) && GTK_IS_BUTTON(next) && GTK_IS_BUTTON(previous));
    retained = g_object_ref(toggle);
    /* A bookmark on the second document must not refer to the first by name. */
    g_signal_emit_by_name(toggle, "clicked");
    UmiDocumentBookmarksSnapshot bookmarks;
    CHECK(UmiDocumentCoordinatorBookmarks(documents, &bookmarks) == UMI_STATUS_OK && bookmarks.count == 1U);
    if (strcmp(name, "retained") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(UmiDocumentCoordinatorBookmarks(documents, &bookmarks) == UMI_STATUS_OK && bookmarks.count == 1U);
        goto cleanup;
    }
    if (strcmp(name, "toggle") == 0) {
        g_signal_emit_by_name(toggle, "clicked");
        CHECK(UmiDocumentCoordinatorBookmarks(documents, &bookmarks) == UMI_STATUS_OK && bookmarks.count == 0U);
    } else {
        CHECK(umi_ui_workbench_activate_document(model, viewId) == UMI_STATUS_OK);
        CHECK(UmiDocumentCoordinatorGoToPosition(documents, 2U, 1U, NULL) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        /* Refresh can rebuild a toolbar; reacquire semantic widgets each time. */
        toggle = Find(root, "studio.editor.bookmark-toggle"); CHECK(GTK_IS_BUTTON(toggle));
        g_signal_emit_by_name(toggle, "clicked");
        if (strcmp(name, "cross-document") != 0) {
            CHECK(UmiDocumentCoordinatorGoToPosition(documents, 3U, 1U, NULL) == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        }
        const char *tag = strcmp(name, "previous") == 0 ? "studio.editor.bookmark-previous" : "studio.editor.bookmark-next";
        GtkWidget *travel = Find(root, tag); CHECK(GTK_IS_BUTTON(travel));
        g_signal_emit_by_name(travel, "clicked");
        CHECK(umi_document_coordinator_active_snapshot(documents, &current) == UMI_STATUS_OK);
        if (strcmp(name, "previous") == 0) {
            CHECK(strcmp(current.view_id, viewId) == 0);
            UmiUiDocumentViewSnapshot view;
            CHECK(umi_ui_document_view_model_find(views, viewId, &view) == UMI_STATUS_OK && view.cursor_offset == 6U);
        } else CHECK(strcmp(current.view_id, otherView) == 0);
    }
    CHECK(TextEquals(views, viewId, initial) && TextEquals(views, otherView, "note second"));
cleanup:
    if (retained != NULL) g_object_unref(retained);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings); g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0) failed = 1;
    if (isolated) {
        if (originalData != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated Studio bookmark fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(originalDirectory); g_free(originalData);
    return failed;
}
