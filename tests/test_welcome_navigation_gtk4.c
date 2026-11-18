/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_welcome_navigation_gtk4.c
 *
 * PURPOSE:
 *   Verify explicit Welcome navigation, editor return and retained document state across native observers.
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


typedef struct WelcomeObserver {
    UmiStudioGtkWorkbench **owner;
    int close;
    unsigned calls;
    UmiStatus nested;
} WelcomeObserver;
static void Observe(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)object; (void)spec;
    WelcomeObserver *observer = data;
    if (observer->calls++ != 0U || *observer->owner == NULL) return;
    if (observer->close) {
        UmiStudioGtkWorkbench *owner = *observer->owner;
        *observer->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else observer->nested = umi_studio_gtk_workbench_execute_command(*observer->owner, "help.welcome", NULL);
}
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
static GtkWidget *FindWelcomeStack(GtkWidget *root)
{
    if (GTK_IS_STACK(root) && gtk_stack_get_child_by_name(GTK_STACK(root), "welcome") != NULL &&
        gtk_stack_get_child_by_name(GTK_STACK(root), "workbench") != NULL) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindWelcomeStack(child);
        if (found != NULL) return found;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    const char *modes[] = {"menu", "direct", "repeat", "refresh", "return", "return-button",
        "new-document", "argument", "return-argument", "retained-menu", "retained-return",
        "observer-close", "observer-reenter", "return-close"};
    if (argc != 2) return 2;
    const char *mode = argv[1]; bool known = false;
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
    GtkWidget *retained = NULL, *stack = NULL;
    gulong observer_id = 0U;
    WelcomeObserver observer = {&workbench, 0, 0U, UMI_STATUS_OK};
    base = g_dir_make_tmp("umicom-welcome-navigation-XXXXXX", NULL);
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
    application = gtk_application_new("org.umicom.studio.welcome-navigation-test", G_APPLICATION_NON_UNIQUE);
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

    GtkWindow *owner = umi_studio_gtk_workbench_window(workbench);
    GtkWidget *found_stack = FindWelcomeStack(GTK_WIDGET(owner));
    CHECK(found_stack != NULL); stack = g_object_ref(found_stack);
    CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "workbench"));
    GtkWidget *welcome = gtk_stack_get_child_by_name(GTK_STACK(stack), "welcome");
    GtkWidget *back = FindCommand(welcome, "studio.welcome.return");
    CHECK(GTK_IS_BUTTON(back));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(owner), "umicom.shell.help-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *row = FindCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), "help.welcome");
    CHECK(GTK_IS_BUTTON(row) && gtk_widget_get_sensitive(row));
    if (strcmp(mode, "retained-menu") == 0) {
        retained = g_object_ref(row);
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked"); goto inspect;
    }
    if (strcmp(mode, "argument") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.welcome", "unexpected") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "workbench")); goto inspect;
    }
    if (strncmp(mode, "observer-", 9U) == 0) {
        observer.close = strcmp(mode, "observer-close") == 0;
        observer_id = g_signal_connect(stack, "notify::visible-child-name", G_CALLBACK(Observe), &observer);
    }
    if (strcmp(mode, "menu") == 0) g_signal_emit_by_name(row, "clicked");
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.welcome", NULL) ==
        (observer.close ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
    if (observer_id != 0U) {
        CHECK(observer.calls != 0U);
/* Public command admission refuses reentry while its native operation guard disables the command; the handler is not reached. The former implementation is retained for owner review. */
#if 0
        if (!observer.close) CHECK(observer.nested == UMI_STATUS_BUSY);
#endif
        if (!observer.close) CHECK(observer.nested == UMI_STATUS_PERMISSION_DENIED);
        g_signal_handler_disconnect(stack, observer_id); observer_id = 0U;
    }
    if (observer.close) { CHECK(workbench == NULL); goto inspect; }
    CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "welcome"));
    if (strcmp(mode, "retained-return") == 0) {
        retained = g_object_ref(back);
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked"); goto inspect;
    }
    if (strcmp(mode, "repeat") == 0)
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.welcome", NULL) == UMI_STATUS_OK);
    if (strcmp(mode, "refresh") == 0) {
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    }
    if (strcmp(mode, "repeat") == 0 || strcmp(mode, "refresh") == 0) {
        CHECK(gtk_stack_get_child_by_name(GTK_STACK(stack), "welcome") == welcome);
        CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "welcome"));
    }
    if (strcmp(mode, "new-document") == 0) {
        char new_id[128];
        CHECK(umi_document_coordinator_new(documents, "other.c", new_id, sizeof new_id) == UMI_STATUS_OK);
        CHECK(strcmp(new_id, document_id) != 0);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "workbench"));
    }
    if (strcmp(mode, "return-argument") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "studio.welcome.return", "unexpected") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "welcome"));
    }
    if (strcmp(mode, "return") == 0 || strcmp(mode, "return-button") == 0 || strcmp(mode, "return-close") == 0) {
        if (strcmp(mode, "return-close") == 0 || strcmp(mode, "return") == 0) {
            observer.close = strcmp(mode, "return-close") == 0;
            observer_id = g_signal_connect(stack, "notify::visible-child-name", G_CALLBACK(Observe), &observer);
        }
        if (strcmp(mode, "return-button") == 0) g_signal_emit_by_name(back, "clicked");
        else CHECK(umi_studio_gtk_workbench_execute_command(workbench, "studio.welcome.return", NULL) ==
            (observer.close ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
        if (observer.close) { CHECK(workbench == NULL && observer.calls != 0U); goto inspect; }
/* Public command admission refuses reentry while its native operation guard disables the command; the handler is not reached. The former implementation is retained for owner review. */
#if 0
        if (strcmp(mode, "return") == 0) CHECK(observer.calls != 0U && observer.nested == UMI_STATUS_BUSY);
#endif
        if (strcmp(mode, "return") == 0) CHECK(observer.calls != 0U && observer.nested == UMI_STATUS_PERMISSION_DENIED);
        CHECK(!g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stack)), "workbench"));
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && memcmp(output, text, output_bytes) == 0);
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 6U);
/* Welcome is an adopted editable document; verify all original document identities and full text rather than assuming an initially empty coordinator. The former implementation is retained for owner review. */
#if 0
    CHECK(umi_document_coordinator_count(documents) == (strcmp(mode, "new-document") == 0 ? 2U : 1U));
#endif
    CHECK(StudioTestDocumentsMatchCount(documents, views, &documentBaseline,
        strcmp(mode, "new-document") == 0 ? 1U : 0U));
cleanup:
    StudioTestDocumentsRelease(&documentBaseline);
    if (observer_id != 0U) g_signal_handler_disconnect(stack, observer_id);
    UmiUiDocumentViewModelFreeText(output);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained); g_clear_object(&stack);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original);
    return failed;
}
