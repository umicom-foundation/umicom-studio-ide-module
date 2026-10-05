/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_selected_code_chat_gtk4.c
 * PURPOSE: Exercise the actual Studio selected-code action with an isolated, non-network window boundary.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/provider_connections/gtk4.h"
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

/* Only window presentation is replaced. The real Studio button, active
 * document choice and weak lifetime binding call this boundary. Capturing
 * bytes still uses the actual Framework selection owner; no user database,
 * network, local credential or provider request is opened by this fixture. */
static unsigned calls;
static char selected[8193];
static UmiStatus last_status;
static UmiStudioGtkWorkbench **close_on_present;
static bool fail_after_close;
/* Extend the original capture-only fixture with synchronous host teardown.
 * The previous boundary is retained so its earlier selection checks stay
 * reviewable; the replacement preserves them before injecting destruction. */
#if 0
UmiStatus FixturePresentSelection(GtkWindow *parent, const char *application, const char *profile,
                                  const UmiUiDocumentViewModel *documents, const char *view_id)
{
    ++calls;
    if (!GTK_IS_WINDOW(parent) || strcmp(application, "studio") != 0 || strcmp(profile, "desktop") != 0)
        return last_status = UMI_STATUS_INVALID_ARGUMENT;
    UmiUiDocumentSelectionInfo info;
    return last_status = UmiUiDocumentViewModelCopySelection(documents, view_id,
                                                             umi_ui_document_view_model_revision(documents),
                                                             selected, sizeof(selected), &info);
}
#endif
UmiStatus FixturePresentSelection(GtkWindow *parent, const char *application, const char *profile,
    const UmiUiDocumentViewModel *documents, const char *view_id)
{
    ++calls;
    if (!GTK_IS_WINDOW(parent) || strcmp(application, "studio") != 0 || strcmp(profile, "desktop") != 0)
        return last_status = UMI_STATUS_INVALID_ARGUMENT;
    UmiUiDocumentSelectionInfo info;
    last_status = UmiUiDocumentViewModelCopySelection(documents, view_id,
        umi_ui_document_view_model_revision(documents), selected, sizeof(selected), &info);
    if (close_on_present != NULL && *close_on_present != NULL) {
        UmiStudioGtkWorkbench *owner = *close_on_present; *close_on_present = NULL;
        umi_studio_gtk_workbench_destroy(owner);
        if (fail_after_close) last_status = UMI_STATUS_UNAVAILABLE;
    }
    return last_status;
}
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
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    const char *name = argv[1];
    const char *cases[] = {"selection", "empty", "large-selection", "new-selection", "retained", "close-success", "close-failure"};
    bool known = false;
    for (size_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(name, cases[i]) == 0)
            known = true;
    if (!known)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0;
    bool changed_directory = false;
    char *original = g_get_current_dir(), *directory = NULL,
         *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    UmiSettings *settings = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    GtkApplication *application = NULL;
    GtkWidget *retained = NULL;
    GError *error = NULL;
    directory = g_dir_make_tmp("umicom-selected-chat-XXXXXX", &error);
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
    UmiStudioServicesOptions services = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char id[UMI_UI_ID_CAPACITY];
    CHECK(umi_document_coordinator_new(documents, "Private filename.c", id, sizeof(id)) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.selected-chat-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_AI_CHAT, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *button =
        Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.ai.selected-code");
    CHECK(GTK_IS_BUTTON(button));
    retained = g_object_ref(button);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
    const char *text = "first chosen second";
    char big[9001];
    memset(big, 'x', sizeof(big));
    big[sizeof(big) - 1U] = '\0';
    if (strcmp(name, "large-selection") == 0)
        text = big;
    view.cursor_offset = 6U;
    view.selection_length = 6U;
    view.dirty = 1;
    if (strcmp(name, "empty") == 0)
        view.selection_length = 0U;
    if (strcmp(name, "large-selection") == 0)
    {
        view.cursor_offset = 0U;
        view.selection_length = 9000U;
    }
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    /* Publish after refresh to isolate capture from unrelated editor repaint. */
    if (strcmp(name, "retained") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(calls == 0U);
        goto cleanup;
    }
    if (strcmp(name, "close-success") == 0 || strcmp(name, "close-failure") == 0) {
        close_on_present = &workbench; fail_after_close = strcmp(name, "close-failure") == 0;
    }
    g_signal_emit_by_name(button, "clicked");
    CHECK(calls == 1U);
    if (close_on_present != NULL) {
        CHECK(workbench == NULL && last_status == (fail_after_close ? UMI_STATUS_UNAVAILABLE : UMI_STATUS_OK));
        goto cleanup;
    }
    if (strcmp(name, "empty") == 0)
        CHECK(last_status == UMI_STATUS_INVALID_STATE);
    else if (strcmp(name, "large-selection") == 0)
        CHECK(last_status == UMI_STATUS_CAPACITY_EXCEEDED);
    else
    {
        CHECK(last_status == UMI_STATUS_OK && strcmp(selected, "chosen") == 0);
        if (strcmp(name, "new-selection") == 0)
        {
            CHECK(umi_ui_document_view_model_find(views, id, &view) == UMI_STATUS_OK);
            view.cursor_offset = 13U;
            view.selection_length = 6U;
            CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
            CHECK(strcmp(selected, "chosen") == 0);
            g_signal_emit_by_name(button, "clicked");
            CHECK(calls == 2U && last_status == UMI_STATUS_OK && strcmp(selected, "second") == 0);
        }
    }
cleanup:
    if (workbench != NULL)
        umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    if (bootstrap != NULL)
        umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL)
        umi_settings_destroy(settings);
    g_clear_object(&application);
    g_clear_error(&error);
    if (changed_directory)
        (void)g_chdir(original);
    if (old_data != NULL)
        (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else
        g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data);
    g_free(directory);
    g_free(original);
    return failed;
}
