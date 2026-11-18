/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_shortcut_reference_gtk4.c
 *
 * PURPOSE:
 *   Verify native shortcut references use installed bindings and preserve source drafts across publication observers.
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


typedef struct ReferenceObserver {
    UmiStudioGtkWorkbench **owner; const char *mode;
    unsigned calls; UmiStatus nested;
} ReferenceObserver;
static void Observe(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    ReferenceObserver *observer = data;
    if (observer->calls++ != 0U || *observer->owner == NULL) return;
    if (strstr(observer->mode, "close") != NULL) {
        UmiStudioGtkWorkbench *owner = *observer->owner;
        *observer->owner = NULL; umi_studio_gtk_workbench_destroy(owner);
    } else if (strstr(observer->mode, "replace") != NULL) {
        GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(buffer, "Observer replacement", -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(object), buffer); g_object_unref(buffer);
    } else observer->nested = umi_studio_gtk_workbench_execute_command(*observer->owner, "help.shortcuts", NULL);
}
static void LocationChanged(GtkEditable *editable, gpointer data) { Observe(G_OBJECT(editable), NULL, data); }
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
    const char *modes[] = {"menu", "direct", "repeat", "argument", "retained-menu",
        "buffer-close", "label-close", "buffer-reenter", "label-reenter", "buffer-replace",
        "location-close", "location-reenter", "reopen"};
    if (argc != 2) return 2;
    const char *mode = argv[1]; bool known = false;
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i) if (!strcmp(mode, modes[i])) known = true;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, moved = 0;
    StudioTestDocumentBaseline documentBaseline = {0};
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *output = NULL, *reference = NULL, document_id[128] = {0}; size_t output_bytes = 0U;
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; GtkApplication *application = NULL;
    GtkWidget *retained = NULL, *preview = NULL, *summary = NULL, *location = NULL;
    GObject *observed = NULL; gulong observer_id = 0U;
    ReferenceObserver observer = {&workbench, mode, 0U, UMI_STATUS_OK};
    base = g_dir_make_tmp("umicom-layout-menu-XXXXXX", NULL);
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
    application = gtk_application_new("org.umicom.studio.layout-menu-test", G_APPLICATION_NON_UNIQUE);
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

    GtkWidget *owner = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu = umi_gtk4_automation_find_tagged_widget(owner, "umicom.shell.help-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *row = FindCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu))), "help.shortcuts");
    CHECK(GTK_IS_BUTTON(row) && gtk_widget_get_sensitive(row));
    if (!strcmp(mode, "retained-menu")) {
        retained = g_object_ref(row); umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked"); goto inspect;
    }
    if (!strcmp(mode, "argument")) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.shortcuts", "unexpected") == UMI_STATUS_INVALID_ARGUMENT);
        goto inspect;
    }
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL) == UMI_STATUS_OK);
    GtkWidget *found = umi_gtk4_automation_find_tagged_widget(owner, "studio.documentation.preview");
    CHECK(GTK_IS_TEXT_VIEW(found)); preview = g_object_ref(found);
    found = umi_gtk4_automation_find_tagged_widget(owner, "studio.documentation.summary");
    CHECK(GTK_IS_LABEL(found)); summary = g_object_ref(found);
    found = umi_gtk4_automation_find_tagged_widget(owner, "studio.documentation.location");
    CHECK(GTK_IS_EDITABLE(found)); location = g_object_ref(found);
    gtk_editable_set_text(GTK_EDITABLE(location), "previous-document.html");
    if (!strncmp(mode, "buffer-", 7U)) {
        observed = G_OBJECT(preview); observer_id = g_signal_connect(preview, "notify::buffer", G_CALLBACK(Observe), &observer);
    } else if (!strncmp(mode, "label-", 6U)) {
        observed = G_OBJECT(summary); observer_id = g_signal_connect(summary, "notify::label", G_CALLBACK(Observe), &observer);
    } else if (!strncmp(mode, "location-", 9U)) {
        observed = G_OBJECT(location); observer_id = g_signal_connect(location, "changed", G_CALLBACK(LocationChanged), &observer);
    }
    int cancelled = strstr(mode, "close") != NULL || strstr(mode, "replace") != NULL;
    if (!strcmp(mode, "menu")) g_signal_emit_by_name(row, "clicked");
    else CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.shortcuts", NULL) ==
        (cancelled ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
    if (observer_id != 0U) {
        CHECK(observer.calls != 0U);
/* Public command admission refuses reentry while its native operation guard disables the command; the handler is not reached. The former implementation is retained for owner review. */
#if 0
        if (strstr(mode, "reenter")) CHECK(observer.nested == UMI_STATUS_BUSY);
#endif
        if (strstr(mode, "reenter")) CHECK(observer.nested == UMI_STATUS_PERMISSION_DENIED);
        g_signal_handler_disconnect(observed, observer_id); observer_id = 0U;
    }
    if (strstr(mode, "close")) { CHECK(workbench == NULL); goto inspect; }
    if (cancelled) goto inspect;
    if (!strcmp(mode, "repeat") || !strcmp(mode, "reopen")) {
        if (!strcmp(mode, "reopen"))
            CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "help.shortcuts", NULL) == UMI_STATUS_OK);
        CHECK(umi_gtk4_automation_find_tagged_widget(owner, "studio.documentation.preview") == preview);
    }
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview));
    GtkTextIter start, end; gtk_text_buffer_get_bounds(buffer, &start, &end);
    reference = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    CHECK(strstr(reference, "Keyboard shortcuts") && strstr(reference, "file.save") &&
        strstr(reference, "studio.command.search") && strstr(reference, "navigate.document.next"));
    CHECK(strstr(reference, "[available]") != NULL && strstr(reference, "[unavailable]") != NULL);
    CHECK(gtk_editable_get_text(GTK_EDITABLE(location))[0] == '\0');
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(summary)), "Keyboard shortcuts") != NULL);
    CHECK(!gtk_text_view_get_editable(GTK_TEXT_VIEW(preview)) && !gtk_text_buffer_get_modified(buffer));
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && !memcmp(output, text, output_bytes));
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 6U);
/* Welcome is an adopted editable document; verify all original document identities and full text rather than assuming an initially empty coordinator. The former implementation is retained for owner review. */
#if 0
    CHECK(umi_document_coordinator_count(documents) == 1U);
#endif
    CHECK(StudioTestDocumentsUnchanged(documents, views, &documentBaseline));
cleanup:
    StudioTestDocumentsRelease(&documentBaseline);
    if (observer_id != 0U) g_signal_handler_disconnect(observed, observer_id);
    UmiUiDocumentViewModelFreeText(output); g_free(reference);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained); g_clear_object(&preview); g_clear_object(&summary); g_clear_object(&location);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original);
    return failed;
}
