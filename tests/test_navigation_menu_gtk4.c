/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_navigation_menu_gtk4.c
 *
 * PURPOSE:
 *   Exercise Navigate source inspection menus, toolbar parity and retained controls.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "line %d: %s\n", __LINE__, #c); \
    failed = 1; goto cleanup; } } while (0)

static GtkWidget *MenuCommand(GtkWidget *root, const char *command)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, command) == 0 &&
        g_object_get_data(G_OBJECT(root), "umicom-contribution-id") != NULL) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = MenuCommand(child, command);
        if (found != NULL) return found;
    }
    return NULL;
}
static GtkWindow *Review(void)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
        GtkWindow *candidate = g_list_model_get_item(windows, i);
        if (umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate),
            "document.completion.review") != NULL) return candidate;
        g_object_unref(candidate);
    }
    return NULL;
}

/* Exercise the real menu row and its existing Framework review. Request is
 * never activated, so a passing fixture would not qualify an external server. */
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *intent = argv[1], *mode = argv[2];
    const char *intents[] = {"symbol", "definition", "references"};
    const char *modes[] = {"open", "cancel", "retained", "busy", "argument",
        "toolbar", "read-only", "no-document", "parent-close"};
    int known_intent = 0, known_mode = 0;
    for (size_t i = 0U; i < 3U; ++i)
        if (strcmp(intent, intents[i]) == 0) known_intent = 1;
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (strcmp(mode, modes[i]) == 0) known_mode = 1;
    if (!known_intent || !known_mode) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, changed_directory = 0;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWindow *dialog = NULL;
    GtkWidget *retained = NULL;
    char *directory = NULL, *original = g_get_current_dir();
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *output = NULL;
    size_t output_bytes = 0U;
    UmiUiDocumentViewModel *views = NULL;
    char document_id[UMI_UI_ID_CAPACITY] = {0}, command[64];
    (void)snprintf(command, sizeof command, "navigate.%s", intent);
    const char *text = "int first;\nint second;\n";
    directory = g_dir_make_tmp("umicom-studio-navigation-XXXXXX", NULL);
    CHECK(directory != NULL && g_chdir(directory) == 0);
    changed_directory = 1;
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
    /* The application intentionally supplies an editable welcome document.
     * This fixture explicitly closes it to exercise an empty editor first. */
    CHECK(umi_document_coordinator_close_active(documents, 0) == UMI_STATUS_OK);
    views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.navigation-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(window,
        "umicom.shell.navigate-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *row = MenuCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), command);
    CHECK(GTK_IS_BUTTON(row) && !gtk_widget_get_sensitive(row));
    retained = g_object_ref(row);
    if (strcmp(mode, "no-document") == 0) {
        g_signal_emit_by_name(row, "clicked");
        CHECK(Review() == NULL && umi_document_coordinator_count(documents) == 0U);
        goto cleanup;
    }
    CHECK(umi_document_coordinator_new(documents, "draft.c", document_id, sizeof document_id) == UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 4U; view.selection_length = 5U; view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    if (strcmp(mode, "read-only") == 0) {
        view.read_only = 1;
        CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
    }
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench,
        UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    CHECK(MenuCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), command) == row);
    CHECK(gtk_widget_get_sensitive(row));
    if (strcmp(mode, "retained") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(Review() == NULL);
        goto inspect;
    }
    if (strcmp(mode, "argument") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, "unexpected") ==
            UMI_STATUS_INVALID_ARGUMENT);
        CHECK(Review() == NULL);
        goto inspect;
    }
    if (strcmp(mode, "toolbar") == 0) {
        const char *tag = strcmp(intent, "symbol") == 0 ? "studio.editor.symbols" :
            strcmp(intent, "definition") == 0 ? "studio.editor.definition" : "studio.editor.references";
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(window, tag);
        CHECK(GTK_IS_BUTTON(button));
        g_signal_emit_by_name(button, "clicked");
    } else g_signal_emit_by_name(row, "clicked");
    dialog = Review();
    CHECK(dialog != NULL);
    const char *title = strcmp(intent, "symbol") == 0 ? "Document symbols" :
        strcmp(intent, "definition") == 0 ? "Definition locations" : "Reference locations";
    CHECK(strcmp(gtk_window_get_title(dialog), title) == 0);
    CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    GtkWidget *cancel = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(dialog), "document.completion.cancel");
    CHECK(GTK_IS_BUTTON(cancel) && gtk_window_get_default_widget(dialog) == cancel);
    if (strcmp(mode, "busy") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(!gtk_widget_get_sensitive(row));
        g_signal_emit_by_name(retained, "clicked");
        GtkWindow *same = Review();
        CHECK(same == dialog); g_clear_object(&same);
        CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
    }
    if (strcmp(mode, "parent-close") == 0) {
        GtkWidget *request = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(dialog), "document.completion.request");
        CHECK(GTK_IS_BUTTON(request));
        g_object_ref(request);
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(request, "clicked");
        CHECK(g_object_get_data(G_OBJECT(dialog), "umicom-completion-pending") == NULL);
        g_object_unref(request);
    } else {
        g_signal_emit_by_name(cancel, "clicked");
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(gtk_widget_get_sensitive(row));
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 5U);
cleanup:
    UmiUiDocumentViewModelFreeText(output);
    if (dialog != NULL) gtk_window_destroy(dialog);
    g_clear_object(&dialog);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed_directory) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(directory); g_free(original);
    return failed;
}
