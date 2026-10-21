/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_live_diagnostics_gtk4.c
 * PURPOSE: Exercise Studio live diagnostics against a named draft and workspace authorization.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/path.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/live_diagnostics.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #value);                            \
            failed = 1;                                                                            \
            goto cleanup;                                                                          \
        }                                                                                          \
    } while (0)
static void Drain(void)
{
    for (unsigned i = 0U; i < 30U; ++i)
    {
        while (g_main_context_iteration(NULL, FALSE))
        {
        }
        g_usleep(1000U);
    }
}
static bool Wait(GtkWidget *panel)
{
    for (unsigned i = 0U; i < 10000U; ++i)
    {
        while (g_main_context_iteration(NULL, FALSE))
        {
        }
        if (!UmiGtk4LiveDiagnosticPanelPending(panel))
            return true;
        g_usleep(1000U);
    }
    return false;
}

static bool WaitText(GtkWidget *view, const char *needle)
{
    for (unsigned attempt = 0U; attempt < 12000U; ++attempt)
    {
        while (g_main_context_iteration(NULL, FALSE))
        {
        }
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
        GtkTextIter first, last;
        gtk_text_buffer_get_bounds(buffer, &first, &last);
        char *text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
        bool found = strstr(text, needle) != NULL;
        g_free(text);
        if (found)
            return true;
        g_usleep(1000U);
    }
    return false;
}

/* Construction observers exercise real window lifecycle boundaries without
 * replacing the Framework diagnostic provider or its ownership checks. */
typedef struct DiagnosticOpeningProbe {
    UmiStudioGtkWorkbench **slot;
    UmiStudioServices *services;
    GtkWidget *open;
    GPtrArray *windows;
    const char *mode;
    unsigned observed;
    UmiStatus status;
} DiagnosticOpeningProbe;
static void DiagnosticOpeningAct(DiagnosticOpeningProbe *probe)
{
    if (probe->observed++ != 0U) return;
    if (strstr(probe->mode, "reenter") != NULL) g_signal_emit_by_name(probe->open, "clicked");
    else if (strstr(probe->mode, "revoke") != NULL)
        probe->status = umi_studio_workspace_set_trusted(probe->services, 0);
    else {
        UmiStudioGtkWorkbench *owner = *probe->slot;
        *probe->slot = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    }
}
static void DiagnosticOpeningObserved(GObject *object, GParamSpec *property, gpointer data)
{
    DiagnosticOpeningProbe *probe = data;
    if (probe->observed != 0U) return;
    if (strcmp(property->name, "visible") == 0 && !gtk_widget_get_visible(GTK_WIDGET(object))) return;
    DiagnosticOpeningAct(probe);
}
static void DiagnosticOpeningAdded(GListModel *model, guint position, guint removed, guint added, gpointer data)
{
    (void)removed;
    DiagnosticOpeningProbe *probe = data;
    for (guint i = 0U; i < added; ++i) {
        GtkWindow *window = g_list_model_get_item(model, position + i);
        if (window == NULL) continue;
        g_ptr_array_add(probe->windows, window);
        if (strncmp(probe->mode, "constructor-", 12U) == 0 && probe->observed == 0U)
            DiagnosticOpeningAct(probe);
        else if (strcmp(probe->mode, "title-close") == 0)
            g_signal_connect(window, "notify::title", G_CALLBACK(DiagnosticOpeningObserved), probe);
        else if (strcmp(probe->mode, "child-close") == 0)
            g_signal_connect(window, "notify::child", G_CALLBACK(DiagnosticOpeningObserved), probe);
        else if (strncmp(probe->mode, "present-", 8U) == 0)
            g_signal_connect(window, "notify::visible", G_CALLBACK(DiagnosticOpeningObserved), probe);
    }
}
static GtkWidget *DiagnosticSourceEditor(GtkWidget *root, const char *viewId)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
    if (id != NULL && strcmp(id, viewId) == 0) {
        GtkWidget *editor = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
        if (GTK_IS_TEXT_VIEW(editor)) return editor;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = DiagnosticSourceEditor(child, viewId);
        if (found != NULL) return found;
    }
    return NULL;
}
static int DiagnosticSourceReachable(GtkWidget *editor, GtkWindow *window)
{
    if (editor == NULL || !gtk_text_view_get_editable(GTK_TEXT_VIEW(editor))) return 0;
    for (GtkWidget *node = editor; node != GTK_WIDGET(window); node = gtk_widget_get_parent(node))
        if (node == NULL || !gtk_widget_get_visible(node) || !gtk_widget_get_child_visible(node)) return 0;
    GtkWidget *focus = gtk_window_get_focus(window);
    return focus == editor || (focus != NULL && gtk_widget_is_ancestor(focus, editor));
}
static void DiagnosticFocusClose(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    UmiStudioGtkWorkbench **slot = data;
    if (*slot != NULL && GTK_IS_TEXT_VIEW(gtk_window_get_focus(GTK_WINDOW(object)))) {
        UmiStudioGtkWorkbench *owner = *slot; *slot = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    }
}

int main(int argc, char **argv)
{
    if (argc != 4)
        return 2;
    const char *mode = argv[1];
    const char *cases[] = {"update",       "retained", "revoke",   "project-close",
                           "parent-close", "denied",   "navigate", "navigate-stale",
        "navigate-hidden", "navigate-focus-close", "navigate-revoke",
        "constructor-close", "constructor-reenter", "constructor-revoke",
        "title-close", "child-close", "present-close", "present-reenter"};
    bool known = false;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(mode, cases[i]) == 0)
            known = true;
    if (!known)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool moved = false;
    char *original = g_get_current_dir(),
         *previous_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
/* Keep language-server evidence in the explicitly supplied build-tree qualification directory; preserve the former system-temporary choice for review. */
#if 0
    char *directory = g_dir_make_tmp("umicom-studio-live-diagnostics-XXXXXX", NULL);
#endif

    char *directory = g_build_filename(argv[3], "live-diagnostics-XXXXXX", NULL);
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *open = NULL, *panel = NULL;
    GtkWindow *dialog = NULL, *owner = NULL;
    GListModel *windows = gtk_window_get_toplevels();
    gulong openingHandler = 0U, focusHandler = 0U;
    DiagnosticOpeningProbe probe = {&workbench, NULL, NULL, g_ptr_array_new_with_free_func(g_object_unref), mode, 0U, UMI_STATUS_OK};
    CHECK(directory != NULL && g_mkdtemp(directory) != NULL && original != NULL && g_chdir(directory) == 0);
    moved = true;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions service_options = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    CHECK(umi_studio_workspace_open(services, directory, strcmp(mode, "denied") != 0, 0) ==
          UMI_STATUS_OK);
    application =
        gtk_application_new("org.umicom.studio.live-diagnostics-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR,
                                                          NULL) == UMI_STATUS_OK);
    CHECK(g_file_set_contents("main.c", "initial source", -1, NULL));
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
    char source_path[UMI_PATH_CAPACITY], view_id[UMI_UI_ID_CAPACITY];
    CHECK(umi_path_join(directory, "main.c", source_path, sizeof source_path) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_open(documents, source_path, view_id, sizeof view_id) ==
          UMI_STATUS_OK);
    owner = g_object_ref(umi_studio_gtk_workbench_window(workbench));
    gtk_window_present(owner);
    Drain();
    open =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(owner), "studio.editor.live-diagnostics");
    CHECK(open != NULL);
    g_object_ref(open);
    if (strcmp(mode, "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    probe.services = services; probe.open = open;
    openingHandler = g_signal_connect(windows, "items-changed", G_CALLBACK(DiagnosticOpeningAdded), &probe);
    guint before = g_list_model_get_n_items(windows);
    g_signal_emit_by_name(open, "clicked");
    if (strcmp(mode, "constructor-close") == 0 || strcmp(mode, "constructor-revoke") == 0 ||
        strcmp(mode, "title-close") == 0 || strcmp(mode, "child-close") == 0 || strcmp(mode, "present-close") == 0) {
        CHECK(probe.observed == 1U && probe.status == UMI_STATUS_OK);
        CHECK(strcmp(mode, "constructor-revoke") == 0 ? workbench != NULL : workbench == NULL);
        for (guint i = 0U; i < probe.windows->len; ++i)
            CHECK(!UmiGtk4WindowIsOpen(g_ptr_array_index(probe.windows, i)));
        goto cleanup;
    }
    if (strcmp(mode, "denied") == 0 || strcmp(mode, "retained") == 0)
    {
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
        goto cleanup;
    }
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i)
    {
        GtkWindow *candidate = g_list_model_get_item(windows, i);
        GtkWidget *found =
            umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate), "live.diagnostics.panel");
        if (found != NULL)
        {
            dialog = candidate;
            panel = g_object_ref(found);
            break;
        }
        g_object_unref(candidate);
    }
    CHECK(dialog != NULL && panel != NULL);
    Drain();
    if (strstr(mode, "reenter") != NULL) {
        CHECK(probe.observed == 1U && probe.windows->len == 1U && !UmiGtk4LiveDiagnosticPanelPending(panel));
        goto cleanup;
    }

    GtkWidget *start = umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.start");
    GtkWidget *stop = umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.stop");
    GtkWidget *program = umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.program");
    GtkWidget *arguments =
        umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.arguments");
    GtkWidget *output = umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.output");
    CHECK(start != NULL && stop != NULL && program != NULL && arguments != NULL && output != NULL);
    gtk_editable_set_text(GTK_EDITABLE(program), argv[2]);
    gtk_editable_set_text(GTK_EDITABLE(arguments), "");
    CHECK(!UmiGtk4LiveDiagnosticPanelPending(panel));
    g_signal_emit_by_name(start, "clicked");
    CHECK(UmiGtk4LiveDiagnosticPanelPending(panel));
    CHECK(WaitText(output, "initial source"));
    if (strncmp(mode, "navigate", 8U) == 0)
    {
        GtkWidget *jump = umi_gtk4_automation_find_tagged_widget(panel, "live.diagnostics.open");
        CHECK(jump != NULL);
        if (strcmp(mode, "navigate-stale") == 0)
        {
            size_t replaced = 0U;
            CHECK(umi_document_coordinator_replace(documents, "initial source", "edited source",
                                                   &replaced) == UMI_STATUS_OK);
        }
        UmiDocumentWorkingCopySnapshot captured;
        CHECK(umi_document_coordinator_active_snapshot(documents, &captured) == UMI_STATUS_OK);
        char other[UMI_UI_ID_CAPACITY];
        CHECK(umi_document_coordinator_new(documents, "other.c", other, sizeof other) ==
              UMI_STATUS_OK);
        UmiDocumentWorkingCopySnapshot before_jump, after_jump;
        CHECK(umi_document_coordinator_active_snapshot(documents, &before_jump) == UMI_STATUS_OK);
        bool stale = strcmp(mode, "navigate-stale") == 0, revoked = strcmp(mode, "navigate-revoke") == 0;
        bool focusClose = strcmp(mode, "navigate-focus-close") == 0;
        if (strcmp(mode, "navigate-hidden") == 0 || focusClose) {
            CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench, "diagnostic-hidden", "Diagnostics") == UMI_STATUS_OK);
            CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
        }
        if (revoked) CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
        if (focusClose) focusHandler = g_signal_connect(owner, "notify::focus-widget", G_CALLBACK(DiagnosticFocusClose), &workbench);
        g_signal_emit_by_name(jump, "clicked");
        if (focusClose) { CHECK(workbench == NULL); CHECK(Wait(panel)); goto cleanup; }
        CHECK(umi_document_coordinator_active_snapshot(documents, &after_jump) == UMI_STATUS_OK);
        CHECK(after_jump.document_id ==
              (!stale && !revoked ? captured.document_id : before_jump.document_id));
        if (!stale && !revoked) CHECK(DiagnosticSourceReachable(DiagnosticSourceEditor(GTK_WIDGET(owner), after_jump.view_id), owner));
        g_signal_emit_by_name(stop, "clicked");
        CHECK(Wait(panel));
        goto cleanup;
    }

    if (strcmp(mode, "revoke") == 0)
        CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
    else if (strcmp(mode, "project-close") == 0)
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
    else if (strcmp(mode, "parent-close") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
    }
    else
    {
        CHECK(strcmp(mode, "update") == 0);
        size_t offset = 0U;
        CHECK(umi_document_coordinator_replace(documents, "initial source", "changed source",
                                               &offset) == UMI_STATUS_OK);
        CHECK(WaitText(output, "changed source"));
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK &&
              active.dirty);
        char *saved = NULL;
        CHECK(g_file_get_contents("main.c", &saved, NULL, NULL));
        bool unchanged = strcmp(saved, "initial source") == 0;
        g_free(saved);
        CHECK(unchanged);
        g_signal_emit_by_name(stop, "clicked");
    }
    CHECK(Wait(panel));
cleanup:
    if (openingHandler != 0U) g_signal_handler_disconnect(windows, openingHandler);
    if (focusHandler != 0U) g_signal_handler_disconnect(owner, focusHandler);
    for (guint i = 0U; i < probe.windows->len; ++i)
        g_signal_handlers_disconnect_by_data(g_ptr_array_index(probe.windows, i), &probe);
    if (dialog != NULL)
        gtk_window_destroy(dialog);
    umi_studio_gtk_workbench_destroy(workbench);
    if (panel != NULL)
        (void)Wait(panel);
    g_clear_object(&dialog);
    g_clear_object(&owner);
    g_ptr_array_unref(probe.windows);
    g_clear_object(&panel);
    g_clear_object(&open);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved)
    {
        if (g_chdir(original) != 0)
            failed = 1;
    }
    if (previous_data != NULL)
        g_setenv("UMICOM_STUDIO_DATA_PATH", previous_data, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    if (directory != NULL)
        printf("Isolated live-diagnostics fixture retained: %s\n", directory);
    g_free(directory);
    g_free(original);
    g_free(previous_data);
    return failed;
}
