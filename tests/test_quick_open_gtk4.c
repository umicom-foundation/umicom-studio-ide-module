/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_quick_open_gtk4.c
 * PURPOSE: Verify indexed file opening through the production Studio toolbar and workspace owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/provider_connections/gtk4.h"
#include "umicom/ui/gtk4/text_folding.h"
#include "umicom/ui/gtk4/action_menu.h"
#include "umicom/document/navigation_history.h"
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

/* Inspect production widgets without exposing the fixture to remote providers.
 * Settings and indexed source files live in a unique temporary directory. */
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

static void Pump(void)
{
    for (unsigned step = 0U; step < 512U && g_main_context_pending(NULL); ++step)
        (void)g_main_context_iteration(NULL, FALSE);
}

static GtkWidget *FindEditor(GtkWidget *root, const char *viewId)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (id != NULL && strcmp(id, viewId) == 0) {
        GtkWidget *editor = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
        if (GTK_IS_TEXT_VIEW(editor)) return editor;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindEditor(child, viewId);
        if (found != NULL) return found;
    }
    return NULL;
}

/* Observe native creation and publication through the production toolbar. */
typedef struct StudioQuickOpening {
    UmiStudioGtkWorkbench **slot;
    UmiStudioServices *services;
    GtkWidget *button;
    const char *other, *mode;
    GPtrArray *windows;
    unsigned observed;
    UmiStatus status;
} StudioQuickOpening;
static void StudioQuickOpeningAct(StudioQuickOpening *probe)
{
    if (probe->observed != 0U) return;
    ++probe->observed;
    if (strstr(probe->mode, "reenter") != NULL) g_signal_emit_by_name(probe->button, "clicked");
    else if (strstr(probe->mode, "workspace") != NULL)
        probe->status = umi_studio_workspace_open(probe->services, probe->other, 0, 0);
    else {
        UmiStudioGtkWorkbench *owner = *probe->slot; *probe->slot = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    }
}
static void StudioQuickOpeningNotify(GObject *object, GParamSpec *property, gpointer data)
{
    if (strcmp(property->name, "visible") == 0 && !gtk_widget_get_visible(GTK_WIDGET(object))) return;
    StudioQuickOpeningAct(data);
}
static void StudioQuickReplacingClosed(GtkWidget *window, gpointer data)
{
    (void)window; StudioQuickOpeningAct(data);
}
static void StudioQuickOpeningAdded(GListModel *model, guint position, guint removed, guint added, gpointer data)
{
    (void)removed;
    StudioQuickOpening *probe = data;
    for (guint i = 0U; i < added; ++i) {
        GtkWindow *window = g_list_model_get_item(model, position + i);
        if (window == NULL) continue;
        g_ptr_array_add(probe->windows, window);
        if (strncmp(probe->mode, "constructor-", 12U) == 0) StudioQuickOpeningAct(probe);
        else if (strcmp(probe->mode, "title-close") == 0)
            g_signal_connect(window, "notify::title", G_CALLBACK(StudioQuickOpeningNotify), probe);
        else if (strcmp(probe->mode, "child-close") == 0)
            g_signal_connect(window, "notify::child", G_CALLBACK(StudioQuickOpeningNotify), probe);
        else if (strncmp(probe->mode, "present-", 8U) == 0)
            g_signal_connect(window, "notify::visible", G_CALLBACK(StudioQuickOpeningNotify), probe);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    const char *name = argv[1], *cases[] = {"button",
                                            "keyboard",
                                            "capture-key",
                                            "filter",
                                            "empty",
                                            "open",
                                            "dirty-existing",
                                            "stale-index",
                                            "refresh-index",
                                            "workspace-switch",
                                            "workspace-return",
                                            "retained-control",
                                            "repeated-open",
                                            "no-workspace",
        "constructor-close", "constructor-reenter", "constructor-workspace", "title-close", "child-close", "present-close", "present-reenter", "replacement-close", "replacement-workspace"};
    int known = 0;
    for (size_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(name, cases[i]) == 0)
            known = 1;
    if (!known)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool changed_directory = false;
    char *original = g_get_current_dir(), *directory = NULL, *project = NULL, *other = NULL, *file = NULL,
         *extra = NULL, *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWindow *dialog = NULL, *retainedOwner = NULL, *previousPicker = NULL;
    GtkWidget *openingButton = NULL;
    GListModel *openingWindows = NULL;
    gulong openingHandler = 0U;
    StudioQuickOpening opening = {0};
    opening.slot = &workbench; opening.mode = name;
    opening.windows = g_ptr_array_new_with_free_func(g_object_unref);
    opening.status = UMI_STATUS_OK;
    GtkWidget *retained = NULL;
    GError *error = NULL;
    GListModel *controllers = NULL;
    char *output = NULL;
    size_t output_bytes = 0U;
    /* Retain the former temporary location choice for review. Native fixtures
     * now keep their files under the explicitly selected build directory. */
#if 0
    directory = g_dir_make_tmp("umicom-studio-quick-open-XXXXXX", &error);
#endif
    CHECK(g_path_is_absolute(argv[2]));
    directory = g_build_filename(argv[2], "quick-open-XXXXXX", NULL);
    CHECK(g_mkdtemp(directory) != NULL);
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
    UmiStudioServicesOptions options = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    project = g_build_filename(directory, "project", NULL);
    other = g_build_filename(directory, "other", NULL);
    CHECK(g_mkdir(project, 0700) == 0 && g_mkdir(other, 0700) == 0);
    file = g_build_filename(project, "main.c", NULL);
    extra = g_build_filename(project, "other.c", NULL);
    CHECK(g_file_set_contents(file, "saved source", 12, NULL));
    if (strcmp(name, "no-workspace") != 0)
        CHECK(umi_studio_workspace_open(services, project, 0, 0) == UMI_STATUS_OK);
    char id[UMI_UI_ID_CAPACITY];
    if (strcmp(name, "dirty-existing") == 0)
        CHECK(umi_document_coordinator_open(documents, file, id, sizeof(id)) == UMI_STATUS_OK);
    else
        CHECK(umi_document_coordinator_new(documents, "Scratch.c", id, sizeof(id)) == UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = "unsaved source";
    view.dirty = 1;
    view.cursor_offset = 0U;
    view.selection_length = 0U;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.quick-open-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions window_options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &window_options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    Pump();
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *button = Find(root, "studio.editor.quick-open");
    CHECK(GTK_IS_BUTTON(button));
    retainedOwner = g_object_ref(GTK_WINDOW(root));
    openingButton = g_object_ref(button);
    opening.services = services; opening.button = openingButton; opening.other = other;
    const int boundary = strncmp(name, "constructor-", 12U) == 0 || strncmp(name, "present-", 8U) == 0 ||
        strncmp(name, "replacement-", 12U) == 0 || strcmp(name, "title-close") == 0 || strcmp(name, "child-close") == 0;
    if (strncmp(name, "replacement-", 12U) == 0) {
        g_signal_emit_by_name(button, "clicked");
        GtkWindow *oldPicker = g_object_get_data(G_OBJECT(root), "umicom-studio-quick-open-window");
        CHECK(oldPicker != NULL); previousPicker = g_object_ref(oldPicker);
        CHECK(UmiGtk4WatchWindowClosed(previousPicker, G_OBJECT(root), StudioQuickReplacingClosed, &opening) == UMI_STATUS_OK);
    }
    if (boundary) {
        openingWindows = gtk_window_get_toplevels();
        openingHandler = g_signal_connect(openingWindows, "items-changed", G_CALLBACK(StudioQuickOpeningAdded), &opening);
    }
    if (strcmp(name, "keyboard") == 0 || strcmp(name, "capture-key") == 0)
    {
        controllers = gtk_widget_observe_controllers(root);
        int found = 0;
        const char *wanted =
            strcmp(name, "capture-key") == 0 ? "umicom-studio-document-navigation" : "umicom-studio-keyboard";
        for (guint i = 0U; i < g_list_model_get_n_items(controllers); ++i)
        {
            GtkEventController *controller = g_list_model_get_item(controllers, i);
            const char *controller_name = gtk_event_controller_get_name(controller);
            if (controller_name != NULL && strcmp(controller_name, wanted) == 0)
            {
                gboolean handled = FALSE;
                g_signal_emit_by_name(controller, "key-pressed", GDK_KEY_p, 0U, GDK_CONTROL_MASK, &handled);
                found = handled;
            }
            g_object_unref(controller);
        }
        CHECK(found);
    }
    else
        g_signal_emit_by_name(button, "clicked");
    GtkWindow *candidate = g_object_get_data(G_OBJECT(root), "umicom-studio-quick-open-window");
    if (boundary) {
        CHECK(opening.observed == 1U && opening.status == UMI_STATUS_OK && opening.windows->len == 1U);
        CHECK(g_object_get_data(G_OBJECT(root), "studio-quick-open-creating") == NULL);
        if (strstr(name, "reenter") != NULL) {
            CHECK(candidate != NULL && UmiGtk4WindowIsOpen(candidate) && workbench != NULL);
            dialog = g_object_ref(candidate); Pump();
            CHECK(Find(GTK_WIDGET(dialog), "umicom-quick-open-query") != NULL);
        } else {
            for (guint i = 0U; i < opening.windows->len; ++i)
                CHECK(!UmiGtk4WindowIsOpen(g_ptr_array_index(opening.windows, i)));
            if (strstr(name, "workspace") == NULL) CHECK(workbench == NULL);
        }
        goto preserved;
    }
    if (strcmp(name, "no-workspace") == 0)
    {
        CHECK(candidate == NULL);
        goto preserved;
    }
    CHECK(candidate != NULL);
    dialog = g_object_ref(candidate);
    Pump();
    GtkWidget *query = Find(GTK_WIDGET(dialog), "umicom-quick-open-query"),
              *open = Find(GTK_WIDGET(dialog), "umicom-quick-open-open"),
              *refresh = Find(GTK_WIDGET(dialog), "umicom-quick-open-refresh"),
              *list = Find(GTK_WIDGET(dialog), "umicom-quick-open-results");
    CHECK(query != NULL && open != NULL && refresh != NULL && list != NULL && gtk_widget_get_sensitive(open));
    if (strcmp(name, "filter") == 0 || strcmp(name, "empty") == 0)
    {
        gtk_editable_set_text(GTK_EDITABLE(query), strcmp(name, "empty") == 0 ? "missing" : "MAIN.C");
        Pump();
        CHECK(gtk_widget_get_sensitive(open) == (strcmp(name, "empty") != 0));
    }
    if (strcmp(name, "open") == 0 || strcmp(name, "dirty-existing") == 0)
    {
        /* An open picker must still reveal its source after a layout hides the
         * editor. Model selection by itself is insufficient for typing. */
        CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
            "quick-open-hidden-editor", "Hidden editor") == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
        g_signal_emit_by_name(open, "clicked");
        CHECK(!UmiGtk4WindowIsOpen(dialog));
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        if (strcmp(name, "dirty-existing") == 0)
            CHECK(strcmp(active.view_id, id) == 0 && active.dirty);
        else
            CHECK(strcmp(active.view_id, id) != 0);
        GtkWidget *editor = FindEditor(root, active.view_id);
        CHECK(GTK_IS_TEXT_VIEW(editor) && gtk_text_view_get_editable(GTK_TEXT_VIEW(editor)));
        GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(root));
        CHECK(focused == editor || (focused != NULL && gtk_widget_is_ancestor(focused, editor)));
        for (GtkWidget *node = editor; node != root; node = gtk_widget_get_parent(node))
            CHECK(node != NULL && gtk_widget_get_visible(node) && gtk_widget_get_child_visible(node));
    }
    if (strcmp(name, "stale-index") == 0 || strcmp(name, "refresh-index") == 0)
    {
        CHECK(g_file_set_contents(extra, "more", 4, NULL));
        CHECK(umi_file_index_update(umi_studio_services_file_index(services), extra) == UMI_STATUS_OK);
        g_signal_emit_by_name(open, "clicked");
        CHECK(UmiGtk4WindowIsOpen(dialog) && !gtk_widget_get_sensitive(open));
        if (strcmp(name, "refresh-index") == 0)
        {
            g_signal_emit_by_name(refresh, "clicked");
            Pump();
            CHECK(gtk_widget_get_sensitive(open));
        }
    }
    if (strcmp(name, "workspace-switch") == 0 || strcmp(name, "workspace-return") == 0)
    {
        CHECK(umi_studio_workspace_open(services, other, 0, 0) == UMI_STATUS_OK);
        if (strcmp(name, "workspace-return") == 0)
            CHECK(umi_studio_workspace_open(services, project, 0, 0) == UMI_STATUS_OK);
        g_signal_emit_by_name(open, "clicked");
        CHECK(UmiGtk4WindowIsOpen(dialog) && !gtk_widget_get_sensitive(open));
    }
    if (strcmp(name, "retained-control") == 0)
    {
        retained = g_object_ref(query);
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        CHECK(!UmiGtk4WindowIsOpen(dialog));
        gtk_editable_set_text(GTK_EDITABLE(retained), "late query");
        g_signal_emit_by_name(open, "clicked");
        Pump();
    }
    if (strcmp(name, "repeated-open") == 0)
    {
        g_signal_emit_by_name(button, "clicked");
        CHECK(!UmiGtk4WindowIsOpen(dialog));
        GtkWindow *replacement = g_object_get_data(G_OBJECT(root), "umicom-studio-quick-open-window");
        CHECK(replacement != NULL && replacement != dialog && UmiGtk4WindowIsOpen(replacement));
    }
preserved:
    CHECK(UmiUiDocumentViewModelCopyText(views, id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && strcmp(output, text) == 0);
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK && view.dirty);
cleanup:
    opening.observed = 1U; /* Teardown must not repeat the one-shot injected action. */
    if (openingHandler != 0U) g_signal_handler_disconnect(openingWindows, openingHandler);
    for (guint i = 0U; i < opening.windows->len; ++i)
        g_signal_handlers_disconnect_by_data(g_ptr_array_index(opening.windows, i), &opening);
    UmiUiDocumentViewModelFreeText(output);
    g_clear_object(&controllers);
    g_clear_object(&retained);
    if (dialog != NULL)
    {
        gtk_window_destroy(dialog);
        g_clear_object(&dialog);
    }
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
    if (previousPicker != NULL) { gtk_window_destroy(previousPicker); g_object_unref(previousPicker); }
    g_clear_object(&retainedOwner); g_clear_object(&openingButton);
    g_ptr_array_unref(opening.windows);
    if (bootstrap != NULL)
        umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL)
        umi_settings_destroy(settings);
    g_clear_object(&application);
    g_clear_error(&error);
    if (file != NULL)
        (void)g_remove(file);
    if (extra != NULL)
        (void)g_remove(extra);
    if (project != NULL)
        (void)g_rmdir(project);
    if (other != NULL)
        (void)g_rmdir(other);
    if (changed_directory)
        (void)g_chdir(original);
    if (old_data != NULL)
        (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data);
    g_free(file);
    g_free(extra);
    g_free(project);
    g_free(other);
    g_free(directory);
    g_free(original);
    return failed;
}
