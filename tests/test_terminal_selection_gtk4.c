/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_terminal_selection_gtk4.c
 * PURPOSE: Exercise terminal selection and native callbacks without launching tools.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/terminal_ui/selection.h"
#include "umicom/ui/gtk4/automation.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

static UmiStudioGtkWorkbench **testHost;
static UmiTerminalController *testController;
static GtkWidget *testEntry, *testBrowse;
static const char *testMode;
/* Use a fixture-specific counter so included production callback tables do not shadow test globals. The previous statement is retained for review. */
#if 0
static int armed, callbacks;
#endif
static int armed, terminalCallbacks;
static unsigned starts;
static GTask *pending;
static GAsyncReadyCallback answer;
static gpointer answerContext;
static GFile *seed;
static void TerminalRefreshHost(void);
static void CloseHost(void)
{
    if (testHost != NULL && *testHost != NULL) {
        UmiStudioGtkWorkbench *old = *testHost;
        *testHost = NULL;
        umi_studio_gtk_workbench_destroy(old);
    }
}
static void Mutate(void)
{
    if (!armed) return;
/* Count only this fixture's terminal callbacks. The previous statement is retained for review. */
#if 0
    armed = 0; ++callbacks;
#endif
    armed = 0; ++terminalCallbacks;
    if (strstr(testMode, "close") != NULL) CloseHost();
    else if (strstr(testMode, "reenter") != NULL) {
        if (strncmp(testMode, "render-", 7U) == 0) TerminalRefreshHost();
        else g_signal_emit_by_name(testBrowse, "clicked");
    } else if (strstr(testMode, "switch") != NULL)
        (void)umi_terminal_controller_activate(testController, "chooser.other");
    else if (strstr(testMode, "edit") != NULL)
        gtk_editable_set_text(GTK_EDITABLE(testEntry), "changed draft");
}
static void PropertyChanged(GObject *object, GParamSpec *property, gpointer context)
{ (void)object; (void)property; (void)context; Mutate(); }
static void ContentChanged(GObject *object, gpointer context)
{ (void)object; (void)context; Mutate(); }
static GtkFileDialog *NewDialog(void)
{
    GtkFileDialog *dialog = gtk_file_dialog_new();
    if (armed && strncmp(testMode, "create-", 7U) == 0) Mutate();
    else if (armed && strncmp(testMode, "title-", 6U) == 0)
        g_signal_connect(dialog, "notify::title", G_CALLBACK(PropertyChanged), NULL);
    else if (armed && strncmp(testMode, "seed-", 5U) == 0)
        g_signal_connect(dialog, "notify::initial-folder", G_CALLBACK(PropertyChanged), NULL);
    return dialog;
}
static void Reply(GFile *file)
{
    GTask *task = pending;
    GAsyncReadyCallback callback = answer;
    gpointer context = answerContext;
    pending = NULL; answer = NULL; answerContext = NULL;
    if (file != NULL) g_task_return_pointer(task, file, g_object_unref);
    else g_task_return_new_error(task, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED, "Dismissed");
    callback(g_task_get_source_object(task), G_ASYNC_RESULT(task), context);
    g_object_unref(task);
}
static void Present(GtkFileDialog *dialog, GtkWindow *parent, GCancellable *cancel,
    GAsyncReadyCallback callback, gpointer context)
{
    (void)parent;
    ++starts;
    g_set_object(&seed, gtk_file_dialog_get_initial_folder(dialog));
    pending = g_task_new(dialog, cancel, NULL, NULL);
    g_task_set_check_cancellable(pending, FALSE);
    answer = callback; answerContext = context;
    if (strcmp(testMode, "inline-reply") == 0) Reply(g_object_ref(seed));
    else if (strcmp(testMode, "present-close") == 0) { CloseHost(); Reply(NULL); }
}
static GFile *Finish(GtkFileDialog *dialog, GAsyncResult *result, GError **error)
{ (void)dialog; return g_task_propagate_pointer(G_TASK(result), error); }
#define gtk_file_dialog_new NewDialog
#define gtk_file_dialog_select_folder Present
#define gtk_file_dialog_select_folder_finish Finish
#include "../src/gui/workbench/workbench_window.c"
#undef gtk_file_dialog_new
#undef gtk_file_dialog_select_folder
#undef gtk_file_dialog_select_folder_finish
static void TerminalRefreshHost(void)
{
    if (testHost != NULL && *testHost != NULL)
        runtime_terminal_refresh_transcript((*testHost)->runtime_chrome);
}
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "%d: %s\n", __LINE__, #value); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    testMode = argv[1];
    const char *cases[] = {"session-seed", "draft-seed", "relative-seed", "empty-seed", "cancel", "inline-reply", "present-close", "create-close", "create-edit", "create-reenter", "title-close", "title-edit", "title-switch", "title-reenter", "seed-close", "seed-edit", "seed-switch", "late-close", "late-edit", "late-edit-return", "late-switch-return", "late-directory", "late-reparent", "reply-close", "render-folder-close", "render-output-close", "render-summary-close", "render-run-close", "render-stop-close", "render-switch", "render-reenter", "render-finished-session"};
    int known = 0;
    for (size_t index = 0U; index < sizeof cases / sizeof cases[0]; ++index)
        if (strcmp(testMode, cases[index]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, changedDirectory = 0, isolated = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "terminal-XXXXXX", NULL);
    char *other = NULL, *draft = NULL, *text = NULL, *cwdAfter = NULL;
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWindow *otherWindow = NULL;
    GtkWidget *view = NULL, *summary = NULL, *run = NULL, *stop = NULL;
    GFile *expected = NULL;
    CHECK(g_mkdtemp(root) != NULL && g_chdir(root) == 0); changedDirectory = 1;
    other = g_build_filename(root, "other folder", NULL);
    draft = g_build_filename(root, "draft folder", NULL);
    CHECK(g_mkdir_with_parents(other, 0700) == 0 && g_mkdir_with_parents(draft, 0700) == 0);
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
    application = gtk_application_new("org.umicom.studio.terminal-selection-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TERMINAL, NULL) == UMI_STATUS_OK);
    testHost = &workbench;
    testController = umi_studio_services_terminal_controller(umi_studio_bootstrap_services(bootstrap));
    UmiTerminalSelection first;
    CHECK(UmiTerminalSelectionCapture(testController, &first) == UMI_STATUS_OK);
    char firstId[UMI_TERMINAL_ID_CAPACITY];
    memcpy(firstId, first.session.session_id, sizeof firstId);
    CHECK(umi_terminal_session_set_working_directory(umi_terminal_controller_active_session(testController), root) == UMI_STATUS_OK);
    CHECK(umi_terminal_controller_open(testController, "", "chooser.other", "Other session", other) == UMI_STATUS_OK);
    CHECK(umi_terminal_controller_activate(testController, firstId) == UMI_STATUS_OK);
    TerminalRefreshHost();
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *foundEntry = umi_gtk4_automation_find_tagged_widget(window, "studio.terminal.folder");
    GtkWidget *foundBrowse = umi_gtk4_automation_find_tagged_widget(window, "studio.terminal.browse");
    GtkWidget *foundView = umi_gtk4_automation_find_tagged_widget(window, "studio.terminal.output");
    CHECK(GTK_IS_ENTRY(foundEntry) && GTK_IS_BUTTON(foundBrowse) && GTK_IS_TEXT_VIEW(foundView));
    testEntry = g_object_ref(foundEntry); testBrowse = g_object_ref(foundBrowse); view = g_object_ref(foundView);
    UmiStudioGtkRuntimeChrome *chrome = workbench->runtime_chrome;
    CHECK(GTK_IS_LABEL(chrome->terminal_summary_label) && GTK_IS_BUTTON(chrome->terminal_run_button) &&
        GTK_IS_BUTTON(chrome->terminal_stop_button));
    summary = g_object_ref(chrome->terminal_summary_label);
    run = g_object_ref(chrome->terminal_run_button); stop = g_object_ref(chrome->terminal_stop_button);
    gtk_editable_set_text(GTK_EDITABLE(testEntry), root);
    if (strncmp(testMode, "render-", 7U) == 0) {
        UmiTerminalSession *session = umi_terminal_controller_active_session(testController);
        CHECK(umi_terminal_transcript_append(umi_terminal_session_transcript(session), 0U,
            UMI_TERMINAL_STREAM_OUTPUT, "fresh terminal output") == UMI_STATUS_OK);
        if (strcmp(testMode, "render-finished-session") == 0)
            CHECK(umi_terminal_session_close(session) == UMI_STATUS_OK);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
        if (strcmp(testMode, "render-folder-close") == 0) {
            ((UmiStudioGtkRuntimeChrome *)workbench->runtime_chrome)->terminal_directory_session[0] = '\0';
            gtk_editable_set_text(GTK_EDITABLE(testEntry), "old folder");
            g_signal_connect(testEntry, "changed", G_CALLBACK(ContentChanged), NULL);
        } else if (strcmp(testMode, "render-summary-close") == 0)
            g_signal_connect(summary, "notify::label", G_CALLBACK(PropertyChanged), NULL);
        else if (strcmp(testMode, "render-run-close") == 0) {
            gtk_widget_set_sensitive(run, FALSE);
            g_signal_connect(run, "notify::sensitive", G_CALLBACK(PropertyChanged), NULL);
        } else if (strcmp(testMode, "render-stop-close") == 0) {
            gtk_widget_set_sensitive(stop, TRUE);
            g_signal_connect(stop, "notify::sensitive", G_CALLBACK(PropertyChanged), NULL);
        } else g_signal_connect(buffer, "changed", G_CALLBACK(ContentChanged), NULL);
        armed = 1; TerminalRefreshHost(); armed = 0;
/* Read the terminal fixture counter. The previous statement is retained for review. */
#if 0
        CHECK(callbacks == 1);
#endif
        CHECK(terminalCallbacks == 1);
        if (strstr(testMode, "close") != NULL) CHECK(workbench == NULL);
        else if (strcmp(testMode, "render-switch") == 0) {
            /* The older pass must not publish the former session summary. A
             * subsequent refresh displays the newly selected session. */
            TerminalRefreshHost();
            CHECK(strstr(gtk_label_get_text(GTK_LABEL(summary)), "Other session") != NULL);
        } else if (strcmp(testMode, "render-reenter") == 0 || strcmp(testMode, "render-finished-session") == 0) {
            GtkTextIter begin, end;
            gtk_text_buffer_get_bounds(buffer, &begin, &end);
            text = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
            CHECK(strstr(text, "fresh terminal output") != NULL);
        } else CHECK(0);
        goto verify;
    }
    if (strcmp(testMode, "draft-seed") == 0) gtk_editable_set_text(GTK_EDITABLE(testEntry), draft);
    if (strcmp(testMode, "relative-seed") == 0) gtk_editable_set_text(GTK_EDITABLE(testEntry), "relative draft");
    if (strcmp(testMode, "empty-seed") == 0) gtk_editable_set_text(GTK_EDITABLE(testEntry), "");
    expected = g_file_new_for_path(strcmp(testMode, "draft-seed") == 0 ? draft : root);
    armed = 1; g_signal_emit_by_name(testBrowse, "clicked"); armed = 0;
    if (strncmp(testMode, "create-", 7U) == 0 || strncmp(testMode, "title-", 6U) == 0 ||
        strncmp(testMode, "seed-", 5U) == 0) {
/* Read the terminal fixture counter. The previous statement is retained for review. */
#if 0
        CHECK(callbacks == 1);
#endif
        CHECK(terminalCallbacks == 1);
        if (strstr(testMode, "reenter") == NULL) {
            CHECK(starts == 0U && pending == NULL);
            if (strstr(testMode, "close") != NULL) CHECK(workbench == NULL);
            else CHECK(!((UmiStudioGtkRuntimeChrome *)workbench->runtime_chrome)->terminal_directory_choosing);
            goto verify;
        }
    }
    CHECK(starts == 1U && seed != NULL && g_file_equal(seed, expected));
    if (strcmp(testMode, "inline-reply") == 0 || strcmp(testMode, "present-close") == 0) {
        CHECK(pending == NULL);
        if (strcmp(testMode, "present-close") == 0) CHECK(workbench == NULL);
        else CHECK(!((UmiStudioGtkRuntimeChrome *)workbench->runtime_chrome)->terminal_directory_choosing);
        goto verify;
    }
    CHECK(pending != NULL);
    if (strcmp(testMode, "late-close") == 0) CloseHost();
    else if (strcmp(testMode, "late-reparent") == 0) {
        GtkWidget *parent = gtk_widget_get_parent(testEntry);
        CHECK(GTK_IS_BOX(parent));
        otherWindow = GTK_WINDOW(g_object_ref_sink(gtk_window_new()));
        gtk_box_remove(GTK_BOX(parent), testEntry);
        gtk_window_set_child(otherWindow, testEntry);
    }
    else if (strcmp(testMode, "late-edit") == 0) gtk_editable_set_text(GTK_EDITABLE(testEntry), "newer draft");
    else if (strcmp(testMode, "late-edit-return") == 0) {
        gtk_editable_set_text(GTK_EDITABLE(testEntry), "temporary draft");
        gtk_editable_set_text(GTK_EDITABLE(testEntry), root);
    } else if (strcmp(testMode, "late-switch-return") == 0) {
        CHECK(umi_terminal_controller_activate(testController, "chooser.other") == UMI_STATUS_OK);
        CHECK(umi_terminal_controller_activate(testController, firstId) == UMI_STATUS_OK);
    } else if (strcmp(testMode, "late-directory") == 0)
        CHECK(umi_terminal_session_set_working_directory(umi_terminal_controller_active_session(testController), draft) == UMI_STATUS_OK);
    else if (strcmp(testMode, "reply-close") == 0) {
        armed = 1; g_signal_connect(testEntry, "changed", G_CALLBACK(ContentChanged), NULL);
    }
    Reply(strcmp(testMode, "cancel") == 0 ? NULL : g_file_new_for_path(other)); armed = 0;
    if (strcmp(testMode, "late-close") == 0 || strcmp(testMode, "reply-close") == 0) CHECK(workbench == NULL);
    else {
        CHECK(!((UmiStudioGtkRuntimeChrome *)workbench->runtime_chrome)->terminal_directory_choosing);
        const char *expectedText = strcmp(testMode, "late-edit") == 0 ? "newer draft" :
            (strncmp(testMode, "late-", 5U) == 0 || strcmp(testMode, "cancel") == 0) ? root : other;
/* Compare native folder identities across slash styles without weakening the exact draft check. The previous statement is retained for review. */
#if 0
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(testEntry)), expectedText) == 0);
#endif
        /* GFile returns native separators; a selected absolute folder is a
         * path identity, while an untouched relative draft is exact text. */
        if (strcmp(testMode, "late-edit") == 0)
            CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(testEntry)), expectedText) == 0);
        else
            CHECK(umi_path_equal(gtk_editable_get_text(GTK_EDITABLE(testEntry)), expectedText));
        UmiTerminalSessionSnapshot actual;
        CHECK(umi_terminal_session_snapshot(umi_terminal_controller_active_session(testController), &actual) == UMI_STATUS_OK);
        CHECK(umi_path_equal(actual.working_directory, strcmp(testMode, "late-directory") == 0 ? draft : root));
    }
verify:
    CHECK(!UmiTerminalControllerJobPending(testController));
    cwdAfter = g_get_current_dir(); CHECK(umi_path_equal(cwdAfter, root));
cleanup:
    armed = 0;
    if (pending != NULL) Reply(NULL);
    testHost = NULL;
    umi_studio_gtk_workbench_destroy(workbench);
    if (otherWindow != NULL) gtk_window_destroy(otherWindow);
    g_clear_object(&otherWindow);
    umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&seed); g_clear_object(&expected);
    g_clear_object(&testEntry); g_clear_object(&testBrowse); g_clear_object(&view);
    g_clear_object(&summary); g_clear_object(&run); g_clear_object(&stop);
    if (changedDirectory) (void)g_chdir(original);
    if (isolated) {
        if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    g_free(root); g_free(other); g_free(draft); g_free(text); g_free(original); g_free(oldData); g_free(cwdAfter);
    return failed;
}
