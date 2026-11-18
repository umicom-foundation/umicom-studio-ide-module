/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_zen_menu_gtk4.c
 *
 * PURPOSE:
 *   Check native Zen menu selection, return layouts and preservation of unsaved drafts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "document_baseline.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/document_view.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)
#define ID(name) UMI_STUDIO_RUNTIME_WORKSPACE_PREFIX name

static GtkWidget *MenuCommand(GtkWidget *root)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, "view.zen") == 0 &&
        g_object_get_data(G_OBJECT(root), "umicom-contribution-id") != NULL) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = MenuCommand(child);
        if (found != NULL) return found;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *mode = argv[1];
    const char *modes[] = {"menu", "direct", "custom", "direct-zen", "intervening",
        "busy", "argument", "retained"};
    bool known = false;
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (strcmp(mode, modes[i]) == 0) known = true;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, moved = 0;
    StudioTestDocumentBaseline documentBaseline = {0};
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *output = NULL, document_id[128] = {0}; size_t output_bytes = 0U;
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; GtkApplication *application = NULL;
    GtkWidget *retained = NULL;
    UmiUiWorkspaceLayout *before = g_try_new0(UmiUiWorkspaceLayout, 1);
    UmiUiWorkspaceLayout *after = g_try_new0(UmiUiWorkspaceLayout, 1);
    CHECK(before != NULL && after != NULL);
    base = g_dir_make_tmp("umicom-zen-menu-XXXXXX", NULL);
    CHECK(base != NULL && g_chdir(base) == 0); moved = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions service_options = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.zen-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_new(documents, "draft.c", document_id, sizeof document_id) == UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    const char *text = "int draft_value = 17;\n";
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 4U; view.selection_length = 6U; view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(StudioTestDocumentsCapture(documents, views, &documentBaseline) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    if (strcmp(mode, "custom") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench, ID("my-canvas"), "My canvas") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, "centre") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    }
    if (strcmp(mode, "direct-zen") == 0)
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "studio.layout.zen", NULL) == UMI_STATUS_OK);
    if (strcmp(mode, "busy") == 0)
        CHECK(umi_studio_gtk_workbench_workspace_begin_edit(workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, before) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.view-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *row = MenuCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))));
    CHECK(GTK_IS_BUTTON(row)); retained = g_object_ref(row);
    CHECK(gtk_widget_get_sensitive(row) == (strcmp(mode, "busy") != 0));
    if (strcmp(mode, "retained") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        goto inspect;
    }
    if (strcmp(mode, "argument") == 0 || strcmp(mode, "busy") == 0) {
        UmiStatus result = umi_studio_gtk_workbench_execute_command(workbench, "view.zen",
            strcmp(mode, "argument") == 0 ? "unexpected" : NULL);
        CHECK(result != UMI_STATUS_OK);
        if (strcmp(mode, "argument") == 0) CHECK(result == UMI_STATUS_INVALID_ARGUMENT);
        else g_signal_emit_by_name(row, "clicked");
        CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
        CHECK(memcmp(before, after, sizeof *before) == 0);
        goto inspect;
    }
    if (strcmp(mode, "menu") == 0) g_signal_emit_by_name(row, "clicked");
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench, "view.zen", NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
    if (strcmp(mode, "direct-zen") == 0) {
        CHECK(strcmp(after->layout_id, ID("default")) == 0);
        goto inspect;
    }
    CHECK(strcmp(after->layout_id, ID("zen")) == 0);
    if (strcmp(mode, "intervening") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "studio.layout.testing", NULL) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, before) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "view.zen", NULL) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "view.zen", NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
    CHECK(memcmp(before, after, sizeof *before) == 0);
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && memcmp(output, text, output_bytes) == 0);
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 6U);
/* The coordinator adopts Welcome and all contributed views; compare the complete captured set instead of assuming only the new draft exists. The former implementation is retained for owner review. */
#if 0
    CHECK(umi_document_coordinator_count(documents) == 1U);
#endif
    CHECK(StudioTestDocumentsUnchanged(documents, views, &documentBaseline));
cleanup:
    StudioTestDocumentsRelease(&documentBaseline);
    UmiUiDocumentViewModelFreeText(output);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original); g_free(before); g_free(after);
    return failed;
}
