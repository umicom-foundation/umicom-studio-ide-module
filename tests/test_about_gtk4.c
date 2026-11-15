/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_about_gtk4.c
 *
 * PURPOSE:
 *   Verify Help About dispatch, actual product metadata, draft preservation and retained native ownership.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "../src/gui/dialogs/include/about.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "umicom/ui/document_view.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)
static GtkWidget *FindCommand(GtkWidget *root, const char *command)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && !strcmp(id, command)) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindCommand(child, command);
        if (found != NULL) return found;
    }
    return NULL;
}
static GtkWindow *FindAbout(GtkWindow *parent)
{
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
        GtkWindow *window = g_list_model_get_item(windows, i);
        if (GTK_IS_ABOUT_DIALOG(window) && gtk_window_get_transient_for(window) == parent) return window;
        g_object_unref(window);
    }
    return NULL;
}
typedef struct AboutOwner {
    UmiStudioGtkWorkbench **workbench;
    int close;
    unsigned calls;
    UmiStatus nested;
} AboutOwner;
static void Observe(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    AboutOwner *owner = data;
    if (!gtk_widget_get_visible(GTK_WIDGET(object)) || owner->calls++ != 0U) return;
    if (owner->close) {
        UmiStudioGtkWorkbench *workbench = *owner->workbench;
        *owner->workbench = NULL;
        umi_studio_gtk_workbench_destroy(workbench);
    } else owner->nested = umi_studio_gtk_workbench_execute_command(*owner->workbench, "help.about", NULL);
}
int main(int argc, char **argv)
{
    const char *modes[] = {"menu", "direct", "repeat", "reopen", "argument", "retained-menu",
        "owner-close", "observer-close", "observer-reenter", "legacy", "legacy-alias"};
    if (argc != 2) return 2;
    const char *mode = argv[1]; int known = 0;
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (!strcmp(mode, modes[i])) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, moved = 0;
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *output = NULL, document_id[128] = {0}; size_t output_bytes = 0U;
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; GtkApplication *application = NULL;
    GtkWidget *retained = NULL; GtkWindow *about = NULL, *again = NULL, *owner = NULL;
    gulong observer_id = 0U;
    AboutOwner observer = {&workbench, !strcmp(mode, "observer-close"), 0U, UMI_STATUS_OK};
    base = g_dir_make_tmp("umicom-studio-about-XXXXXX", NULL);
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
    application = gtk_application_new("org.umicom.studio.about-test", G_APPLICATION_NON_UNIQUE);
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
    owner = g_object_ref(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(owner), "umicom.shell.help-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *row = FindCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), "help.about");
    CHECK(GTK_IS_BUTTON(row) && gtk_widget_get_sensitive(row));
    CHECK(FindAbout(owner) == NULL);
    if (!strcmp(mode, "argument")) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.about", "unexpected") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(FindAbout(owner) == NULL); goto inspect;
    }
    if (!strcmp(mode, "retained-menu")) {
        retained = g_object_ref(row); umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(FindAbout(owner) == NULL); goto inspect;
    }
    if (!strcmp(mode, "menu")) g_signal_emit_by_name(row, "clicked");
    else if (!strcmp(mode, "legacy")) umi_about_show(owner);
    else if (!strcmp(mode, "legacy-alias")) umi_show_about(owner);
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.about", NULL) == UMI_STATUS_OK);
    about = FindAbout(owner); CHECK(about != NULL);
    CHECK(gtk_widget_get_visible(GTK_WIDGET(about)));
    CHECK(!strcmp(gtk_about_dialog_get_program_name(GTK_ABOUT_DIALOG(about)), "Umicom Studio IDE"));
    CHECK(!strcmp(gtk_about_dialog_get_version(GTK_ABOUT_DIALOG(about)), UMICOM_STUDIO_VERSION_STRING));
    CHECK(gtk_about_dialog_get_logo(GTK_ABOUT_DIALOG(about)) != NULL);
    if (!strcmp(mode, "repeat") || !strcmp(mode, "reopen")) {
        if (!strcmp(mode, "reopen")) gtk_window_destroy(about);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.about", NULL) == UMI_STATUS_OK);
        again = FindAbout(owner); CHECK(again != NULL);
        CHECK(!strcmp(mode, "repeat") ? again == about : again != about);
    } else if (!strcmp(mode, "owner-close")) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        CHECK(!UmiGtk4WindowIsOpen(about));
    } else if (!strncmp(mode, "observer-", 9U)) {
        gtk_widget_set_visible(GTK_WIDGET(about), FALSE);
        observer_id = g_signal_connect(about, "notify::visible", G_CALLBACK(Observe), &observer);
        UmiStatus result = umi_studio_gtk_workbench_execute_command(workbench, "help.about", NULL);
        CHECK(observer.calls != 0U);
        if (observer.close) CHECK(result == UMI_STATUS_CANCELLED && workbench == NULL);
        else CHECK(result == UMI_STATUS_OK && observer.nested == UMI_STATUS_BUSY);
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && !memcmp(output, text, output_bytes));
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 6U);
    CHECK(umi_document_coordinator_count(documents) == 1U);
cleanup:
    if (observer_id != 0U) g_signal_handler_disconnect(about, observer_id);
    UmiUiDocumentViewModelFreeText(output);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    if (about != NULL && UmiGtk4WindowIsOpen(about)) gtk_window_destroy(about);
    if (again != NULL && UmiGtk4WindowIsOpen(again)) gtk_window_destroy(again);
    g_clear_object(&again); g_clear_object(&about); g_clear_object(&owner); g_clear_object(&retained);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original);
    return failed;
}
