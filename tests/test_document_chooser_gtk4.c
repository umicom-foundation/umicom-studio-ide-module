/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_document_chooser_gtk4.c
 * PURPOSE: Exercise production chooser requests with deterministic native replies.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

/* Intercept only native chooser presentation and replies. Compile the real
 * workbench below, so request identity, location seeding and document IO use
 * the production implementation. The ordinary first-use target separately
 * links that workbench without these native-boundary substitutions. */
static UmiStudioGtkWorkbench **chooserHost;
static const char *chooserScenario, *chooserNextProject;
static UmiStatus chooserNestedStatus;
static int chooserArmed, chooserClosed;
static unsigned chooserStarts;
static GTask *chooserTask;
static GAsyncReadyCallback chooserCallback;
static gpointer chooserContext;
static GFile *chooserFolder;
static char *chooserName, *chooserTitle;
static void ChooserCloseHost(void)
{
    if (chooserHost != NULL && *chooserHost != NULL) {
        UmiStudioGtkWorkbench *closing = *chooserHost;
        *chooserHost = NULL; chooserClosed = 1;
        umi_studio_gtk_workbench_destroy(closing);
    }
}
static void ChooserProperty(GObject *object, GParamSpec *property, gpointer context)
{
    (void)object; (void)property; (void)context;
    if (!chooserClosed) ChooserCloseHost();
}
static GtkFileDialog *ChooserNew(void)
{
    GtkFileDialog *dialog = gtk_file_dialog_new();
    if (chooserArmed) {
        if (strcmp(chooserScenario, "open-switch-project") == 0)
            chooserNestedStatus = umi_studio_gtk_workbench_execute_command(*chooserHost, "workspace.open-folder", chooserNextProject);
        else if (strstr(chooserScenario, "create-close") != NULL) ChooserCloseHost();
        else if (strstr(chooserScenario, "seed-close") != NULL)
            g_signal_connect(dialog, "notify::initial-folder", G_CALLBACK(ChooserProperty), NULL);
    }
    return dialog;
}
static void ChooserReply(GFile *file)
{
    GTask *task = chooserTask;
    GAsyncReadyCallback callback = chooserCallback;
    gpointer context = chooserContext;
    chooserTask = NULL; chooserCallback = NULL; chooserContext = NULL;
    if (file != NULL) g_task_return_pointer(task, file, g_object_unref);
    else g_task_return_new_error(task, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED, "Dismissed");
    callback(g_task_get_source_object(task), G_ASYNC_RESULT(task), context);
    g_object_unref(task);
}
static void ChooserPresent(GtkFileDialog *dialog, GtkWindow *parent, GCancellable *cancel,
    GAsyncReadyCallback callback, gpointer context)
{
    (void)parent;
    ++chooserStarts;
    g_set_object(&chooserFolder, gtk_file_dialog_get_initial_folder(dialog));
    g_free(chooserName); chooserName = g_strdup(gtk_file_dialog_get_initial_name(dialog));
    g_free(chooserTitle); chooserTitle = g_strdup(gtk_file_dialog_get_title(dialog));
    chooserTask = g_task_new(dialog, cancel, NULL, NULL);
    g_task_set_check_cancellable(chooserTask, FALSE);
    chooserCallback = callback; chooserContext = context;
    if (strstr(chooserScenario, "present-close") != NULL) { ChooserCloseHost(); ChooserReply(NULL); }
}
static GFile *ChooserFinish(GtkFileDialog *dialog, GAsyncResult *result, GError **error)
{
    (void)dialog; return g_task_propagate_pointer(G_TASK(result), error);
}
#define gtk_file_dialog_new ChooserNew
#define gtk_file_dialog_open ChooserPresent
#define gtk_file_dialog_save ChooserPresent
#define gtk_file_dialog_select_folder ChooserPresent
#define gtk_file_dialog_open_finish ChooserFinish
#define gtk_file_dialog_save_finish ChooserFinish
#define gtk_file_dialog_select_folder_finish ChooserFinish
#include "../src/gui/workbench/workbench_window.c"
#undef gtk_file_dialog_new
#undef gtk_file_dialog_open
#undef gtk_file_dialog_save
#undef gtk_file_dialog_select_folder
#undef gtk_file_dialog_open_finish
#undef gtk_file_dialog_save_finish
#undef gtk_file_dialog_select_folder_finish

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); failed = 1; goto cleanup; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *mode = argv[1];
    const char *cases[] = {"open-create-close", "save-create-close", "folder-create-close",
        "open-seed-close", "save-seed-close", "folder-seed-close", "open-present-close",
        "save-present-close", "folder-present-close", "open-cancel", "save-cancel", "folder-cancel",
        "open-choose", "save-choose", "folder-choose", "open-late-reply", "save-late-reply",
        "folder-late-reply", "save-other-tab", "open-switch-project"};
    int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (strcmp(mode, cases[i]) == 0) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, changedDirectory = 0;
    char *original = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *root = g_build_filename(argv[2], "chooser-XXXXXX", NULL);
    char *project = NULL, *destination = NULL, *secondProject = NULL, *disk = NULL;
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; GtkApplication *application = NULL;
    GFile *expectedFolder = NULL;
    const int save = strncmp(mode, "save-", 5U) == 0;
    const int folder = strncmp(mode, "folder-", 7U) == 0;
    CHECK(g_mkdtemp(root) != NULL && g_chdir(root) == 0); changedDirectory = 1;
    project = g_build_filename(root, "selected project", NULL);
    secondProject = g_build_filename(root, "next project", NULL);
    destination = g_build_filename(project, "source.c", NULL);
    CHECK(g_mkdir_with_parents(project, 0700) == 0 && g_mkdir_with_parents(secondProject, 0700) == 0);
    if (!save) CHECK(g_file_set_contents(destination, "opened source\n", -1, NULL));
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions serviceOptions = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.chooser-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", project) == UMI_STATUS_OK);
    /* Folder adoption intentionally refuses an unfinished index task. Wait for
     * this fixture setup before exercising chooser-driven project changes. */
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot captured = {0};
    if (save) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.new", NULL) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &captured) == UMI_STATUS_OK);
        UmiUiDocumentViewSnapshot view;
        CHECK(umi_ui_document_view_model_find(views, captured.view_id, &view) == UMI_STATUS_OK); view.dirty = 1;
        CHECK(UmiUiDocumentViewModelUpsertText(views, &view, "chosen draft\n", 13U) == UMI_STATUS_OK);
    }
    expectedFolder = g_file_new_for_path(project);
    chooserHost = &workbench; chooserScenario = mode; chooserNextProject = secondProject; chooserArmed = 1;
    UmiStatus status = umi_studio_gtk_workbench_execute_command(workbench,
        save ? "file.save-as" : folder ? "workspace.open-folder" : "file.open", NULL);
    chooserArmed = 0;
    if (strstr(mode, "create-close") != NULL || strstr(mode, "seed-close") != NULL) {
        CHECK(status == UMI_STATUS_CANCELLED && workbench == NULL && chooserClosed && chooserStarts == 0U);
        CHECK(chooserTask == NULL); goto verify;
    }
    if (strstr(mode, "present-close") != NULL) {
        CHECK(workbench == NULL && chooserClosed && chooserStarts == 1U && chooserTask == NULL);
        CHECK(status == UMI_STATUS_OK); goto verify;
    }
    CHECK(status == UMI_STATUS_OK && chooserTask != NULL && chooserStarts == 1U);
    CHECK(chooserFolder != NULL && g_file_equal(chooserFolder, expectedFolder));
    if (strcmp(mode, "open-switch-project") == 0) CHECK(chooserNestedStatus == UMI_STATUS_OK);
    CHECK(strcmp(chooserTitle, save ? "Save As" : folder ? "Open Workspace Folder" : "Open File") == 0);
    if (save) CHECK(chooserName != NULL && strcmp(chooserName, captured.display_name) == 0);
    if (strstr(mode, "late-reply") != NULL) ChooserCloseHost();
    UmiDocumentWorkingCopySnapshot other = {0};
    if (strcmp(mode, "save-other-tab") == 0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench, "file.new", NULL) == UMI_STATUS_OK);
        CHECK(umi_document_coordinator_active_snapshot(documents, &other) == UMI_STATUS_OK);
    }
    if (strstr(mode, "cancel") != NULL) ChooserReply(NULL);
    else ChooserReply(g_file_new_for_path(folder ? secondProject : destination));
    if (strcmp(mode, "save-other-tab") == 0) {
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && active.document_id == other.document_id);
    }
    if (strstr(mode, "choose") != NULL || strcmp(mode, "save-other-tab") == 0 ||
        strcmp(mode, "open-switch-project") == 0) {
        if (save) CHECK(g_file_get_contents(destination, &disk, NULL, NULL) && strcmp(disk, "chosen draft\n") == 0);
        else if (folder) {
            UmiStudioWorkspaceSnapshot current;
            CHECK(umi_studio_workspace_snapshot(services, &current) == UMI_STATUS_OK && umi_path_equal(current.graph.root, secondProject));
        } else {
            UmiDocumentWorkingCopySnapshot active;
            CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK && umi_path_equal(active.path, destination));
        }
        goto cleanup;
    }
verify:
    if (save) {
        CHECK(!g_file_test(destination, G_FILE_TEST_EXISTS));
        UmiUiDocumentViewSnapshot view;
        CHECK(umi_ui_document_view_model_find(views, captured.view_id, &view) == UMI_STATUS_OK && view.dirty);
    } else CHECK(umi_document_coordinator_count(documents) == 0U);
cleanup:
    chooserArmed = 0;
    if (chooserTask != NULL) ChooserReply(NULL);
    chooserHost = NULL;
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application); g_clear_object(&expectedFolder); g_clear_object(&chooserFolder);
    if (changedDirectory) (void)g_chdir(original);
    if (oldData != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE); else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(chooserName); g_free(chooserTitle); g_free(disk); g_free(root); g_free(project); g_free(secondProject);
    g_free(destination); g_free(original); g_free(oldData);
    return failed;
}
