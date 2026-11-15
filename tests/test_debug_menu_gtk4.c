/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_menu_gtk4.c
 *
 * PURPOSE:
 *   Check debugger menu reveal, retained controls and absence of implicit process actions.
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
#include "umicom/studio/debugger.h"
#include "umicom/studio/build.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); \
    failed = 1; goto cleanup; } } while (0)

typedef struct RevealObserver {
    UmiStudioGtkWorkbench **owner;
    const char *command;
    unsigned calls;
    int close;
    UmiStatus nested_status;
    const char *mode;
} RevealObserver;

static void Revealed(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)object; (void)spec;
    RevealObserver *observer = data;
    if (observer->calls++ != 0U || *observer->owner == NULL) return;
    if (observer->close) {
        UmiStudioGtkWorkbench *owner = *observer->owner;
        *observer->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (strcmp(observer->mode, "observer-redirect") == 0)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(object), 0);
    else if (strcmp(observer->mode, "observer-remove") == 0)
        gtk_notebook_remove_page(GTK_NOTEBOOK(object),
            gtk_notebook_get_current_page(GTK_NOTEBOOK(object)));
    else if (strcmp(observer->mode, "observer-hide") == 0)
        gtk_widget_set_visible(gtk_notebook_get_nth_page(GTK_NOTEBOOK(object),
            gtk_notebook_get_current_page(GTK_NOTEBOOK(object))), FALSE);
    else if (strcmp(observer->mode, "observer-reorder") == 0)
        gtk_notebook_reorder_child(GTK_NOTEBOOK(object),
            gtk_notebook_get_nth_page(GTK_NOTEBOOK(object),
                gtk_notebook_get_current_page(GTK_NOTEBOOK(object))), 0);
    else observer->nested_status = umi_studio_gtk_workbench_execute_command(
        *observer->owner, observer->command, NULL);
}

static GtkWidget *FindCommand(GtkWidget *root, const char *command)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, command) == 0 &&
        g_object_get_data(G_OBJECT(root), "umicom-contribution-id") != NULL) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindCommand(child, command);
        if (found != NULL) return found;
    }
    return NULL;
}

/* No process fixture is supplied. These menu actions are navigation only;
 * native attach, adapter requests and breakpoint mutation have separate tests. */
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *command = argv[1], *mode = argv[2];
    int attach = strcmp(command, "debug.attach") == 0;
    if (!attach && strcmp(command, "debug.breakpoints") != 0) return 2;
    const char *cases[] = {"menu", "direct", "repeat", "argument", "retained",
        "observer-close", "observer-reenter", "observer-redirect", "observer-remove",
        "observer-hide", "observer-reorder"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(mode, cases[i]) == 0) known = 1;
    if (!known || (attach && (strcmp(mode, "observer-redirect") == 0 ||
        strcmp(mode, "observer-remove") == 0 || strcmp(mode, "observer-hide") == 0 ||
        strcmp(mode, "observer-reorder") == 0))) return 2;
    if (!gtk_init_check()) return 77;

    int failed = 0, moved = 0;
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *retained = NULL, *observed = NULL, *retained_target = NULL;
    gulong observer_id = 0U;
    RevealObserver observer = {&workbench, command, 0U, 0, UMI_STATUS_OK, mode};
    base = g_dir_make_tmp("umicom-debug-menu-XXXXXX", NULL);
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
    UmiStudioServices *services = umi_studio_ui_services(ui);
    UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
    UmiStudioBuildService *build = umi_studio_services_build(services);
    application = gtk_application_new("org.umicom.studio.debug-menu-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DEBUG, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *run_menu = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.run-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(run_menu));
    GtkWidget *row = FindCommand(GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(run_menu))), command);
    CHECK(GTK_IS_BUTTON(row) && gtk_widget_get_sensitive(row));
    retained = g_object_ref(row);
    GtkWidget *form = umi_gtk4_automation_find_tagged_widget(window, "studio.debug.attach.form");
    GtkWidget *pid = umi_gtk4_automation_find_tagged_widget(window, "studio.debug.attach.pid");
    GtkWidget *program = umi_gtk4_automation_find_tagged_widget(window, "studio.debug.attach.program");
    GtkWidget *notebook = umi_gtk4_automation_find_tagged_widget(window, "studio.debug.details");
    GtkWidget *breakpoints = umi_gtk4_automation_find_tagged_widget(window, "studio.debug.breakpoints");
    CHECK(GTK_IS_EXPANDER(form) && GTK_IS_EDITABLE(pid) && GTK_IS_EDITABLE(program));
    CHECK(GTK_IS_NOTEBOOK(notebook) && GTK_IS_LIST_BOX(breakpoints));
    retained_target = g_object_ref(breakpoints);
    gtk_expander_set_expanded(GTK_EXPANDER(form), FALSE);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
    gtk_editable_set_text(GTK_EDITABLE(pid), "4321");
    gtk_editable_set_text(GTK_EDITABLE(program), "unsubmitted symbols path");
    CHECK(!UmiStudioDebuggerNativeBusy(debugger) && !UmiStudioBuildBusy(build));
    if (strcmp(mode, "retained") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(!UmiStudioDebuggerNativeBusy(debugger) && !UmiStudioBuildBusy(build));
        goto cleanup;
    }
    if (strcmp(mode, "argument") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, "4321") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(!gtk_expander_get_expanded(GTK_EXPANDER(form)));
        CHECK(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == 0);
        goto inspect;
    }
    if (strncmp(mode, "observer-", 9U) == 0) {
        observed = g_object_ref(attach ? form : notebook);
        observer.close = strcmp(mode, "observer-close") == 0;
        observer_id = g_signal_connect(observed, attach ? "notify::expanded" : "notify::page",
            G_CALLBACK(Revealed), &observer);
    }
    if (strcmp(mode, "menu") == 0) g_signal_emit_by_name(row, "clicked");
    else {
        UmiStatus status = umi_studio_gtk_workbench_execute_command(workbench, command, NULL);
/* Removed, hidden or redirected notebook pages now report abandonment as well.
 * Retain the former owner-only expectation for review. */
#if 0
        CHECK(status == (observer.close ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
#endif
        int abandoned_page = strcmp(mode, "observer-redirect") == 0 ||
            strcmp(mode, "observer-remove") == 0 || strcmp(mode, "observer-hide") == 0;
        CHECK(status == (observer.close || abandoned_page ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
    }
    if (observer_id != 0U) {
        CHECK(observer.calls != 0U);
/* Only reentry sends a second command; other observers alter the notebook. */
#if 0
        if (!observer.close) CHECK(observer.nested_status != UMI_STATUS_OK);
#endif
        if (strcmp(mode, "observer-reenter") == 0) CHECK(observer.nested_status != UMI_STATUS_OK);
    }
    if (observer.close) {
        CHECK(workbench == NULL && !UmiStudioDebuggerNativeBusy(debugger));
        goto cleanup;
    }
    if (strcmp(mode, "observer-redirect") == 0 || strcmp(mode, "observer-remove") == 0 ||
        strcmp(mode, "observer-hide") == 0) goto inspect;
    if (strcmp(mode, "repeat") == 0)
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, command, NULL) == UMI_STATUS_OK);
    if (attach) CHECK(gtk_expander_get_expanded(GTK_EXPANDER(form)));
    else {
        GtkWidget *page = gtk_widget_get_ancestor(breakpoints, GTK_TYPE_SCROLLED_WINDOW);
        int expected = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), page);
        CHECK(expected >= 0 && gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == expected);
    }
inspect:
    CHECK(!UmiStudioDebuggerNativeBusy(debugger) && !UmiStudioBuildBusy(build));
    CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(pid)), "4321") == 0);
    CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(program)), "unsubmitted symbols path") == 0);
    CHECK(umi_document_coordinator_count(umi_studio_ui_documents(ui)) == 0U);
cleanup:
    if (observer_id != 0U) g_signal_handler_disconnect(observed, observer_id);
    g_clear_object(&observed);
    g_clear_object(&retained_target);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original);
    return failed;
}
