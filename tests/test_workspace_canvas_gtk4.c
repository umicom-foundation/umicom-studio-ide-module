/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_workspace_canvas_gtk4.c
 *
 * PURPOSE:
 *   Exercise the actual Studio GTK adapter, real editor and shared canvas
 *   without presenting windows, probing tools or using personal session files.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/source_control.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/document_view.h"
#include "umicom/workbench_layout_data/key_codec.h"

#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

/* Keep failure checks active with NDEBUG and release all fixture-owned state. */
#define REQUIRE(expression) do { \
    if (!(expression)) { \
        (void)fprintf(stderr, "Line %d: %s\n", __LINE__, #expression); \
        failed = 1; \
        goto cleanup; \
    } \
} while (0)

/* Locate semantic controls independently of translated labels or child order. */
static GtkWidget *find_tag(GtkWidget *widget, const char *tag)
{
    GtkWidget *child;
    const char *actual;
    if (widget == NULL) return NULL;
    actual = g_object_get_data(G_OBJECT(widget), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0) return widget;
    for (child = gtk_widget_get_first_child(widget); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = find_tag(child, tag);
        if (found != NULL) return found;
    }
    return NULL;
}

/* Select the operational text editor, not Output, Terminal or a summary card. */
static GtkWidget *find_editor(GtkWidget *widget)
{
    GtkWidget *child;
    if (widget == NULL) return NULL;
    if (GTK_IS_TEXT_VIEW(widget) && gtk_widget_has_css_class(widget, "umicom-editor"))
        return widget;
    for (child = gtk_widget_get_first_child(widget); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = find_editor(child);
        if (found != NULL) return found;
    }
    return NULL;
}

/* Shared context controls expose a stable style role without product labels. */
static GtkWidget *find_context_group(GtkWidget *widget)
{
    GtkWidget *child;
    if (widget == NULL) return NULL;
    if (GTK_IS_BUTTON(widget) && gtk_widget_has_css_class(widget, "umicom-context-group-button"))
        return widget;
    for (child = gtk_widget_get_first_child(widget); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = find_context_group(child);
        if (found != NULL) return found;
    }
    return NULL;
}

/* Compare the live draft without borrowing text after GTK releases it. */
static int buffer_contains(GtkTextBuffer *buffer, const char *expected)
{
    GtkTextIter start;
    GtkTextIter end;
    char *text;
    int matches;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    matches = text != NULL && strcmp(text, expected) == 0;
    g_free(text);
    return matches;
}

/* GTK offsets count characters, while Framework source positions count UTF-8
 * bytes. Preserve the native selection direction across ordinary refresh. */
static int has_unicode_selection(GtkTextBuffer *buffer)
{
    GtkTextIter insert;
    GtkTextIter bound;
    gtk_text_buffer_get_iter_at_mark(buffer, &insert, gtk_text_buffer_get_insert(buffer));
    gtk_text_buffer_get_iter_at_mark(buffer, &bound, gtk_text_buffer_get_selection_bound(buffer));
    return gtk_text_iter_get_offset(&insert) == 6 && gtk_text_iter_get_offset(&bound) == 3;
}

/* Process bounded idle work only; no sleeps, application activation or runs. */
static int drain_context(void)
{
    unsigned int count;
    for (count = 0U; count < 512U; ++count) {
        if (!g_main_context_pending(NULL)) return 1;
        (void)g_main_context_iteration(NULL, FALSE);
    }
    return !g_main_context_pending(NULL);
}

/* A regression must never map either the main window or an incidental dialog. */
static int all_windows_unpresented(void)
{
    GListModel *windows = gtk_window_get_toplevels();
    guint index;
    for (index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *window = g_list_model_get_item(windows, index);
        int presented = gtk_widget_get_visible(GTK_WIDGET(window)) ||
            gtk_widget_get_mapped(GTK_WIDGET(window));
        g_object_unref(window);
        if (presented) return 0;
    }
    return 1;
}

/* Exact binary fractions make geometry equality useful without pixel rounding. */
static int window_has_rect(const UmiUiWorkspaceWindow *window,
    const UmiApplicationSuiteLayoutRect *rect)
{
    return window != NULL && window->x == rect->x && window->y == rect->y &&
        window->width == rect->width && window->height == rect->height;
}

/* Find a readable named arrangement in the real model-backed dropdown. */
static guint layout_choice(GtkWidget *selector, const char *name)
{
    GListModel *choices = gtk_drop_down_get_model(GTK_DROP_DOWN(selector));
    guint index;
    if (choices == NULL) return GTK_INVALID_LIST_POSITION;
    for (index = 0U; index < g_list_model_get_n_items(choices); ++index) {
        GtkStringObject *choice = g_list_model_get_item(choices, index);
        int match = strcmp(gtk_string_object_get_string(choice), name) == 0;
        g_object_unref(choice);
        if (match) return index;
    }
    return GTK_INVALID_LIST_POSITION;
}

/* Capture saved fixture chunks by their public namespace, without
 * guessing checkpoint key formatting or changing records during enumeration. */
typedef struct WorkspaceChunkFixture {
    char keys[4U][UMI_WORKBENCH_LAYOUT_DATA_KEY_CAPACITY];
    char values[4U][UMI_WORKBENCH_LAYOUT_DATA_VALUE_CAPACITY];
    size_t count;
} WorkspaceChunkFixture;

/* Only this fixture's isolated connection is visited. Reject excess records
 * instead of silently leaving an untested primary or recovery chunk intact. */
static UmiStatus capture_workspace_chunk(const char *key, const char *value, void *context)
{
    WorkspaceChunkFixture *fixture = (WorkspaceChunkFixture *)context;
    UmiWorkbenchLayoutDataKeyParts parts;
    if (umi_workbench_layout_data_key_parse(key, &parts) != UMI_STATUS_OK ||
        parts.kind != UMI_WORKBENCH_LAYOUT_DATA_RECORD_WORKSPACE_CHUNK) return UMI_STATUS_OK;
    if (fixture->count >= G_N_ELEMENTS(fixture->keys)) return UMI_STATUS_CAPACITY_EXCEEDED;
    if (strlen(value) >= sizeof(fixture->values[fixture->count])) return UMI_STATUS_CAPACITY_EXCEEDED;
    (void)g_strlcpy(fixture->keys[fixture->count], key, sizeof(fixture->keys[fixture->count]));
    (void)g_strlcpy(fixture->values[fixture->count], value, sizeof(fixture->values[fixture->count]));
    fixture->count += 1U;
    return UMI_STATUS_OK;
}

/* Close the real native service graph and SQLite connection between save and
 * recovery. This is a restart fixture, not a claim of process-crash testing.
 * All files remain beneath the already isolated test directory. */
static int verify_durable_canvas(GtkApplication *application, const char *fixture_path)
{
    static const char layout_id[] = "umicom.studio.layout.durable-native-test";
    static const char retained_draft[] = "int independently_owned_draft = 11;\n";
    const char *editor_id = umi_studio_runtime_workspace_surface_id(UMI_STUDIO_SURFACE_EDITOR);
    UmiStudioGtkWorkbenchOptions options = {0};
    UmiStudioServicesOptions services_options = {0};
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiDataServer *server = NULL;
    UmiUiWorkspaceCheckpointReport checkpoint;
    UmiUiWorkspaceLayout *layout = g_try_new0(UmiUiWorkspaceLayout, 1);
    UmiUiWorkspaceLayout *before = g_try_new0(UmiUiWorkspaceLayout, 1);
    UmiApplicationSuiteLayoutRect saved_rect = {0.125, 0.25, 0.5, 0.5};
    WorkspaceChunkFixture chunk = {0};
    GtkWidget *window;
    GtkWidget *save_button;
    GtkWidget *restore_button;
    GtkWidget *selector;
    GtkWidget *editor = NULL;
    GtkTextBuffer *buffer;
    char *database_path = g_build_filename(fixture_path, "canvas-checkpoint.sqlite", NULL);
    UmiStatus status;
    size_t record_count;
    size_t chunk_index;
    size_t recovered_count = 0U;
    uint64_t before_storage_revision;
    int failed = 0;

    REQUIRE(layout != NULL && before != NULL && database_path != NULL);
    status = umi_data_server_create_sqlite(database_path, &server);
    if (status == UMI_STATUS_UNAVAILABLE) {
        (void)puts("SKIP: SQLite is unavailable; durable native canvas acceptance was not run.");
        failed = 77;
        goto cleanup;
    }
    REQUIRE(status == UMI_STATUS_OK);
    REQUIRE(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application,
        umi_studio_bootstrap_ui(bootstrap), umi_studio_bootstrap_desktop_shell(bootstrap),
        &options, &workbench) == UMI_STATUS_OK);

    /* Storage-off construction must not quietly bind the service database. */
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_gtk_workbench_workspace_save(workbench) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_gtk_workbench_workspace_bind_storage(workbench, server, 1) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
    REQUIRE(checkpoint.durable && checkpoint.storage_revision_known && checkpoint.storage_revision == 0U);
    REQUIRE(umi_data_server_count(server) == 0U);
    REQUIRE(umi_studio_gtk_workbench_workspace_create_blank(workbench, layout_id,
        "Durable native canvas") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR,
        "canvas") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench, editor_id,
        &saved_rect, layout->revision) == UMI_STATUS_OK);
    REQUIRE(drain_context());
    window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    save_button = find_tag(window, "studio.workspace.save");
    restore_button = find_tag(window, "studio.workspace.restore");
    REQUIRE(GTK_IS_BUTTON(save_button) && GTK_IS_BUTTON(restore_button));
    REQUIRE(!gtk_widget_get_sensitive(save_button) && !gtk_widget_get_sensitive(restore_button));
    REQUIRE(umi_studio_gtk_workbench_workspace_save(workbench) == UMI_STATUS_BUSY);
    REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) == UMI_STATUS_BUSY);
    REQUIRE(umi_data_server_count(server) == 0U);
    REQUIRE(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(gtk_widget_get_sensitive(save_button));
    g_signal_emit_by_name(save_button, "clicked");
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
    REQUIRE(checkpoint.durable && checkpoint.storage_revision == 1U &&
        strcmp(checkpoint.layout_id, layout_id) == 0 && !checkpoint.recovered_last_good);
    REQUIRE(umi_data_server_count(server) > 0U && all_windows_unpresented());

    /* Destroy both the owner and borrowed connection: a fresh native runtime
     * must reconstruct the named canvas from disk, not an in-memory baseline. */
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    umi_studio_bootstrap_destroy(bootstrap);
    bootstrap = NULL;
    umi_data_server_destroy(server);
    server = NULL;
    REQUIRE(drain_context());
    REQUIRE(umi_data_server_create_sqlite(database_path, &server) == UMI_STATUS_OK);
    REQUIRE(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application,
        umi_studio_bootstrap_ui(bootstrap), umi_studio_bootstrap_desktop_shell(bootstrap),
        &options, &workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, layout_id) != 0);
    REQUIRE(umi_studio_gtk_workbench_workspace_bind_storage(workbench, server, 1) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, layout_id) == 0 && layout->locked && layout->window_count == 1U);
    REQUIRE(window_has_rect(umi_ui_workspace_layout_find_window(layout, editor_id), &saved_rect));
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
    REQUIRE(checkpoint.durable && checkpoint.storage_revision_known && checkpoint.storage_revision == 1U);
    window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    selector = find_tag(window, "studio.workspace.layout-selector");
    REQUIRE(GTK_IS_DROP_DOWN(selector));
    REQUIRE(layout_choice(selector,
        "Durable native canvas") != GTK_INVALID_LIST_POSITION);
    editor = find_editor(find_tag(window, "studio.workspace.editor-root"));
    REQUIRE(editor != NULL);
    g_object_ref(editor);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, retained_draft, -1);

    /* The actual Restore button changes only the arrangement. It must retain
     * the same operational editor and its independently owned unsaved draft. */
    REQUIRE(umi_studio_gtk_workbench_workspace_create_blank(workbench,
        "umicom.studio.layout.unsaved-alternative", "Unsaved alternative") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    restore_button = find_tag(window, "studio.workspace.restore");
    REQUIRE(GTK_IS_BUTTON(restore_button) && gtk_widget_get_sensitive(restore_button));
    g_signal_emit_by_name(restore_button, "clicked");
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, layout_id) == 0 && window_has_rect(
        umi_ui_workspace_layout_find_window(layout, editor_id), &saved_rect));
    REQUIRE(find_editor(find_tag(window, "studio.workspace.editor-root")) == editor &&
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer && buffer_contains(buffer, retained_draft));

    /* A borrowed server may already belong to a product operation. BUSY must
     * preserve the observed checkpoint revision and leave that transaction
     * open for its owner, so an ordinary retry remains possible afterward. */
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
    REQUIRE(checkpoint.storage_revision_known);
    before_storage_revision = checkpoint.storage_revision;
    REQUIRE(umi_data_server_begin(server) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_save(workbench) == UMI_STATUS_BUSY);
    REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) == UMI_STATUS_BUSY);
    REQUIRE(umi_data_server_in_transaction(server));
    REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
    REQUIRE(checkpoint.storage_revision_known && checkpoint.storage_revision == before_storage_revision);
    REQUIRE(umi_data_server_rollback(server) == UMI_STATUS_OK);
    save_button = find_tag(window, "studio.workspace.save");
    REQUIRE(GTK_IS_BUTTON(save_button) && gtk_widget_get_sensitive(save_button));
    REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) == UMI_STATUS_OK);
    REQUIRE(find_editor(find_tag(window, "studio.workspace.editor-root")) == editor &&
        buffer_contains(buffer, retained_draft));

    /* Damage each first-save copy in turn, restoring its fixture bytes before
     * trying the other. Exactly one read uses last-good recovery; this checks
     * the public report without depending on private slot naming or key order. */
    REQUIRE(umi_data_server_visit(server, capture_workspace_chunk, &chunk) == UMI_STATUS_OK);
    REQUIRE(chunk.count == 2U);
    for (chunk_index = 0U; chunk_index < chunk.count; ++chunk_index) {
        REQUIRE(umi_data_server_set(server, chunk.keys[chunk_index], "damaged single copy") == UMI_STATUS_OK);
        REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) == UMI_STATUS_OK);
        REQUIRE(umi_studio_gtk_workbench_workspace_storage_snapshot(workbench, &checkpoint) == UMI_STATUS_OK);
        if (checkpoint.recovered_last_good) recovered_count += 1U;
        REQUIRE(checkpoint.storage_revision_known && checkpoint.storage_revision == 1U);
        REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
        REQUIRE(strcmp(layout->layout_id, layout_id) == 0 && window_has_rect(
            umi_ui_workspace_layout_find_window(layout, editor_id), &saved_rect));
        REQUIRE(find_editor(find_tag(window, "studio.workspace.editor-root")) == editor &&
            buffer_contains(buffer, retained_draft));
        REQUIRE(umi_data_server_set(server, chunk.keys[chunk_index], chunk.values[chunk_index]) == UMI_STATUS_OK);
    }
    REQUIRE(recovered_count == 1U);

    /* Damage both the primary and its first-save recovery copy. Neither may
     * publish partial decoded state; all malformed evidence remains on disk. */
    REQUIRE(umi_studio_gtk_workbench_workspace_create_blank(workbench,
        "umicom.studio.layout.corruption-survivor", "Keep current canvas") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, before) == UMI_STATUS_OK);
    record_count = umi_data_server_count(server);
    for (chunk_index = 0U; chunk_index < chunk.count; ++chunk_index)
        REQUIRE(umi_data_server_set(server, chunk.keys[chunk_index], "damaged checkpoint fixture") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_restore(workbench) != UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(memcmp(before, layout, sizeof(*layout)) == 0 && umi_data_server_count(server) == record_count);
    REQUIRE(buffer_contains(buffer, retained_draft) && all_windows_unpresented());

cleanup:
    umi_studio_gtk_workbench_destroy(workbench);
    if (editor != NULL) g_object_unref(editor);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_data_server_destroy(server);
    g_free(layout);
    g_free(before);
    g_free(database_path);
    return failed;
}

/* Use a unique owned working directory for the real service graph. Its local
 * fixture files are retained for diagnosis; no source or user file is deleted.
 * The bootstrap lifecycle is deliberately not started. Explicit constructor
 * policy disables Git discovery, GTK session storage and refresh timers. */
int main(void)
{
    static const char draft[] = "/* \xCE\xBB \xF0\x9F\x9A\x80 unsaved canvas acceptance draft. */\nint value = 7;\n";
    static const char external_text[] = "/* \xCE\xBB external source update. */\nint value = 9;\n";
    static const char layout_id[] = "umicom.studio.layout.native-canvas-test";
    const char *editor_id = umi_studio_runtime_workspace_surface_id(UMI_STUDIO_SURFACE_EDITOR);
    UmiStudioGtkWorkbenchOptions options = {0};
    UmiStudioGtkWorkbenchOptions defaults = umi_studio_gtk_workbench_options_default();
    UmiStudioServicesOptions services_options = {0};
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    UmiUiWorkspaceLayout *layout = NULL;
    UmiUiDocumentViewSnapshot *document = NULL;
    UmiUiDocumentViewModel *documents = NULL;
    UmiGtk4WorkspaceLayoutHostSnapshot host;
    UmiStudioGtkWorkbenchActivitySnapshot activity;
    UmiStudioSourceControlSnapshot source_control;
    UmiApplicationSuiteLayoutRect applied = {0.125, 0.125, 0.625, 0.625};
    UmiApplicationSuiteLayoutRect cancelled = {0.25, 0.25, 0.5, 0.5};
    const UmiUiWorkspaceWindow *editor_record;
    GtkApplication *application = NULL;
    GtkWidget *editor_root = NULL;
    GtkWidget *editor = NULL;
    GtkWidget *selector = NULL;
    GtkWidget *old_context_button = NULL;
    GtkWidget *last_context_button = NULL;
    GtkWidget *native_window;
    GtkTextBuffer *buffer = NULL;
    GtkTextIter insert;
    GtkTextIter bound;
    GError *error = NULL;
    char *fixture_path = NULL;
    char *original_directory = NULL;
    char *original_data_path = NULL;
    char *oversized = NULL;
    uint64_t before_revision;
    char original_layout_id[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    char original_layout_name[UMI_UI_WORKSPACE_LAYOUT_NAME_CAPACITY];
    guint choice;
    int environment_isolated = 0;
    int directory_changed = 0;
    int before_dirty;
    int failed = 0;

    /* Keep accessibility in-process rather than starting a desktop bus bridge. */
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) {
        (void)puts("SKIP: GTK display is unavailable; no Studio window was created.");
        return 77;
    }
    REQUIRE(defaults.present_window && defaults.persist_session &&
        defaults.restore_session && defaults.automatic_refresh);
    original_directory = g_get_current_dir();
    original_data_path = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    fixture_path = g_dir_make_tmp("umicom-studio-canvas-XXXXXX", &error);
    REQUIRE(original_directory != NULL && fixture_path != NULL);
    REQUIRE(g_chdir(fixture_path) == 0);
    directory_changed = 1;
    /* An ambient data override must not select a personal SQLite database. */
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    environment_isolated = 1;
    REQUIRE(g_mkdir_with_parents("config", 0700) == 0);
    REQUIRE(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) ==
        UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
    REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    REQUIRE(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    umi_settings_destroy(settings);
    settings = NULL;
    REQUIRE(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
    REQUIRE(umi_studio_source_control_service_snapshot(umi_studio_services_source_control(
        umi_studio_bootstrap_services(bootstrap)), &source_control) == UMI_STATUS_OK);
    REQUIRE(!source_control.available &&
        strcmp(source_control.provider_id, "umicom.vcs.unavailable") == 0);
    application = gtk_application_new("org.umicom.studio.native-canvas-test", G_APPLICATION_NON_UNIQUE);
    REQUIRE(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application,
        umi_studio_bootstrap_ui(bootstrap), umi_studio_bootstrap_desktop_shell(bootstrap),
        &options, &workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_activity_snapshot(workbench, &activity) == UMI_STATUS_OK);
    REQUIRE(!activity.automatic_refresh_active && !activity.session_storage_configured);
    REQUIRE(all_windows_unpresented());
    native_window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    editor_root = find_tag(native_window, "studio.workspace.editor-root");
    REQUIRE(editor_root != NULL);
    g_object_ref(editor_root);
    editor = find_editor(editor_root);
    REQUIRE(editor != NULL);
    g_object_ref(editor);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));
    gtk_text_buffer_set_text(buffer, draft, -1);
    REQUIRE(buffer_contains(buffer, draft));
    gtk_text_buffer_get_iter_at_offset(buffer, &insert, 6);
    gtk_text_buffer_get_iter_at_offset(buffer, &bound, 3);
    gtk_text_buffer_select_range(buffer, &insert, &bound);
    REQUIRE(has_unicode_selection(buffer));
    old_context_button = find_context_group(editor_root);
    REQUIRE(old_context_button != NULL);
    g_object_ref(old_context_button);

    /* Oversize insertion is rejected before GTK changes either the draft or
     * its bounded view model; it must not silently save a shortened string. */
    documents = umi_ui_workbench_documents(umi_studio_ui_workbench(
        umi_studio_bootstrap_ui(bootstrap)));
    document = g_try_new0(UmiUiDocumentViewSnapshot, 1);
    oversized = g_try_malloc(UMI_UI_DOCUMENT_CONTENT_CAPACITY + 1U);
    REQUIRE(documents != NULL && document != NULL && oversized != NULL);
    (void)memset(oversized, 'x', UMI_UI_DOCUMENT_CONTENT_CAPACITY);
    oversized[UMI_UI_DOCUMENT_CONTENT_CAPACITY] = '\0';
    before_revision = umi_ui_document_view_model_revision(documents);
    gtk_text_buffer_get_end_iter(buffer, &insert);
    gtk_text_buffer_insert(buffer, &insert, oversized, (gint)UMI_UI_DOCUMENT_CONTENT_CAPACITY);
    REQUIRE(buffer_contains(buffer, draft) && has_unicode_selection(buffer));
    REQUIRE(umi_ui_document_view_model_revision(documents) == before_revision);
    REQUIRE(umi_ui_document_view_model_find(documents, "studio.editor.welcome", document) == UMI_STATUS_OK);
    REQUIRE(strcmp(document->source_text, draft) == 0);
    REQUIRE(document->cursor_offset == 3U && document->selection_length == 7U);

    /* Native paste can delete the current selection before insertion fails.
     * One rejected user action must restore that text and selection together;
     * provisional deletion and rollback may legitimately advance revision. */
    before_dirty = document->dirty;
    gtk_text_buffer_begin_user_action(buffer);
    (void)gtk_text_buffer_get_selection_bounds(buffer, &bound, &insert);
    gtk_text_buffer_delete(buffer, &bound, &insert);
    gtk_text_buffer_insert(buffer, &bound, oversized, (gint)UMI_UI_DOCUMENT_CONTENT_CAPACITY);
    gtk_text_buffer_end_user_action(buffer);
    REQUIRE(buffer_contains(buffer, draft) && has_unicode_selection(buffer));
    REQUIRE(umi_ui_document_view_model_find(documents, "studio.editor.welcome", document) == UMI_STATUS_OK);
    REQUIRE(strcmp(document->source_text, draft) == 0 && document->dirty == before_dirty);
    REQUIRE(document->cursor_offset == 3U && document->selection_length == 7U);
    layout = g_try_new0(UmiUiWorkspaceLayout, 1);
    REQUIRE(layout != NULL);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->window_count > 0U);
    (void)g_strlcpy(original_layout_id, layout->layout_id, sizeof(original_layout_id));
    (void)g_strlcpy(original_layout_name, layout->name, sizeof(original_layout_name));
    selector = find_tag(native_window, "studio.workspace.layout-selector");
    REQUIRE(selector != NULL);
    g_object_ref(selector);
    REQUIRE(GTK_IS_DROP_DOWN(selector));

    /* Blank really means empty. Status sync must not resurrect hidden defaults. */
    REQUIRE(umi_studio_gtk_workbench_workspace_create_blank(workbench, layout_id,
        "Native canvas acceptance") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, layout_id) == 0 && layout->window_count == 0U && !layout->locked);
    choice = layout_choice(selector, "Native canvas acceptance");
    REQUIRE(choice != GTK_INVALID_LIST_POSITION &&
        gtk_drop_down_get_selected(GTK_DROP_DOWN(selector)) == choice);
    REQUIRE(umi_studio_gtk_workbench_workspace_host_snapshot(workbench, &host) == UMI_STATUS_OK);
    REQUIRE(host.panel_count == 0U && host.canvas_count == 0U && host.canvas_editable);
    REQUIRE(host.content_retention_enabled && host.retained_content_count > 0U);
    REQUIRE(find_tag(native_window, "workstation.workspace-canvas") != NULL);
    REQUIRE(!GTK_IS_NOTEBOOK(find_tag(native_window, "workstation.workspace-canvas")));
    REQUIRE(find_tag(native_window, "studio.workspace.editor-root") == NULL);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->window_count == 0U && buffer_contains(buffer, draft) && has_unicode_selection(buffer));
    REQUIRE(!gtk_widget_get_sensitive(old_context_button));
    g_signal_emit_by_name(old_context_button, "clicked");

    /* Catalogue opening must restore the operational editor, not a summary. */
    REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench,
        UMI_STUDIO_SURFACE_EDITOR, "canvas") == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench,
        UMI_STUDIO_SURFACE_EXPLORER, "canvas") == UMI_STATUS_OK);
    REQUIRE(find_tag(native_window, "studio.workspace.editor-root") == editor_root);
    REQUIRE(find_editor(editor_root) == editor && buffer_contains(buffer, draft));
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->window_count == 2U);
    REQUIRE(umi_studio_gtk_workbench_workspace_host_snapshot(workbench, &host) == UMI_STATUS_OK);
    REQUIRE(host.canvas_count == 2U && host.floating_count == 0U);

    /* Requests are deferred and revision guarded exactly like native gestures. */
    before_revision = layout->revision;
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &applied, before_revision) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->revision == before_revision);
    REQUIRE(umi_studio_gtk_workbench_workspace_host_snapshot(workbench, &host) == UMI_STATUS_OK);
    REQUIRE(host.geometry_pending);
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &applied, before_revision) == UMI_STATUS_BUSY);
    REQUIRE(drain_context());
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    editor_record = umi_ui_workspace_layout_find_window(layout, editor_id);
    REQUIRE(layout->revision > before_revision && window_has_rect(editor_record, &applied));
    REQUIRE(find_editor(editor_root) == editor && buffer_contains(buffer, draft));
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &cancelled, before_revision) == UMI_STATUS_INVALID_STATE);

    /* Apply locks geometry; repeated status updates preserve bodies and layout. */
    REQUIRE(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->locked);
    before_revision = layout->revision;
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &cancelled, before_revision) == UMI_STATUS_PERMISSION_DENIED);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->revision == before_revision && layout->window_count == 2U);
    REQUIRE(find_editor(editor_root) == editor &&
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer && buffer_contains(buffer, draft) &&
        has_unicode_selection(buffer));

    /* Select both named layouts using the actual dropdown callback, retaining
     * the same editor body when its outer placement changes between them. */
    choice = layout_choice(selector, original_layout_name);
    REQUIRE(choice != GTK_INVALID_LIST_POSITION);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(selector), choice);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, original_layout_id) == 0);
    REQUIRE(find_editor(editor_root) == editor && buffer_contains(buffer, draft));
    choice = layout_choice(selector, "Native canvas acceptance");
    REQUIRE(choice != GTK_INVALID_LIST_POSITION);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(selector), choice);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(strcmp(layout->layout_id, layout_id) == 0 && window_has_rect(
        umi_ui_workspace_layout_find_window(layout, editor_id), &applied));

    /* Cancel restores model geometry without discarding an unsaved document. */
    REQUIRE(umi_studio_gtk_workbench_workspace_begin_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &cancelled, layout->revision) == UMI_STATUS_OK);
    REQUIRE(drain_context());
    REQUIRE(umi_studio_gtk_workbench_workspace_cancel_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(layout->locked && window_has_rect(
        umi_ui_workspace_layout_find_window(layout, editor_id), &applied));
    REQUIRE(find_editor(editor_root) == editor && buffer_contains(buffer, draft));
    REQUIRE(all_windows_unpresented());

    /* Find and Go To update byte-oriented model positions. An external byte
     * offset after the non-BMP character must become GTK character offset 6,
     * proving retention does not simply ignore all subsequent model changes. */
    REQUIRE(umi_ui_document_view_model_find(documents, "studio.editor.welcome", document) == UMI_STATUS_OK);
    document->cursor_offset = 10U;
    document->selection_length = 0U;
    REQUIRE(umi_ui_document_view_model_upsert(documents, document) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(find_editor(editor_root) == editor && buffer_contains(buffer, draft));
    gtk_text_buffer_get_iter_at_mark(buffer, &insert, gtk_text_buffer_get_insert(buffer));
    gtk_text_buffer_get_iter_at_mark(buffer, &bound, gtk_text_buffer_get_selection_bound(buffer));
    REQUIRE(gtk_text_iter_get_offset(&insert) == 6 && gtk_text_iter_get_offset(&bound) == 6);
    gtk_text_buffer_get_iter_at_offset(buffer, &insert, 6);
    gtk_text_buffer_get_iter_at_offset(buffer, &bound, 3);
    gtk_text_buffer_select_range(buffer, &insert, &bound);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(has_unicode_selection(buffer));

    /* Authoritative source replacement changes text inside the same editor.
     * Keep this after draft-retention assertions so those checks stay useful. */
    REQUIRE(umi_ui_document_view_model_find(documents, "studio.editor.welcome", document) == UMI_STATUS_OK);
    (void)g_strlcpy(document->source_text, external_text, sizeof(document->source_text));
    document->cursor_offset = 0U;
    document->selection_length = 0U;
    REQUIRE(umi_ui_document_view_model_upsert(documents, document) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    REQUIRE(find_editor(editor_root) == editor &&
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)) == buffer && buffer_contains(buffer, external_text));
    gtk_text_buffer_get_iter_at_mark(buffer, &insert, gtk_text_buffer_get_insert(buffer));
    gtk_text_buffer_get_iter_at_mark(buffer, &bound, gtk_text_buffer_get_selection_bound(buffer));
    REQUIRE(gtk_text_iter_get_offset(&insert) == 0 && gtk_text_iter_get_offset(&bound) == 0);

    /* Closing the real owner cancels a queued callback before services die. */
    last_context_button = find_context_group(editor_root);
    REQUIRE(last_context_button != NULL);
    g_object_ref(last_context_button);
    REQUIRE(umi_studio_gtk_workbench_workspace_begin_edit(workbench) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_request_canvas_geometry(workbench,
        editor_id, &cancelled, layout->revision) == UMI_STATUS_OK);
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    REQUIRE(drain_context());
    /* Retaining a destroyed host's selector must not retain a callable owner. */
    gtk_drop_down_set_selected(GTK_DROP_DOWN(selector), GTK_INVALID_LIST_POSITION);
    REQUIRE(!gtk_widget_get_sensitive(last_context_button));
    g_signal_emit_by_name(old_context_button, "clicked");
    g_signal_emit_by_name(last_context_button, "clicked");
    REQUIRE(drain_context());
    REQUIRE(all_windows_unpresented());
    failed = verify_durable_canvas(application, fixture_path);

cleanup:
    umi_studio_gtk_workbench_destroy(workbench);
    if (editor != NULL) g_object_unref(editor);
    if (editor_root != NULL) g_object_unref(editor_root);
    if (selector != NULL) g_object_unref(selector);
    if (old_context_button != NULL) g_object_unref(old_context_button);
    if (last_context_button != NULL) g_object_unref(last_context_button);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    if (application != NULL) g_object_unref(application);
    g_free(layout);
    g_free(document);
    g_free(oversized);
    if (directory_changed && g_chdir(original_directory) != 0) failed = 1;
    if (environment_isolated) {
        if (original_data_path != NULL) {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data_path, TRUE)) failed = 1;
        } else {
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
        }
    }
    if (fixture_path != NULL)
        (void)printf("Retained isolated Studio fixture: %s\n", fixture_path);
    if (error != NULL) (void)fprintf(stderr, "Native fixture: %s\n", error->message);
    g_clear_error(&error);
    g_free(original_data_path);
    g_free(original_directory);
    g_free(fixture_path);
    return failed;
}
