/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_checkpoint_menu_gtk4.c
 *
 * PURPOSE:
 *   Verify checkpoint menu ownership, storage conflict refusal and unsaved document preservation.
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

static GtkWidget *FindCommand(GtkWidget *root, const char *command)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, command) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindCommand(child, command);
        if (found != NULL) return found;
    }
    return NULL;
}


int main(int argc, char **argv)
{
    const char *modes[] = {"no-storage-save", "no-storage-restore", "argument-save",
        "argument-restore", "busy-save", "busy-restore", "empty-restore",
        "menu-roundtrip", "direct-roundtrip", "compatibility-roundtrip",
        "transaction-save", "transaction-restore", "conflict", "retained-save", "retained-restore"};
    if (argc != 2) return 2;
    const char *mode = argv[1];
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
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL, *other_bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL, *other = NULL; GtkApplication *application = NULL;
    UmiDataServer *server = NULL;
    GtkWidget *retained = NULL;
    UmiUiWorkspaceLayout *before = g_try_new0(UmiUiWorkspaceLayout, 1);
    UmiUiWorkspaceLayout *after = g_try_new0(UmiUiWorkspaceLayout, 1);
    UmiUiWorkspaceCheckpointReport initial = {0}, saved = {0}, observed = {0};
    CHECK(before != NULL && after != NULL);
    base = g_dir_make_tmp("umicom-checkpoint-menu-XXXXXX", NULL);
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
    application = gtk_application_new("org.umicom.studio.checkpoint-menu-test", G_APPLICATION_NON_UNIQUE);
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

    CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
        ID("checkpoint-menu-saved"), "Checkpoint menu saved") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench,
        UMI_STUDIO_SURFACE_EDITOR, "centre") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, before) == UMI_STATUS_OK);
    int restoring = strstr(mode, "restore") != NULL;
    const char *command = restoring ? "layout.restore" : "layout.save";
    if (strncmp(mode, "no-storage-", 11U) != 0) {
        CHECK(umi_data_server_create_memory(&server) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_bind_storage(workbench, server, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &initial) == UMI_STATUS_OK);
        CHECK(initial.storage_revision_known && initial.storage_revision == 0U && !initial.durable);
    }
    if (strncmp(mode, "busy-", 5U) == 0)
        CHECK(umi_studio_gtk_workbench_workspace_begin_edit(workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.window-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *popover = GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu)));
    GtkWidget *save_row = FindCommand(popover, "layout.save");
    GtkWidget *restore_row = FindCommand(popover, "layout.restore");
    CHECK(GTK_IS_BUTTON(save_row) && GTK_IS_BUTTON(restore_row));
    CHECK(gtk_widget_get_sensitive(save_row) ==
        (server != NULL && strncmp(mode, "busy-", 5U) != 0));
    CHECK(gtk_widget_get_sensitive(restore_row) ==
        (server != NULL && strncmp(mode, "busy-", 5U) != 0));
    retained = g_object_ref(restoring ? restore_row : save_row);
    size_t records_before = server != NULL ? umi_data_server_count(server) : 0U;
    if (strncmp(mode, "retained-", 9U) == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(umi_data_server_count(server) == records_before);
        goto inspect;
    }
    if (strncmp(mode, "no-storage-", 11U) == 0) {
/* No storage disables the public command before the handler can report UNAVAILABLE. The former implementation is retained for owner review. */
#if 0
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_UNAVAILABLE);
#endif
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_PERMISSION_DENIED);
        goto unchanged;
    }
    if (strncmp(mode, "argument-", 9U) == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, "unexpected") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(umi_data_server_count(server) == records_before);
        goto unchanged;
    }
    if (strncmp(mode, "busy-", 5U) == 0) {
/* An active layout edit disables public command admission. Transaction refusal later still reaches the handler and expects BUSY. The former implementation is retained for owner review. */
#if 0
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_BUSY);
#endif
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_PERMISSION_DENIED);
        CHECK(umi_data_server_count(server) == records_before);
        CHECK(umi_studio_gtk_workbench_workspace_cancel_edit(workbench) == UMI_STATUS_OK);
        goto unchanged;
    }
    if (strcmp(mode, "empty-restore") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_NOT_FOUND);
        CHECK(umi_data_server_count(server) == records_before);
        goto unchanged;
    }
    if (strncmp(mode, "transaction-", 12U) == 0) {
        CHECK(umi_data_server_begin(server) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_BUSY);
        CHECK(umi_data_server_in_transaction(server));
        CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &observed) == UMI_STATUS_OK);
        CHECK(observed.storage_revision_known && observed.storage_revision == initial.storage_revision);
        CHECK(umi_data_server_rollback(server) == UMI_STATUS_OK);
        CHECK(umi_data_server_count(server) == records_before);
        goto unchanged;
    }
    if (strcmp(mode, "conflict") == 0) {
        CHECK(umi_studio_bootstrap_create_with_options(&service_options, &other_bootstrap) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_create_with_options(application,
            umi_studio_bootstrap_ui(other_bootstrap), umi_studio_bootstrap_desktop_shell(other_bootstrap),
            &options, &other) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_bind_storage(other, server, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(other, "studio.layout.testing", NULL) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_save(other) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "layout.save", NULL) == UMI_STATUS_INVALID_STATE);
        CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &observed) == UMI_STATUS_OK);
        CHECK(observed.storage_revision_known && observed.storage_revision == initial.storage_revision);
        CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
        CHECK(memcmp(before, after, sizeof *before) == 0);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "layout.restore", NULL) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
        CHECK(strcmp(after->layout_id, ID("testing")) == 0);
        CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &observed) == UMI_STATUS_OK);
        CHECK(observed.storage_revision == 1U);
        goto inspect;
    }
    if (strcmp(mode, "menu-roundtrip") == 0) g_signal_emit_by_name(save_row, "clicked");
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench,
        strcmp(mode, "compatibility-roundtrip") == 0 ? "studio.layout.save" : "layout.save", NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &saved) == UMI_STATUS_OK);
    CHECK(saved.storage_revision_known && saved.storage_revision == 1U && !saved.durable);
    CHECK(strcmp(saved.layout_id, before->layout_id) == 0);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "studio.layout.testing", NULL) == UMI_STATUS_OK);
    if (strcmp(mode, "menu-roundtrip") == 0) g_signal_emit_by_name(restore_row, "clicked");
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench,
        strcmp(mode, "compatibility-roundtrip") == 0 ? "studio.layout.restore" : "layout.restore", NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, after) == UMI_STATUS_OK);
    CHECK(strcmp(after->layout_id, before->layout_id) == 0);
    CHECK(after->window_count == before->window_count);
    CHECK(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &observed) == UMI_STATUS_OK);
    CHECK(observed.storage_revision == saved.storage_revision);
    goto inspect;
unchanged:
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
    if (other != NULL) umi_studio_gtk_workbench_destroy(other);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    if (other_bootstrap != NULL) umi_studio_bootstrap_destroy(other_bootstrap);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (server != NULL) {
        if (umi_data_server_in_transaction(server)) (void)umi_data_server_rollback(server);
        umi_data_server_destroy(server);
    }
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original); g_free(before); g_free(after);
    return failed;
}
