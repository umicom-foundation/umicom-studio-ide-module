/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_terminal_draft_gtk4.c
 * PURPOSE: Verify command acceptance preserves newer native entry drafts.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#include "../src/gui/workbench/workbench_window.c"

static UmiStudioGtkWorkbench **host;
static GtkWidget *entry;
static const char *mode;
static unsigned calls;
static int armed;
static char received[128];
static void DraftClose(void)
{
    if (host != NULL && *host != NULL) {
        UmiStudioGtkWorkbench *old = *host; *host = NULL; umi_studio_gtk_workbench_destroy(old);
    }
}
/* Substitute only the isolated registry's command handler. Production GTK
 * dispatch, weak ownership, background admission and entry clearing still run.
 * This fixture never starts a shell or another executable. */
static UmiStatus DraftCommand(void *data, const char *argument, char *message, size_t capacity)
{
    (void)data; (void)message; (void)capacity;
    ++calls; g_strlcpy(received, argument, sizeof received);
    if (strcmp(mode, "dispatch-close") == 0) DraftClose();
    else if (strcmp(mode, "edit") == 0 || strcmp(mode, "edit-return") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(entry), "next command");
        if (strcmp(mode, "edit-return") == 0) gtk_editable_set_text(GTK_EDITABLE(entry), argument);
    } else if (strcmp(mode, "reenter") == 0) runtime_terminal_execute_entry((*host)->runtime_chrome);
    return strcmp(mode, "rejected") == 0 ? UMI_STATUS_UNAVAILABLE : UMI_STATUS_OK;
}
static void DraftChanged(GtkEditable *editable, gpointer data)
{
    (void)data;
    if (!armed || gtk_editable_get_text(editable)[0] != '\0') return;
    armed = 0;
    if (strcmp(mode, "clear-close") == 0) DraftClose();
    else if (strcmp(mode, "clear-edit") == 0 || strcmp(mode, "clear-reenter") == 0) {
        gtk_editable_set_text(editable, "next command");
        if (strcmp(mode, "clear-reenter") == 0) runtime_terminal_execute_entry((*host)->runtime_chrome);
    }
}
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *cases[] = {"accepted", "rejected", "edit", "edit-return", "dispatch-close", "clear-close", "clear-edit", "clear-reenter", "reenter", "retry", "entry-activate", "empty"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(argv[1], cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    mode = argv[1];
    int failed = 0, changed = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "draft-XXXXXX", NULL);
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *run = NULL; gulong handler = 0U;
    CHECK(g_mkdtemp(root) != NULL && g_chdir(root) == 0); changed = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions serviceOptions = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.terminal-draft-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    host = &workbench;
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TERMINAL, NULL) == UMI_STATUS_OK);
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *foundRun = umi_gtk4_automation_find_tagged_widget(window, "studio.terminal.run");
    CHECK(GTK_IS_BUTTON(foundRun) && GTK_IS_ENTRY(chrome->terminal_entry));
    run = g_object_ref(foundRun); entry = g_object_ref(chrome->terminal_entry);
    UmiCommandRegistry *registry = umi_studio_bootstrap_command_registry(bootstrap);
    CHECK(umi_command_registry_unregister(registry, UMI_STUDIO_COMMAND_TERMINAL_EXECUTE) == UMI_STATUS_OK);
    UmiCommandDescriptor descriptor = {0};
    descriptor.structure_size = (uint32_t)sizeof descriptor;
    descriptor.command_id = UMI_STUDIO_COMMAND_TERMINAL_EXECUTE;
    descriptor.title = "Terminal acceptance fixture"; descriptor.category = "Qualification";
    descriptor.description = "Accept without starting a process";
    descriptor.handler = DraftCommand;
    CHECK(umi_command_registry_register(registry, &descriptor) == UMI_STATUS_OK);
    gtk_editable_set_text(GTK_EDITABLE(entry), strcmp(mode, "empty") == 0 ? "" : "original command");
    handler = g_signal_connect(entry, "changed", G_CALLBACK(DraftChanged), NULL); armed = 1;
    if (strcmp(mode, "entry-activate") == 0) g_signal_emit_by_name(entry, "activate");
    else g_signal_emit_by_name(run, "clicked");
    CHECK(calls == (strcmp(mode, "empty") == 0 ? 0U : 1U));
    if (calls != 0U) CHECK(strcmp(received, "original command") == 0);
    if (strcmp(mode, "dispatch-close") == 0 || strcmp(mode, "clear-close") == 0) CHECK(workbench == NULL);
    else {
        CHECK(workbench != NULL);
        const char *expected = strcmp(mode, "edit") == 0 || strcmp(mode, "clear-edit") == 0 || strcmp(mode, "clear-reenter") == 0
            ? "next command" : strcmp(mode, "edit-return") == 0 || strcmp(mode, "rejected") == 0 ? "original command" : "";
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(entry)), expected) == 0);
        CHECK(g_object_get_data(G_OBJECT(entry), "umicom-terminal-dispatch") == NULL);
        UmiTerminalController *controller = RuntimeTerminalController(workbench->runtime_chrome);
        CHECK(!UmiTerminalControllerBackgroundArmed(controller));
        CHECK(!UmiTerminalControllerJobPending(controller));
        CHECK(umi_process_supervisor_stats(umi_terminal_controller_process_supervisor(controller)).jobs == 0U);
        if (strcmp(mode, "retry") == 0) {
            gtk_editable_set_text(GTK_EDITABLE(entry), "second command");
            g_signal_emit_by_name(run, "clicked");
            CHECK(calls == 2U && strcmp(received, "second command") == 0);
            CHECK(gtk_editable_get_text(GTK_EDITABLE(entry))[0] == '\0');
        }
    }
cleanup:
    armed = 0; host = NULL;
    if (handler != 0U) g_signal_handler_disconnect(entry, handler);
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&entry); g_clear_object(&run);
    if (changed) (void)g_chdir(original);
    if (isolated) { if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE); else g_unsetenv("UMICOM_STUDIO_DATA_PATH"); }
    g_free(original); g_free(oldData); g_free(root);
    return failed;
}
