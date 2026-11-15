/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_branch_menu_gtk4.c
 *
 * PURPOSE:
 *   Check native Branches menu selection, project identity and absence of repository operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Include the production host so checkout tests use its actual private row factory,
 * weak binding and registry wrapper. No alternate callback is substituted. */
#if 0
#include "workbench_window.h"
#endif
#include "../src/gui/workbench/workbench_window.c"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/source_control.h"
#include "umicom/ui/gtk4/automation.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)

typedef struct BranchObserver {
    UmiStudioGtkWorkbench **owner;
    UmiStudioServices *services;
    const char *mode;
    unsigned calls;
    UmiStatus nested;
} BranchObserver;

static void PageChanged(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    BranchObserver *observer = data;
    if (observer->calls++ != 0U || *observer->owner == NULL) return;
    if (strcmp(observer->mode, "observer-close") == 0) {
        UmiStudioGtkWorkbench *owner = *observer->owner;
        *observer->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (strcmp(observer->mode, "observer-project") == 0)
        observer->nested = umi_studio_workspace_close(observer->services);
    else if (strcmp(observer->mode, "observer-redirect") == 0)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(object), 0);
    else if (strcmp(observer->mode, "observer-remove") == 0)
        gtk_notebook_remove_page(GTK_NOTEBOOK(object),
            gtk_notebook_get_current_page(GTK_NOTEBOOK(object)));
    else observer->nested = umi_studio_gtk_workbench_execute_command(
        *observer->owner, "source-control.branches", NULL);
}

static GtkWidget *FindCommand(GtkWidget *root)
{
    const char *id = g_object_get_data(G_OBJECT(root), "umicom-command-id");
    if (GTK_IS_BUTTON(root) && id != NULL && strcmp(id, "source-control.branches") == 0 &&
        g_object_get_data(G_OBJECT(root), "umicom-contribution-id") != NULL) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = FindCommand(child);
        if (found != NULL) return found;
    }
    return NULL;
}


typedef struct CheckoutProbe {
    UmiStudioGtkWorkbench **owner;
    unsigned calls;
    int close_owner, enabled;
    char argument[UMI_VCS_NAME_CAPACITY];
} CheckoutProbe;

static int CheckoutEnabled(void *data, const char *argument)
{
    (void)argument;
    return ((CheckoutProbe *)data)->enabled;
}

/* Only the final repository command is replaced. The real native callback,
 * current-list/project checks and registered dispatcher all remain active. */
static UmiStatus CheckoutObserved(void *data, const char *argument,
    char *message, size_t capacity)
{
    CheckoutProbe *probe = data;
    ++probe->calls;
    g_strlcpy(probe->argument, argument != NULL ? argument : "", sizeof probe->argument);
    if (probe->close_owner && *probe->owner != NULL) {
        UmiStudioGtkWorkbench *owner = *probe->owner;
        *probe->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    }
    if (message != NULL && capacity != 0U) g_strlcpy(message, "Synthetic checkout observed", capacity);
    return UMI_STATUS_OK;
}

static int ExerciseCheckout(UmiStudioGtkWorkbench **owner, UmiStudioUi *ui,
    UmiStudioSourceControlService *vcs, const char *mode)
{
    int failed = 0;
    GtkWidget *row = NULL, *button = NULL;
    CheckoutProbe probe = {owner, 0U, strcmp(mode, "checkout-callback-close") == 0,
        strcmp(mode, "checkout-registry-denied") != 0, {0}};
    UmiCommandRegistry *commands = umi_ui_workbench_commands(umi_studio_ui_workbench(ui));
    CHECK(umi_command_registry_unregister(commands, UMI_STUDIO_COMMAND_VCS_BRANCH_CHECKOUT) == UMI_STATUS_OK);
    UmiCommandDescriptor descriptor = {0};
    descriptor.structure_size = sizeof descriptor;
    descriptor.command_id = UMI_STUDIO_COMMAND_VCS_BRANCH_CHECKOUT;
    descriptor.title = "Synthetic checkout observer";
    descriptor.category = "Test";
    descriptor.handler = CheckoutObserved;
    descriptor.enabled = CheckoutEnabled;
    descriptor.user_data = &probe;
    CHECK(umi_command_registry_register(commands, &descriptor) == UMI_STATUS_OK);
    UmiStudioGtkRuntimeChrome *runtime = (*owner)->runtime_chrome;
    UmiVcsBranch branch = {0};
    g_strlcpy(branch.name, "feature-review", sizeof branch.name);
    branch.current = strcmp(mode, "checkout-current") == 0;
    button = RuntimeCreateBranchCheckout(runtime, &branch);
    CHECK(GTK_IS_BUTTON(button));
    g_object_ref_sink(button);
    row = g_object_ref_sink(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_append(GTK_BOX(row), button);
    gtk_list_box_append(GTK_LIST_BOX(runtime->source_control_branches_list), row);
    CHECK(gtk_widget_get_sensitive(button) == !branch.current);
    if (strcmp(mode, "checkout-disabled") == 0) gtk_widget_set_sensitive(button, FALSE);
    if (strcmp(mode, "checkout-orphan") == 0) {
        GtkWidget *wrapper = gtk_widget_get_parent(row);
        CHECK(GTK_IS_LIST_BOX_ROW(wrapper));
        gtk_list_box_remove(GTK_LIST_BOX(runtime->source_control_branches_list), wrapper);
    }
    if (strcmp(mode, "checkout-replaced") == 0)
        runtime_source_control_refresh_details(runtime);
    if (strcmp(mode, "checkout-project-close") == 0)
        CHECK(umi_studio_workspace_close(umi_studio_ui_services(ui)) == UMI_STATUS_OK);
    if (strcmp(mode, "checkout-project-revision") == 0)
        CHECK(umi_studio_workspace_set_trusted(umi_studio_ui_services(ui), 1) == UMI_STATUS_OK);
    if (strcmp(mode, "checkout-retired") == 0) {
        UmiStudioGtkWorkbench *retired = *owner; *owner = NULL;
        umi_studio_gtk_workbench_destroy(retired);
    }
    /* Signal emission deliberately bypasses GTK's normal disabled hit testing. */
    g_signal_emit_by_name(button, "clicked");
    unsigned expected = strcmp(mode, "checkout-valid") == 0 || probe.close_owner ? 1U : 0U;
    CHECK(probe.calls == expected);
    if (expected != 0U) CHECK(strcmp(probe.argument, "feature-review") == 0);
    if (probe.close_owner || strcmp(mode, "checkout-retired") == 0) CHECK(*owner == NULL);
    UmiStudioSourceControlSnapshot snapshot = {0};
    CHECK(umi_studio_source_control_service_snapshot(vcs, &snapshot) == UMI_STATUS_OK);
    CHECK(snapshot.operations == 0U && !snapshot.available);
cleanup:
    /* The callback data is stack-owned: remove it before this scope ends. */
    (void)umi_command_registry_unregister(commands, UMI_STUDIO_COMMAND_VCS_BRANCH_CHECKOUT);
    g_clear_object(&button); g_clear_object(&row);
    return failed;
}

/* An untrusted temporary project and discovery-disabled services ensure that
 * viewing Branches cannot be mistaken for a successful Git operation. */
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *mode = argv[1];
    const char *cases[] = {"menu", "direct", "repeat", "argument", "retained",
        "no-workspace", "closed-workspace", "observer-close", "observer-project",
        "observer-reenter", "observer-redirect", "observer-remove",
        "checkout-valid", "checkout-current", "checkout-disabled", "checkout-orphan",
        "checkout-replaced", "checkout-project-close", "checkout-project-revision",
        "checkout-retired", "checkout-callback-close", "checkout-registry-denied"};
    int known = 0, failed = 0, moved = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
        if (strcmp(mode, cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *retained = NULL, *notebook = NULL, *branches = NULL, *entry = NULL;
    gulong observer_id = 0U;
    BranchObserver observer = {&workbench, NULL, mode, 0U, UMI_STATUS_OK};
    base = g_dir_make_tmp("umicom-branch-menu-XXXXXX", NULL);
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
    observer.services = services;
    if (strcmp(mode, "no-workspace") != 0)
        CHECK(umi_studio_workspace_open(services, base, 0, 0) == UMI_STATUS_OK);
    UmiStudioSourceControlService *vcs = umi_studio_services_source_control(services);
    application = gtk_application_new("org.umicom.studio.branch-menu-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_SOURCE_CONTROL, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    if (strncmp(mode, "checkout-", 9U) == 0) {
        failed = ExerciseCheckout(&workbench, ui, vcs, mode);
        goto cleanup;
    }
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *row = FindCommand(window);
    CHECK(GTK_IS_BUTTON(row));
    retained = g_object_ref(row);
    GtkWidget *found = umi_gtk4_automation_find_tagged_widget(window, "studio.source-control.notebook");
    CHECK(GTK_IS_NOTEBOOK(found)); notebook = g_object_ref(found);
    found = umi_gtk4_automation_find_tagged_widget(window, "studio.source-control.branches");
    CHECK(GTK_IS_LIST_BOX(found)); branches = g_object_ref(found);
    found = umi_gtk4_automation_find_tagged_widget(window, "studio.source-control.commit-message");
    CHECK(GTK_IS_EDITABLE(found)); entry = g_object_ref(found);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
    gtk_editable_set_text(GTK_EDITABLE(entry), "unsubmitted message");
    UmiStudioSourceControlSnapshot before = {0}, after = {0};
    CHECK(umi_studio_source_control_service_snapshot(vcs, &before) == UMI_STATUS_OK);
    CHECK(!before.available && before.operations == 0U);
    if (strcmp(mode, "retained") == 0) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        goto inspect;
    }
    if (strcmp(mode, "closed-workspace") == 0) {
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_source_control_service_snapshot(vcs, &before) == UMI_STATUS_OK);
    }
    if (strcmp(mode, "no-workspace") == 0 || strcmp(mode, "closed-workspace") == 0) {
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(!gtk_widget_get_sensitive(row));
        /* Public dispatch refuses a disabled menu before invoking its route. */
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,
            "source-control.branches", NULL) == UMI_STATUS_PERMISSION_DENIED);
        CHECK(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == 0);
        goto inspect;
    }
    CHECK(gtk_widget_get_sensitive(row));
    if (strcmp(mode, "argument") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,
            "source-control.branches", "main") == UMI_STATUS_INVALID_ARGUMENT);
        CHECK(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == 0);
        goto inspect;
    }
    if (strncmp(mode, "observer-", 9U) == 0)
        observer_id = g_signal_connect(notebook, "notify::page", G_CALLBACK(PageChanged), &observer);
    if (strcmp(mode, "menu") == 0) g_signal_emit_by_name(row, "clicked");
    else {
        UmiStatus status = umi_studio_gtk_workbench_execute_command(workbench, "source-control.branches", NULL);
        int cancelled = observer_id != 0U && strcmp(mode, "observer-reenter") != 0;
        CHECK(status == (cancelled ? UMI_STATUS_CANCELLED : UMI_STATUS_OK));
    }
    if (observer_id != 0U) {
        CHECK(observer.calls != 0U);
        if (strcmp(mode, "observer-reenter") == 0) CHECK(observer.nested == UMI_STATUS_PERMISSION_DENIED);
        if (strcmp(mode, "observer-project") == 0) CHECK(observer.nested == UMI_STATUS_OK);
        if (strcmp(mode, "observer-close") == 0) CHECK(workbench == NULL);
    }
    if (strcmp(mode, "repeat") == 0)
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "source-control.branches", NULL) == UMI_STATUS_OK);
    if (observer_id == 0U || strcmp(mode, "observer-reenter") == 0) {
        GtkWidget *page = gtk_widget_get_ancestor(branches, GTK_TYPE_SCROLLED_WINDOW);
        int expected = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), page);
        CHECK(expected >= 0 && gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == expected);
    }
inspect:
    CHECK(umi_studio_source_control_service_snapshot(vcs, &after) == UMI_STATUS_OK);
    CHECK(!after.available && after.operations == 0U && after.changes == before.changes);
    if (strcmp(mode, "observer-project") != 0) CHECK(after.revision == before.revision);
    CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(entry)), "unsubmitted message") == 0);
    CHECK(umi_document_coordinator_count(umi_studio_ui_documents(ui)) == 0U);
cleanup:
    if (observer_id != 0U) g_signal_handler_disconnect(notebook, observer_id);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained); g_clear_object(&notebook);
    g_clear_object(&branches); g_clear_object(&entry);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(base); g_free(original);
    return failed;
}
