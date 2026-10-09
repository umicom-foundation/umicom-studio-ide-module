/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_decoded_search_gtk4.c
 * PURPOSE: Exercise decoded saved-file search and exact result navigation through Studio production controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/search.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(x))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                          \
            failed = 1;                                                                                      \
            goto cleanup;                                                                                    \
        }                                                                                                    \
    } while (0)

/* The rendered-child walk omitted controls owned by collapsed expanders. The shared bounded logical-tree lookup replaces it; retain the earlier traversal for review. */
#if 0
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    if (root == NULL)
        return NULL;
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0)
        return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkWidget *found = Find(child, tag);
        if (found != NULL)
            return found;
    }
    return NULL;
}
#endif
/* Use the Framework logical tree so a collapsed panel can be inspected
 * without changing the user's layout or overlooking an ambiguous identifier. */
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    return umi_gtk4_automation_find_tagged_widget(root, tag);
}

/* The unpresented workbench uses a private temporary configuration. No tool
 * discovery, build process, remote AI or personal session restore is enabled. */

static GtkWidget *FindResult(GtkWidget *root)
{
    if (root == NULL)
        return NULL;
    if (g_object_get_data(G_OBJECT(root), "umicom-search-row") != NULL)
        return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        GtkWidget *found = FindResult(child);
        if (found != NULL)
            return found;
    }
    return NULL;
}
static UmiStatus Collect(const UmiSearchMatch *match, void *context)
{
    *(UmiSearchMatch *)context = *match;
    return UMI_STATUS_OK;
}
int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    const char *mode = argv[1],
               *cases[] = {"utf16-le",    "utf16-be",        "utf8-bom",    "bare-cr",        "unicode-path",
                           "unsupported", "draft-preserved", "stale-index", "retained-result"};
    int known = 0;
    for (size_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(mode, cases[i]) == 0)
            known = 1;
    if (!known)
        return 1;
    int failed = 0, changed_directory = 0, isolated = 0;
    GtkApplication *application = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiStudioServicesOptions services_options = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    GtkWidget *retained = NULL;
    char *directory = NULL, *original_directory = NULL, *original_data = NULL;
    GError *error = NULL;
    char workspace[UMI_PATH_CAPACITY], source[UMI_PATH_CAPACITY];
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check())
        return 77;
    original_directory = g_get_current_dir();
    original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-search-scope-XXXXXX", &error);
    CHECK(original_directory != NULL && directory != NULL && g_chdir(directory) == 0);
    changed_directory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_path_join(directory, "workspace", workspace, sizeof(workspace)) == UMI_STATUS_OK);
    CHECK(umi_fs_make_directories(workspace) == UMI_STATUS_OK);
    CHECK(umi_path_join(workspace, strcmp(mode, "unicode-path") == 0 ? "caf\xc3\xa9.txt" : "notes.txt",
                        source, sizeof(source)) == UMI_STATUS_OK);
    const char *text = "top\r\n  needle\r\n";
    unsigned char encoded[128];
    size_t count = 0U;
    if (strcmp(mode, "utf8-bom") == 0)
    {
        memcpy(encoded, "\xef\xbb\xbf", 3U);
        count = 3U;
        memcpy(encoded + count, text, strlen(text));
        count += strlen(text);
    }
    else if (strcmp(mode, "bare-cr") == 0)
    {
        text = "top\r  needle\r";
        count = strlen(text);
        memcpy(encoded, text, count);
    }
    else if (strcmp(mode, "unsupported") == 0)
    {
        const unsigned char malformed[] = {0xff, 0xfe, 0x00, 0xd8};
        memcpy(encoded, malformed, sizeof(malformed));
        count = sizeof(malformed);
    }
    else
    {
        int big = strcmp(mode, "utf16-be") == 0;
        encoded[count++] = big ? 0xfeU : 0xffU;
        encoded[count++] = big ? 0xffU : 0xfeU;
        for (size_t i = 0U; text[i] != '\0'; ++i)
        {
            encoded[count++] = big ? 0U : (unsigned char)text[i];
            encoded[count++] = big ? (unsigned char)text[i] : 0U;
        }
    }
    CHECK(g_file_set_contents(source, (const char *)encoded, (gssize)count, &error));
    CHECK(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    CHECK(umi_studio_workspace_open(umi_studio_bootstrap_services(bootstrap), workspace, 0, 0) ==
          UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.search-scope-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                       umi_studio_bootstrap_desktop_shell(bootstrap),
                                                       &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_SEARCH, NULL) ==
          UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *query = Find(root, "umicom-file-search-query");
    GtkWidget *include = Find(root, "umicom-file-search-include");
    GtkWidget *exclude = Find(root, "umicom-file-search-exclude");
    GtkWidget *start = Find(root, "umicom-file-search-start");
    CHECK(GTK_IS_EDITABLE(query) && GTK_IS_EDITABLE(include) && GTK_IS_EDITABLE(exclude) &&
          GTK_IS_BUTTON(start));
    /* Synchronous API clients and the native search panel must share decoded
     * coordinates. The production panel runs its usual background worker. */
    UmiSearchRequest direct = umi_search_request_default("needle");
    UmiSearchStats stats;
    UmiSearchMatch direct_match = {0};
    CHECK(umi_studio_search_text(umi_studio_bootstrap_services(bootstrap), &direct, Collect, &direct_match,
                                 &stats) == UMI_STATUS_OK);
    size_t expected = strcmp(mode, "unsupported") == 0 ? 0U : 1U;
    CHECK(stats.matches == expected);
    gtk_editable_set_text(GTK_EDITABLE(query), "needle");
    gtk_editable_set_text(GTK_EDITABLE(include), "*.txt");
    g_signal_emit_by_name(start, "clicked");
    UmiFileSearchSnapshot state;
    for (unsigned attempt = 0U; attempt < 10000U; ++attempt)
    {
        CHECK(UmiStudioUiSearchRead(ui, &state) == UMI_STATUS_OK);
        if (!state.active)
            break;
        g_usleep(1000UL);
    }
    CHECK(state.ready && state.stats.matches == expected);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    if (expected == 0U)
    {
        CHECK(state.stats.binary_files_skipped == 1U);
        goto cleanup;
    }
    UmiSearchMatch match;
    CHECK(UmiStudioUiSearchMatchAt(ui, state.requestId, 0U, &match) == UMI_STATUS_OK);
    CHECK(match.line == 2U && match.column == 3U && direct_match.line == match.line &&
          direct_match.column == match.column);
    GtkWidget *result = FindResult(root);
    CHECK(GTK_IS_BUTTON(result));
    retained = g_object_ref(result);
    g_signal_emit_by_name(result, "clicked");
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    CHECK(umi_path_equal(active.path, source));
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, active.view_id, &view) == UMI_STATUS_OK);
    CHECK(view.cursor_offset == 6U);
    char *draft = NULL;
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &draft, &bytes) == UMI_STATUS_OK);
    CHECK(strcmp(draft, "top\n  needle\n") == 0);
    UmiUiDocumentViewModelFreeText(draft);
    if (strcmp(mode, "draft-preserved") == 0)
    {
        const char *edited = "unsaved\ntop\n  needle\n";
        view.dirty = 1;
        view.cursor_offset = 0U;
        view.selection_length = 0U;
        CHECK(UmiUiDocumentViewModelUpsertText(views, &view, edited, strlen(edited)) == UMI_STATUS_OK);
        CHECK(UmiStudioUiSearchOpen(ui, state.requestId, 0U) == UMI_STATUS_NOT_FOUND);
        CHECK(UmiUiDocumentViewModelCopyText(views, active.view_id, &draft, &bytes) == UMI_STATUS_OK);
        CHECK(strcmp(draft, edited) == 0);
        UmiUiDocumentViewModelFreeText(draft);
    }
    if (strcmp(mode, "stale-index") == 0)
    {
        CHECK(umi_file_index_clear(
                  umi_studio_services_file_index(umi_studio_bootstrap_services(bootstrap))) == UMI_STATUS_OK);
        CHECK(UmiStudioUiSearchOpen(ui, state.requestId, 0U) == UMI_STATUS_BUSY);
    }
    if (strcmp(mode, "retained-result") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        g_signal_emit_by_name(retained, "clicked");
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK &&
              umi_path_equal(active.path, source));
    }
cleanup:
    g_clear_object(&retained);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed_directory && g_chdir(original_directory) != 0)
        failed = 1;
    if (isolated)
    {
        if (original_data != NULL)
        {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data, TRUE))
                failed = 1;
        }
        else
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL)
        fprintf(stderr, "%s\n", error->message);
    if (directory != NULL)
        printf("Retained isolated Studio search fixture: %s\n", directory);
    g_clear_error(&error);
    g_free(directory);
    g_free(original_directory);
    g_free(original_data);
    return failed;
}
